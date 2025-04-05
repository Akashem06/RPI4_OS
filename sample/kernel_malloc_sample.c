#include "kernel.h"

#include "kernel_malloc.h"
#include "log.h"
#include "mem_utils.h"
#include "mini_uart.h"
#include "utils.h"

#define STRESS_ALLOCS 10

UartSettings settings = {
  .uart = UART0,
  .tx = 14,
  .rx = 15,
};

// Simple delay function that doesn't rely on timer.h
void simple_delay(unsigned int count) {
  for (volatile unsigned int i = 0; i < count; i++) {
    // Just a busy wait
  }
}

typedef struct {
  u32 id;
  u32 value;
  char name[32];
} TestStruct;

void test_small_allocations() {
  log("Testing small allocations...\n\r");

  void *ptrs[10];
  for (int i = 0; i < 10; i++) {
    size_t size = (i + 1) * 8;
    ptrs[i] = kmalloc(size);
    if (ptrs[i]) {
      log("  Allocated %d bytes at 0x%lx\n\r", size, (u64)ptrs[i]);
      // Write some data to validate memory
      for (size_t j = 0; j < size; j++) {
        ((u8 *)ptrs[i])[j] = (u8)j;
      }
    } else {
      log("  Failed to allocate %d bytes\n\r", size);
    }
  }

  // Free half of the allocations
  for (int i = 0; i < 10; i += 2) {
    if (ptrs[i]) {
      log("  Freeing allocation at %p\n\r", ptrs[i]);
      kfree(ptrs[i]);
      ptrs[i] = NULL;
    }
  }

  // Allocate some more to test reuse
  for (int i = 0; i < 10; i += 2) {
    size_t size = (i + 1) * 8;
    ptrs[i] = kmalloc(size);
    if (ptrs[i]) {
      log("  Reallocated %d bytes at %p\n\r", size, ptrs[i]);
    } else {
      log("  Failed to reallocate %d bytes\n\r", size);
    }
  }

  // Free all remaining allocations
  for (int i = 0; i < 10; i++) {
    if (ptrs[i]) {
      kfree(ptrs[i]);
    }
  }

  log("Small allocations test complete\n\r");
}

void test_medium_allocations() {
  log("Testing medium allocations...\n\r");

  void *ptrs[5];
  size_t sizes[5] = { 256, 512, 1024, 2048, 4096 / 4 };

  for (int i = 0; i < 5; i++) {
    ptrs[i] = kmalloc(sizes[i]);
    if (ptrs[i]) {
      log("  Allocated %d bytes at %p\n\r", sizes[i], ptrs[i]);
      memzero((u64)ptrs[i], sizes[i]);
      ((u32 *)ptrs[i])[0] = 0xDEADBEEF;
      ((u32 *)ptrs[i])[sizes[i] / 4 - 1] = 0xCAFEBABE;
    } else {
      log("  Failed to allocate %d bytes\n\r", sizes[i]);
    }
  }

  for (int i = 0; i < 5; i++) {
    if (ptrs[i]) {
      if (((u32 *)ptrs[i])[0] != 0xDEADBEEF || ((u32 *)ptrs[i])[sizes[i] / 4 - 1] != 0xCAFEBABE) {
        log("  Data corruption detected in allocation %d\n\r", i);
      } else {
        log("  Allocation %d data verified: first=0x%x, last=0x%x\n\r", i, ((u32 *)ptrs[i])[0],
            ((u32 *)ptrs[i])[sizes[i] / 4 - 1]);
      }
    }
  }

  for (int i = 0; i < 5; i++) {
    if (ptrs[i]) {
      log("  Freeing allocation at %p\n\r", ptrs[i]);
      kfree(ptrs[i]);
    }
  }

  log("Medium allocations test complete\n\r");
}

void test_large_allocations() {
  log("Testing large allocations...\n\r");

  void *ptrs[4];
  size_t sizes[4] = { 4096, 8192, 16384, 517120 };

  for (int i = 0; i < 4; i++) {
    ptrs[i] = kmalloc(sizes[i]);
    if (ptrs[i]) {
      log("  Allocated %d bytes at %p\n\r", sizes[i], ptrs[i]);

      for (size_t j = 0; j < sizes[i]; j += 4096) {
        ((u32 *)ptrs[i])[j / 4] = i + 1;
      }
    } else {
      log("  Failed to allocate %d bytes\n\r", sizes[i]);
    }
  }

  for (int i = 0; i < 4; i++) {
    if (ptrs[i]) {
      log("  Freeing large allocation at %p\n\r", ptrs[i]);
      kfree(ptrs[i]);
    }
  }

  log("Large allocations test complete\n\r");
}

void test_zero_allocation() {
  log("Testing zero allocations...\n\r");

  size_t size = 128;
  u32 *ptr = (u32 *)kzalloc(size);

  if (ptr) {
    log("  Allocated zeroed memory at %p\n\r", ptr);

    bool is_zeroed = true;
    for (size_t i = 0; i < size / 4; i++) {
      if (ptr[i] != 0) {
        is_zeroed = false;
        log("  Error: Non-zero value found at offset %d: 0x%x\n\r", i * 4, ptr[i]);
        break;
      }
    }

    if (is_zeroed) {
      log("  Zeroed memory verified\n\r");
    }

    for (size_t i = 0; i < size / 4; i++) {
      ptr[i] = i + 0x100;
    }

    bool data_valid = true;
    for (size_t i = 0; i < size / 4; i++) {
      if (ptr[i] != i + 0x100) {
        data_valid = false;
        log("  Error: Data corruption at offset %d\n\r", i * 4);
        break;
      }
    }

    if (data_valid) {
      log("  Data integrity verified\n\r");
    }

    kfree(ptr);
  } else {
    log("  Failed to allocate zeroed memory\n\r");
  }

  log("Zero allocations test complete\n\r");
}

void test_struct_allocations() {
  log("Testing struct allocations...\n\r");

  int count = 5;
  TestStruct *structs = (TestStruct *)kmalloc(count * sizeof(TestStruct));

  if (structs) {
    log("  Allocated array of %d structs at %p (each %d bytes)\n\r", count, structs, sizeof(TestStruct));

    for (int i = 0; i < count; i++) {
      structs[i].id = i + 1;
      structs[i].value = i * 100;
      char name[32];
      name[0] = 'T';
      name[1] = 'e';
      name[2] = 's';
      name[3] = 't';
      name[4] = '0' + i;
      name[5] = '\0';

      for (int j = 0; j < 6; j++) {
        structs[i].name[j] = name[j];
      }
    }

    for (int i = 0; i < count; i++) {
      log("  Struct %d: id=%d, value=%d, name=%s\n\r", i, structs[i].id, structs[i].value, structs[i].name);
    }

    kfree(structs);
  } else {
    log("  Failed to allocate struct array\n\r");
  }

  log("Struct allocations test complete\n\r");
}

void kernel_init() {
  uart_init(&settings);

  log_init(LOG_MODE_UART);

  int el = get_el();

  log("QEMU Test: Kernel booted at EL%d\n\r", el);
}

void kernel_main() {
  kernel_init();

  log("\n\r===== KERNEL MEMORY ALLOCATOR TEST =====\n\r");

  // Initialize our memory pool
  log("Starting memory allocator tests...\n\r");

  // Test small allocations (slab allocator)
  test_small_allocations();
  simple_delay(10000000);

  // Test medium allocations (larger slab sizes)
  test_medium_allocations();
  simple_delay(10000000);

  // Test large allocations (direct buddy allocator)
  test_large_allocations();
  simple_delay(10000000);

  // Test zeroed allocations
  test_zero_allocation();
  simple_delay(10000000);

  // Test struct allocations
  test_struct_allocations();
  simple_delay(10000000);

  log("\n\r===== ALL TESTS COMPLETED =====\n\r");

  u32 counter = 0;
  while (1) {
    log("Memory tests completed. Counter: %d\n\r", counter);
    counter++;
    simple_delay(50000000);
  }
}
