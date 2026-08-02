#include <stddef.h>

#include "hardware.h"
#include "hci.h"
#include "log.h"
#include "timer.h"
#include "uart.h"

#define HCI_COMMAND_PACKET 0x01
#define HCI_EVENT_PACKET 0x04
#define HCI_RESET 0x0C03
#define HCI_READ_LOCAL_VERSION 0x1001
#define EVT_COMMAND_COMPLETE 0x0E

static uint8_t event_buffer[256];

// Helper function to send HCI command
void send_hci_command(uint16_t opcode, uint8_t *params, uint8_t param_len) {
  uint8_t header[4];
  header[0] = HCI_COMMAND_PACKET;
  header[1] = opcode & 0xFF;
  header[2] = (opcode >> 8) & 0xFF;
  header[3] = param_len;

  log("Sending command: ");
  for (int i = 0; i < 4; i++) {
    log("%d ", header[i]);
    hw_transmit_byte(header[i]);
    hw_delay_ms(1);
  }
  log("\r\n");

  if (param_len > 0) {
    for (int i = 0; i < param_len; i++) {
      hw_transmit_byte(params[i]);
      hw_delay_ms(1);
    }
  }
}

uint16_t receive_hci_event(uint8_t *buffer, uint16_t buffer_size) {
  uint32_t timeout = hw_get_time_ms() + 1000;  // 1 second timeout

  // Wait for and read packet type
  while (1) {
    buffer[0] = hw_receive_byte();
    log("Received byte: %d\r\n", buffer[0]);

    if (buffer[0] == HCI_EVENT_PACKET) {
      break;
    }

    if (hw_get_time_ms() > timeout) {
      log("Timeout waiting for event packet\r\n");
      return 0;
    }
    hw_delay_ms(1);
  }

  buffer[1] = hw_receive_byte();
  log("Event code: %d\r\n", buffer[1]);

  buffer[2] = hw_receive_byte();
  uint8_t param_len = buffer[2];
  log("Parameter length: %d\r\n", param_len);

  if (param_len + 3 > buffer_size) {
    log("Event too large for buffer\r\n");
    return 0;
  }

  log("Parameters: ");
  for (int i = 0; i < param_len; i++) {
    buffer[i + 3] = hw_receive_byte();
    log("%d ", buffer[i + 3]);
  }
  log("\r\n");

  return param_len + 3;
}

void process_event(uint8_t *buffer, uint16_t length) {
  if (length < 3) {
    log("Event too short\r\n");
    return;
  }

  uint8_t evt_code = buffer[1];
  uint8_t param_len = buffer[2];

  log("Processing event - Type: %d, Code: %d, Length: %d\r\n", buffer[0], evt_code, param_len);

  if (evt_code == EVT_COMMAND_COMPLETE && length >= 7) {
    uint16_t opcode = buffer[4] | (buffer[5] << 8);
    uint8_t status = buffer[6];

    log("Command Complete - Opcode: %d, Status: %d\r\n", opcode, status);

    if (opcode == HCI_READ_LOCAL_VERSION && status == 0 && length >= 15) {
      uint8_t hci_ver = buffer[7];
      uint16_t hci_rev = buffer[8] | (buffer[9] << 8);
      uint8_t lmp_ver = buffer[10];
      uint16_t manufacturer = buffer[11] | (buffer[12] << 8);
      uint16_t lmp_subver = buffer[13] | (buffer[14] << 8);

      log("Version Information:\r\n");
      log("  HCI Version: %d\r\n", hci_ver);
      log("  HCI Revision: %d\r\n", hci_rev);
      log("  LMP Version: %d\r\n", lmp_ver);
      log("  Manufacturer: %d\r\n", manufacturer);
      log("  LMP Subversion: %d\r\n", lmp_subver);
    }
  }
}

void kernel_main() {
  mini_uart_init();
  log_init(LOG_MODE_MINIUART);
  log("Starting Bluetooth test...\r\n");

  UartSettings bt_settings = { .uart = UART0, .cts = 30, .rts = 31, .tx = 32, .rx = 33, .bluetooth = true };

  log("Initializing UART...\r\n");
  uart_init(&bt_settings);

  log("Waiting for controller to stabilize...\r\n");
  hw_delay_ms(250);

  log("Sending HCI Reset command...\r\n");
  send_hci_command(HCI_RESET, NULL, 0);

  hw_delay_ms(100);
  uint16_t length = receive_hci_event(event_buffer, sizeof(event_buffer));
  if (length > 0) {
    process_event(event_buffer, length);
  }

  hw_delay_ms(250);

  log("Sending Read Local Version command...\r\n");
  send_hci_command(HCI_READ_LOCAL_VERSION, NULL, 0);

  hw_delay_ms(100);
  length = receive_hci_event(event_buffer, sizeof(event_buffer));
  if (length > 0) {
    process_event(event_buffer, length);
  }

  log("Test complete\r\n");

  while (1) {
    hw_delay_ms(1000);
  }
}