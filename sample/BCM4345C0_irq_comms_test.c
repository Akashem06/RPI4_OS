#include <stddef.h>

#include "hardware.h"
#include "hci.h"
#include "irq.h"
#include "log.h"
#include "mini_uart.h"
#include "mm.h"
#include "timer.h"
#include "uart.h"

UartSettings test_bt_settings = {
  .uart = UART0, .cts = 30, .rts = 31, .tx = 32, .rx = 33, .bluetooth = true
};

/* State machine states */
typedef enum { STATE_IDLE, STATE_WAIT_RESET, STATE_WAIT_VERSION, STATE_COMPLETE } TestState;

/* Global state */
static volatile TestState current_state = STATE_IDLE;
static volatile bool command_complete = false;
static volatile uint16_t last_opcode = 0;
static volatile uint8_t last_status = 0;
static volatile struct {
  uint8_t hci_version;
  uint16_t hci_revision;
  uint8_t lmp_version;
  uint16_t manufacturer;
  uint16_t lmp_subversion;
} version_info;

/* Buffer for receiving HCI events */
static uint8_t rx_buffer[256];
static volatile uint16_t rx_write_idx = 0;
static volatile uint16_t rx_read_idx = 0;

/* UART interrupt handler */
void handle_uart0_irq(void) {
  if (test_bt_settings.uart->mis & ((1 << 4) | (1 << 6))) {
    test_bt_settings.uart->icr = (1 << 4) | (1 << 6);

    // Handle RX FIFO full
    if (test_bt_settings.uart->fr & (1 << 6)) {
      test_bt_settings.uart->cr &= ~(1 << 11);  // Assert RTS
    }

    // Read available bytes
    while (!(test_bt_settings.uart->fr & (1 << 4))) {  // While RX FIFO not empty
      uint8_t byte = test_bt_settings.uart->dr & 0xFF;
      uint16_t next_idx = (rx_write_idx + 1) % sizeof(rx_buffer);
      if (next_idx != rx_read_idx) {  // If buffer not full
        rx_buffer[rx_write_idx] = byte;
        rx_write_idx = next_idx;
        log("BYTE %d \r\n", byte);
      }
    }

    // Release RTS if buffer has space
    if ((rx_write_idx + 32) % sizeof(rx_buffer) != rx_read_idx) {
      test_bt_settings.uart->cr |= (1 << 11);
    }
  }

  // Clear any other pending interrupts
  test_bt_settings.uart->icr = 0x7FF;
}

/* Process received HCI events */
static void process_hci_event(void) {
  if (rx_read_idx == rx_write_idx) {
    log("No data\r\n");
    return;  // No data
  }

  // Wait for complete event header
  if ((rx_write_idx - rx_read_idx + sizeof(rx_buffer)) % sizeof(rx_buffer) < 3) return;

  uint8_t event_code = rx_buffer[rx_read_idx + 1];
  uint8_t param_len = rx_buffer[rx_read_idx + 2];

  // Wait for complete event
  if ((rx_write_idx - rx_read_idx + sizeof(rx_buffer)) % sizeof(rx_buffer) < (3 + param_len))
    return;

  // Process Command Complete event
  if (event_code == 0x0E) {
    uint16_t opcode = rx_buffer[rx_read_idx + 4] | (rx_buffer[rx_read_idx + 5] << 8);
    uint8_t status = rx_buffer[rx_read_idx + 6];

    last_opcode = opcode;
    last_status = status;
    command_complete = true;

    // Handle Read Local Version Information response
    if (opcode == 0x1001 && status == 0) {
      version_info.hci_version = rx_buffer[rx_read_idx + 7];
      version_info.hci_revision = rx_buffer[rx_read_idx + 8] | (rx_buffer[rx_read_idx + 9] << 8);
      version_info.lmp_version = rx_buffer[rx_read_idx + 10];
      version_info.manufacturer = rx_buffer[rx_read_idx + 11] | (rx_buffer[rx_read_idx + 12] << 8);
      version_info.lmp_subversion =
          rx_buffer[rx_read_idx + 13] | (rx_buffer[rx_read_idx + 14] << 8);
    }
  }

  // Advance read pointer
  rx_read_idx = (rx_read_idx + 3 + param_len) % sizeof(rx_buffer);
}

/* Send HCI command with timeout */
static bool send_hci_command(uint16_t opcode, uint8_t *params, uint8_t param_len) {
  uint8_t cmd[256] = { 0x01, opcode & 0xFF, (opcode >> 8) & 0xFF, param_len };

  memcpy(&cmd[4], params, param_len);

  command_complete = false;

  // Send command bytes
  for (int i = 0; i < param_len + 4; i++) {
    while (test_bt_settings.uart->fr & (1 << 5)) {  // Wait for TX FIFO space
      log("Waiting for FIFO Space\r\n");
    }
    test_bt_settings.uart->dr = cmd[i];
    hw_delay_ms(1);
  }

  return true;
}

/* Wait for command completion with timeout */
static bool wait_command_complete(uint16_t expected_opcode) {
  while (!command_complete) {
    log("Waiting for command complete \r\n");
    process_hci_event();
  }

  return command_complete && last_opcode == expected_opcode && last_status == 0;
}

void kernel_main(void) {
  mini_uart_init();
  log_init(LOG_MODE_MINIUART);
  log("Initializing Bluetooth test (interrupt version)...\n\r");

  // Initialize hardware
  uart_init(&test_bt_settings);
  irq_init_vectors();
  enable_interrupt_controller();
  irq_enable();

  // Wait for controller to stabilize
  hw_delay_ms(100);

  current_state = STATE_WAIT_RESET;
  log("Sending HCI Reset command...\n");

  // Send HCI Reset
  if (!send_hci_command(0x0C03, NULL, 0) || !wait_command_complete(0x0C03)) {
    log("HCI Reset failed!\n");
    return;
  }

  current_state = STATE_WAIT_VERSION;
  log("Sending Read Local Version command...\n");

  // Send Read Local Version Information
  if (!send_hci_command(0x1001, NULL, 0) || !wait_command_complete(0x1001)) {
    log("Read Local Version failed!\n");
    return;
  }

  // Print version information
  log("Version Information:\n");
  log("  HCI Version: %d\n", version_info.hci_version);
  log("  HCI Revision: %d\n", version_info.hci_revision);
  log("  LMP Version: %d\n", version_info.lmp_version);
  log("  Manufacturer: %d\n", version_info.manufacturer);
  log("  LMP Subversion: %d\n", version_info.lmp_subversion);

  current_state = STATE_COMPLETE;
  log("Test complete\n");

  while (1) {
    // Main loop - could add more commands here
    process_hci_event();
  }
}