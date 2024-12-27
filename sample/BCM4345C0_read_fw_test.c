#include <stddef.h>
#include <stdint.h>

#include "common.h"
#include "kernel.h"
#include "log.h"
#include "mm.h"
#include "utils.h"

extern const uint8_t _binary_BCM4345C0_hcd_start[];
extern const uint8_t _binary_BCM4345C0_hcd_end[];
extern const size_t _binary_BCM4345C0_hcd_size;

volatile uint8_t *bcm4345c0_test_ptr = ((uint8_t *)_binary_BCM4345C0_hcd_start);
volatile uint8_t *bcm4345c0_test_end = ((uint8_t *)_binary_BCM4345C0_hcd_end);
size_t bcm4345c0_test_size = (size_t)(&_binary_BCM4345C0_hcd_size);

void kernel_init() {
  mini_uart_init();
  log_init(LOG_MODE_MINIUART);
  int el = get_el();
  log("Hello! Welcome to BCM4345C0 test app. EL: %d\n\r", el);
}

void kernel_main() {
  kernel_init();
  int status = 0;

  log("Checking first few bytes for data integrity...\n\r");
  log("Firmware start address: %d\r\n", bcm4345c0_test_ptr);
  log("Firmware end address %d\n\r", bcm4345c0_test_end);
  log("Firmware size %d\n\r", bcm4345c0_test_size);

  if (bcm4345c0_test_size != (bcm4345c0_test_end - bcm4345c0_test_ptr)) {
    log("Size mismatch. Linker Size %d. Calculated size %d\n\r", bcm4345c0_test_size,
        (bcm4345c0_test_end - bcm4345c0_test_ptr));
    return;
  }

  // Display first few bytes for verification
  log("First few bytes: ");
  for (int i = 0; i < 16; i++) {
    log("%d ", bcm4345c0_test_ptr[i]);
  }
  log("\n\r");

  uint16_t expected_values[] = {
    0x4c, 0xfc, 0x46, 0x00, 0x9c, 0x21, 0x00, 0x42, 0x52, 0x43, 0x4d, 0x63, 0x66, 0x67, 0x53, 0x00,
    0x00, 0x00, 0x00, 0x32, 0x00, 0x00, 0x00, 0x01, 0x01, 0x04, 0x18, 0x92, 0x00, 0x00, 0x00, 0x03,
    0x06, 0xac, 0x1f, 0x00, 0xc0, 0x45, 0x43, 0x00, 0x01, 0x1c, 0x42, 0x9c, 0x21, 0x00, 0x00, 0x00,
  };

  for (int i = 0; i < sizeof(expected_values) / sizeof(expected_values[0]); i++) {
    if (bcm4345c0_test_ptr[i] != expected_values[i]) {
      log("Failed reading BCM4345C0 firmware: byte %d\n\r", i + 1);
      log("Expected: %d\n\r", expected_values[i]);
      log("Actual: %d\n\r", bcm4345c0_test_ptr[i]);
      status = 1;
    }
  }

  while (1) {
    if (status) {
      log("Status %d\n\r", status);
      return;
    } else {
      log("Passed!\n\r");
      return;
    }
  }
}
