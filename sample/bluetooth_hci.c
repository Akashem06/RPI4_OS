#include <stdint.h>

#include "common.h"
#include "hci.h"
#include "hci_defs.h"
#include "irq.h"
#include "kernel.h"
#include "log.h"
#include "timer.h"
#include "utils.h"

/* Test Bluetooth Address */
uint8_t test_bt_addr[6] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66 };
char local_name[] = "Aryan's RPI BLE Stack";
BCM4345C0Info module_info;

void kernel_init() {
  mini_uart_init();
  log_init(LOG_MODE_MINIUART);
  int el = get_el();
  log("Hello! Welcome to Bluetooth HCI sample app. EL: %d\n\r", el);
}

void kernel_main() {
  kernel_init();

  HCIError status;

  log("Starting Bluetooth initialization...\n\r");

  // Initialize HCI
  status = HCI_init();
  if (status != HCI_ERROR_SUCCESS) {
    log("HCI init failed with status: %d\n\r", status);
    return;
  }

  // Set Bluetooth address
  log("Setting Bluetooth address...\n\r");
  status = HCI_set_bt_addr(test_bt_addr);
  if (status != HCI_ERROR_SUCCESS) {
    log("Setting BT address failed with status: %d\n\r", status);
    return;
  }

  // Get Bluetooth address
  static uint8_t read_bt_addr[6];
  log("Getting Bluetooth address...\n\r");
  status = HCI_get_bt_addr(read_bt_addr);

  for (uint8_t i = 0; i < 6; i++) {
    log("BT ADDR READ: %d\r\n", read_bt_addr[i]);
  }

  if (status != HCI_ERROR_SUCCESS) {
    log("Setting BT address failed with status: %d\n\r", status);
    return;
  }

  log("Setting event mask\r\n");
  status = HCI_BLE_set_event_mask(0xff);
  if (status != HCI_ERROR_SUCCESS) {
    log("Setting event mask failed with status: %d\n\r", status);
    return;
  }

  log("Setting local name...\n\r");
  status = HCI_set_local_name(local_name);
  if (status != HCI_ERROR_SUCCESS) {
    log("Failed to set local name with status %d\n\r", status);
    return;
  }

  // Configure advertising parameters
  uint8_t dummy_addr[6] = { 0, 0, 0, 0, 0, 0 };

  status = HCI_BLE_set_advertising_param(
      100,                                              /*! min interval (100ms) */
      100,                                              /*! max interval (100ms) */
      ADV_TYPE_UNDIRECT_CONN,                           /*! connectable undirected advertising */
      ADV_OWN_ADDR_PUBLIC,                              /*! own address type */
      ADV_DIR_ADDR_PUBLIC,                              /*! direct address type */
      dummy_addr,                                       /*! direct address */
      ADV_CHANNEL_37 | ADV_CHANNEL_38 | ADV_CHANNEL_39, /*! all channels */
      ADV_FILTER_POLICY_ALLOW_ALL                       /*! no filtering */
  );

  if (status != HCI_ERROR_SUCCESS) {
    log("Setting advertising parameters failed with status: %d\n\r", status);
    return;
  }

  status = HCI_BLE_set_advertising_enable(true);
  log("Enabling advertising\n\r");
  if (status != HCI_ERROR_SUCCESS) {
    log("Enabling advertising data failed with status %d\n\r", status);
    return;
  }

  HCI_get_module_status(&module_info);
  HCI_print_module_status(&module_info);

  // Main loop - monitor state and print status
  while (1) {
    HCIState current_state = HCI_get_state();
    switch (current_state) {
      case HCI_STATE_ADVERTISING:
        log("Device is advertising. Name: %s\n\r", local_name);
        log("You should be able to see this device on your phone's Bluetooth scanner\n\r");
        break;
      case HCI_STATE_ON:
        log("Bluetooth is on but not advertising\n\r");
        break;
      case HCI_STATE_IDLE:
        log("Bluetooth is off\n\r");
        break;
      default:
        log("Current state: %d\n\r", current_state);
        break;
    }
    timer_sleep(1000);  // Status update every second
  }
}
