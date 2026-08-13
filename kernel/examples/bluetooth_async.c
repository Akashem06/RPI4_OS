/*******************************************************************************************************************************
 * @file   bluetooth_async.c
 *
 * @brief  Kernel-space example, exercises the async BT stack end to end in QEMU
 *
 * @details QEMU raspi4b has no BT controller, so this drives the stack through a mock transport.
 *          The mock captures controller-bound bytes and, for commands, injects the matching
 *          Command Complete back into the RX ring. Covers the whole async path: ring + worker,
 *          blocking command sync, the HCI->GAP event bridge, and the GATT/ATT server (exchange
 *          MTU / read / write / error / notify) including a fragmented ACL PDU reassembled by the
 *          worker. Scheduling is cooperative, the orchestrator yields with schedule() and
 *          init_task's idle loop keeps the worker and timeouts running.
 *
 * @date   2026-08-05
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>

/* Inter-component Headers */
#include "irq.h"
#include "log.h"
#include "scheduler.h"
#include "timer.h"

/* Intra-component Headers */
#include "bt_ringbuf.h"
#include "bt_worker.h"
#include "gap.h"
#include "gatt.h"
#include "hci.h"
#include "hci_defs.h"
#include "kernel.h"

/* ATT PDU starts after the 5-byte HCI ACL header and the 4-byte L2CAP header */
#define ATT_OFF 9

/* Battery service + level characteristic used as the server DB under test */
#define SVC_UUID 0x180F
#define CHR_UUID 0x2A19
#define CHR_VALUE_HANDLE 3
#define TEST_CONN 0x0040

static int passes = 0;
static int fails = 0;

static void check(bool cond, const char *what) {
  if (cond) {
    passes++;
    log("  [PASS] %s\n\r", what);
  } else {
    fails++;
    log("  [FAIL] %s\n\r", what);
  }
}

/* ------------------------------------------------------------------ mock controller -------- */
#define TX_SINK_SIZE 256u

static u8 tx_sink[TX_SINK_SIZE];
static u16 tx_sink_len;
static int tx_calls;

static bool mock_autorespond = true;
static u8 mock_resp_status;
static u8 mock_resp_params[32];
static u16 mock_resp_param_len;
static u8 mock_cc[64];

static void mock_tx(const u8 *buf, u16 len) {
  tx_sink_len = 0;
  for (u16 i = 0; i < len && i < sizeof(tx_sink); i++) {
    tx_sink[i] = buf[i];
    tx_sink_len++;
  }
  tx_calls++;

  /* Only commands get an auto Command Complete, ACL data is captured for assertions */
  if (!mock_autorespond || len < 3 || buf[0] != HCI_COMMAND_PACKET) {
    return;
  }

  u16 plen = (u16)(4 + mock_resp_param_len);
  u16 n = 0;
  mock_cc[n++] = HCI_EVENT_PACKET;
  mock_cc[n++] = EVNT_BT_COMMAND_COMPLETE;
  mock_cc[n++] = (u8)plen;
  mock_cc[n++] = 0x01;
  mock_cc[n++] = buf[1];
  mock_cc[n++] = buf[2];
  mock_cc[n++] = mock_resp_status;
  for (u16 i = 0; i < mock_resp_param_len; i++) {
    mock_cc[n++] = mock_resp_params[i];
  }

  bt_rx_inject(mock_cc, n);
}

static const struct HciTransport mock_transport = { .tx = mock_tx };

/* Inject a raw ACL fragment (controller -> host): HCI ACL header + payload */
static u8 acl_pkt[96];
static void inject_acl(uint16_t handle, u8 pb_flag, const u8 *payload, u16 payload_len) {
  u16 n = 0;
  acl_pkt[n++] = HCI_ASYNC_DATA_PACKET;
  acl_pkt[n++] = handle & 0xFF;
  acl_pkt[n++] = ((handle >> 8) & 0x0F) | (pb_flag << 4);
  acl_pkt[n++] = payload_len & 0xFF;
  acl_pkt[n++] = (payload_len >> 8) & 0xFF;
  for (u16 i = 0; i < payload_len; i++) {
    acl_pkt[n++] = payload[i];
  }
  bt_rx_inject(acl_pkt, n);
}

/* Inject a complete single-fragment ATT PDU wrapped in an L2CAP frame */
static u8 l2cap_frame[64];
static void inject_att(uint16_t handle, const u8 *att, u16 att_len) {
  u16 n = 0;
  l2cap_frame[n++] = att_len & 0xFF;
  l2cap_frame[n++] = (att_len >> 8) & 0xFF;
  l2cap_frame[n++] = L2CAP_ATT_CID & 0xFF;
  l2cap_frame[n++] = (L2CAP_ATT_CID >> 8) & 0xFF;
  for (u16 i = 0; i < att_len; i++) {
    l2cap_frame[n++] = att[i];
  }
  inject_acl(handle, 0x02, l2cap_frame, n);
}

/* --------------------------------------------------------------- command completes --------- */
static void test_command_complete(void) {
  log("[cmd] blocking send unblocks on the injected Command Complete\n\r");
  mock_autorespond = true;
  mock_resp_status = 0x00;
  mock_resp_param_len = 0;
  HCI_set_state(HCI_STATE_IDLE);

  HCICommand reset = { .op_code.raw = CMD_BT_RESET, .parameter_length = 0, .parameters = NULL };
  HCIError st = HCI_send_command(&reset);

  check(st == HCI_ERROR_SUCCESS && HCI_get_state() == HCI_STATE_ON, "reset round-tripped through the worker");
}

/* --------------------------------------------------------------------- GAP events ---------- */
static int gap_cb_calls;
static GAPEventType gap_last_type;
static uint16_t gap_last_handle;

static void app_gap_cb(GAPEvent *event) {
  gap_cb_calls++;
  gap_last_type = event->type;
  gap_last_handle = event->connection_handle;
}

static void test_gap_connect_disconnect(void) {
  log("[gap] connection-complete + disconnection-complete translate to GAPEvents\n\r");
  static const u8 conn[] = { HCI_EVENT_PACKET, EVNT_BLE_EVENT_CODE, 0x13, SUB_EVNT_BLE_CONNECTION_COMPLETE,
                             0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  int before = gap_cb_calls;
  bt_rx_inject(conn, sizeof(conn));
  schedule();
  check(gap_cb_calls == before + 1 && gap_last_type == GAP_EVENT_CONNECTED && gap_last_handle == TEST_CONN,
        "connected event delivered with the right handle");

  static const u8 disc[] = { HCI_EVENT_PACKET, EVNT_BT_DISCONNECTION_COMPLETE, 0x04, 0x00, 0x40, 0x00, 0x13 };
  before = gap_cb_calls;
  bt_rx_inject(disc, sizeof(disc));
  schedule();
  check(gap_cb_calls == before + 1 && gap_last_type == GAP_EVENT_DISCONNECTED, "disconnected event delivered");
}

/* --------------------------------------------------------------------- GATT server --------- */
static bool gatt_write_fired;
static uint16_t gatt_write_len;
static u8 gatt_write_data[16];

static void app_gatt_cb(GATTEvent *event) {
  if (event->type == GATT_EVENT_WRITE_REQUEST) {
    gatt_write_fired = true;
    gatt_write_len = event->length;
    for (uint16_t i = 0; i < event->length && i < sizeof(gatt_write_data); i++) {
      gatt_write_data[i] = event->data[i];
    }
  }
}

static u8 att_buf[16];

static void test_gatt_exchange_mtu(void) {
  log("[gatt] exchange-MTU request -> response with our max MTU\n\r");
  att_buf[0] = ATT_EXCHANGE_MTU_REQUEST;
  att_buf[1] = 0xF7; /* client rx mtu 247 */
  att_buf[2] = 0x00;
  inject_att(TEST_CONN, att_buf, 3);
  schedule();
  check(tx_sink[ATT_OFF] == ATT_EXCHANGE_MTU_RESPONSE && tx_sink[ATT_OFF + 1] == (ATT_MAX_MTU & 0xFF) &&
            tx_sink[ATT_OFF + 2] == ((ATT_MAX_MTU >> 8) & 0xFF),
        "server answered with ATT_MAX_MTU");
}

static void test_gatt_read(void) {
  log("[gatt] read request -> read response with the characteristic value\n\r");
  att_buf[0] = ATT_READ_REQUEST;
  att_buf[1] = CHR_VALUE_HANDLE;
  att_buf[2] = 0x00;
  inject_att(TEST_CONN, att_buf, 3);
  schedule();
  check(tx_sink[ATT_OFF] == ATT_READ_RESPONSE && tx_sink[ATT_OFF + 1] == 0xDE && tx_sink[ATT_OFF + 2] == 0xAD,
        "read response carried the stored value 0xDEAD");
}

static void test_gatt_write(void) {
  log("[gatt] write request -> value stored, app event, write response\n\r");
  gatt_write_fired = false;
  att_buf[0] = ATT_WRITE_REQUEST;
  att_buf[1] = CHR_VALUE_HANDLE;
  att_buf[2] = 0x00;
  att_buf[3] = 0xBE;
  att_buf[4] = 0xEF;
  inject_att(TEST_CONN, att_buf, 5);
  schedule();

  check(gatt_write_fired && gatt_write_len == 2 && gatt_write_data[0] == 0xBE && gatt_write_data[1] == 0xEF,
        "app GATT_EVENT_WRITE_REQUEST fired with the written bytes");
  check(tx_sink[ATT_OFF] == ATT_WRITE_RESPONSE, "server emitted an ATT write response");

  u8 rb[8];
  uint16_t rl = sizeof(rb);
  GATT_read_characteristic_value(SVC_UUID, CHR_UUID, rb, &rl);
  check(rl == 2 && rb[0] == 0xBE && rb[1] == 0xEF, "characteristic value was updated in the DB");
}

static void test_gatt_read_unknown(void) {
  log("[gatt] read of an unknown handle -> ATT error response\n\r");
  att_buf[0] = ATT_READ_REQUEST;
  att_buf[1] = 0x63; /* handle 99, not in the DB */
  att_buf[2] = 0x00;
  inject_att(TEST_CONN, att_buf, 3);
  schedule();
  check(tx_sink[ATT_OFF] == ATT_ERROR_RESPONSE && tx_sink[ATT_OFF + 1] == ATT_READ_REQUEST &&
            tx_sink[ATT_OFF + 4] == GATT_ERROR_INVALID_HANDLE,
        "error response reported INVALID_HANDLE for the read request");
}

static void test_gatt_notification(void) {
  log("[gatt] GATT_send_notification builds a valid notification ACL\n\r");
  static const u8 nval[2] = { 0x12, 0x34 };
  GATTError ns = GATT_send_notification(TEST_CONN, CHR_VALUE_HANDLE, (uint8_t *)nval, 2);
  check(ns == GATT_ERROR_SUCCESS, "notification send succeeded");
  check(tx_sink[ATT_OFF] == ATT_HANDLE_VALUE_NOTIFICATION && tx_sink[ATT_OFF + 1] == CHR_VALUE_HANDLE &&
            tx_sink[ATT_OFF + 3] == 0x12 && tx_sink[ATT_OFF + 4] == 0x34,
        "notification ACL carried handle + value");
}

static u8 frag_buf[16];

static void test_gatt_reassembly(void) {
  log("[gatt] a write split across two ACL fragments is reassembled\n\r");
  gatt_write_fired = false;

  /* Full L2CAP frame: len=7, cid=0x0004, ATT write req handle 3 value AABBCCDD. Split 7 + 4. */
  frag_buf[0] = 0x07;
  frag_buf[1] = 0x00;
  frag_buf[2] = L2CAP_ATT_CID & 0xFF;
  frag_buf[3] = (L2CAP_ATT_CID >> 8) & 0xFF;
  frag_buf[4] = ATT_WRITE_REQUEST;
  frag_buf[5] = CHR_VALUE_HANDLE;
  frag_buf[6] = 0x00;
  inject_acl(TEST_CONN, 0x02, frag_buf, 7); /* start fragment */

  frag_buf[0] = 0xAA;
  frag_buf[1] = 0xBB;
  frag_buf[2] = 0xCC;
  frag_buf[3] = 0xDD;
  inject_acl(TEST_CONN, 0x01, frag_buf, 4); /* continuation */
  schedule();

  check(gatt_write_fired && gatt_write_len == 4 && gatt_write_data[0] == 0xAA && gatt_write_data[3] == 0xDD,
        "reassembled write delivered all four bytes to the app");
}

/* ------------------------------------------------------------------------- suite ----------- */
static void suite_thread(u64 arg) {
  (void)arg;
  log("\n\r===== BLUETOOTH ASYNC TEST (Phase 4) =====\n\r");

  schedule(); /* let the worker start up and park on its wakeup */

  test_command_complete();

  static const u8 dev_addr[6] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
  GAPError gi = GAP_init(app_gap_cb, (uint8_t *)dev_addr);
  check(gi == GAP_ERROR_SUCCESS, "GAP_init registered its event hook");
  test_gap_connect_disconnect();

  GATT_init();
  GATT_register_event_handler(app_gatt_cb);
  GATT_register_service(SVC_UUID, true);
  static const u8 initial[2] = { 0xDE, 0xAD };
  GATT_add_characteristic(SVC_UUID, CHR_UUID, GATT_PROP_READ | GATT_PROP_WRITE | GATT_PROP_NOTIFY,
                          GATT_PERM_READ | GATT_PERM_WRITE, (uint8_t *)initial, 2);

  test_gatt_exchange_mtu();
  test_gatt_read();
  test_gatt_write();
  test_gatt_read_unknown();
  test_gatt_notification();
  test_gatt_reassembly();

  log("\n\r===== BT ASYNC TEST DONE: %d passed, %d failed =====\n\r", passes, fails);

  while (1) {
    schedule();
  }
}

void kernel_main() {
  kernel_boot();

  log("\n\r===== STARTING SCHEDULER =====\n\r");
  scheduler_init();
  scheduler_start_tick(SCHED_TICK_HZ);

  HCI_set_transport(&mock_transport);
  bt_worker_start();

  scheduler_create_task(PF_KTHREAD, (u64)&suite_thread, 0, 9);

  irq_enable();

  /* init_task idle loop, also drives _schedule so the worker and timeouts keep running */
  while (1) {
    schedule();
  }
}
