RPI_VERSION ?= 4
ARMGNU ?= aarch64-linux-gnu

# Directory structure
BUILD_DIR    := build
OBJ_DIR     := $(BUILD_DIR)/obj
DEP_DIR     := $(BUILD_DIR)/dep
SRC_DIRS    := lib/src utils/src drivers/src kernel/src RPI_Bluetooth/src BCM2711_hardware/src filesystem/src mm/src
INC_DIRS    := lib/inc utils/inc drivers/inc kernel/inc RPI_Bluetooth/inc BCM2711_hardware/inc filesystem/inc mm/inc
BCM4345C0_DIR := BCM4345C0

# Simulation in QEMU
QEMU      	:= qemu-system-aarch64
QEMU_FLAGS 	:= -M raspi4b -cpu cortex-a72 -smp 4 -m 2G -kernel $(BUILD_DIR)/kernel8.img -serial stdio -display none -d int,mmu

# Compiler and linker flags
WARNINGS     := -Wall -Wextra -Werror -Wno-unused-function -Wno-unused-variable -Wno-unused-parameter
COMMON_FLAGS := -DRPI_VERSION=$(RPI_VERSION) $(WARNINGS) -nostdlib -nostartfiles -ffreestanding -mgeneral-regs-only -march=armv8-a -g -O0
C_FLAGS      := $(COMMON_FLAGS) $(addprefix -I,$(INC_DIRS))
ASM_FLAGS    := $(COMMON_FLAGS) $(addprefix -I,$(INC_DIRS))
LD_FLAGS     := 

# Find all source files
C_SRCS   := $(shell find $(SRC_DIRS) -name '*.c')
ASM_SRCS := $(shell find lib/src -name '*.S')

# Generate object file names
C_OBJS   := $(C_SRCS:%.c=$(OBJ_DIR)/%_c.o)
ASM_OBJS := $(ASM_SRCS:%.S=$(OBJ_DIR)/%_s.o)
ALL_OBJS := $(C_OBJS) $(ASM_OBJS)

# Generate dependency file names
DEP_FILES := $(ALL_OBJS:$(OBJ_DIR)/%.o=$(DEP_DIR)/%.d)

# Firmware file
BCM_FW := $(OBJ_DIR)/BCM4345C0.o

all: directories $(BUILD_DIR)/kernel8.img

directories:
	@mkdir -p $(BUILD_DIR) $(OBJ_DIR) $(DEP_DIR) \
		$(addprefix $(OBJ_DIR)/,$(SRC_DIRS)) \
		$(addprefix $(DEP_DIR)/,$(SRC_DIRS))

clean:
	@echo "Cleaning build directory..."
	@rm -rf $(BUILD_DIR)
	@echo "Clean complete"

doxygen:
	@echo "Generating documentation..."
	@doxygen doxygen/Doxyfile
	@echo "Documentation generated"

$(OBJ_DIR)/%_c.o: %.c
	@echo "Compiling $<..."
	@mkdir -p $(dir $@) $(dir $(DEP_DIR)/$*_c.d)
	@$(ARMGNU)-gcc $(C_FLAGS) -MMD -MF $(DEP_DIR)/$*_c.d -c $< -o $@

$(OBJ_DIR)/%_s.o: %.S
	@echo "Assembling $<..."
	@mkdir -p $(dir $@) $(dir $(DEP_DIR)/$*_s.d)
	@$(ARMGNU)-gcc $(ASM_FLAGS) -MMD -MF $(DEP_DIR)/$*_s.d -c $< -o $@

# Bluetooth firmware object generation
$(BCM_FW): $(BCM4345C0_DIR)/BCM4345C0.hcd
	@echo "Converting Bluetooth firmware..."
	@mkdir -p $(dir $@)
	@cp $< $(OBJ_DIR)/BCM4345C0.hcd
	@cd $(OBJ_DIR) && $(ARMGNU)-objcopy -I binary -O elf64-littleaarch64 \
		-B aarch64 --rename-section .data=.rodata,alloc,load,readonly,data,contents \
		BCM4345C0.hcd BCM4345C0.o
	@rm $(OBJ_DIR)/BCM4345C0.hcd

# Final kernel image
$(BUILD_DIR)/kernel8.img: linker.ld $(ALL_OBJS) $(BCM_FW)
	@echo "Linking kernel..."
	@$(ARMGNU)-ld $(LD_FLAGS) $(BCM_FW) $(ALL_OBJS) -T linker.ld -o $(BUILD_DIR)/kernel8.elf
	@$(ARMGNU)-objcopy $(BUILD_DIR)/kernel8.elf -O binary $@
	@echo ""
	@echo "Building for RPI $(value RPI_VERSION)"
	@echo "Kernel build complete"
	@echo ""

sim: $(BUILD_DIR)/kernel8.img
	@echo "Starting QEMU simulation..."
	@$(QEMU) $(QEMU_FLAGS)

sim-debug: $(BUILD_DIR)/kernel8.img
	@echo "Starting QEMU simulation with GDB server..."
	@$(QEMU) $(QEMU_FLAGS) -s -S

# Armstub compilation
armstub/build/armstub_s.o: armstub/src/armstub.S
	@echo "Building armstub..."
	@mkdir -p $(@D)
	@$(ARMGNU)-gcc $(C_FLAGS) -MMD -c $< -o $@

armstub: armstub/build/armstub_s.o
	@$(ARMGNU)-ld --section-start=.text=0 -o armstub/build/armstub.elf $<
	@$(ARMGNU)-objcopy armstub/build/armstub.elf -O binary armstub/build/armstub-new.bin
	@echo "Armstub build complete"

format:
	@echo "Formatting files..."
	@find . -type f \( -name "*.c" -o -name "*.h" -o -name "*.c" \) -exec clang-format -i -style=file {} +

help:
	@echo "Available targets:"
	@echo "  all        - Build the kernel image"
	@echo "  doxygen    - Generate an HTML documentation website"
	@echo "  armstub    - Build the custom armstub"
	@echo "  clean      - Remove build directory"
	@echo "  format     - Format source files using clang-format"
	@echo "  sim        - Run kernel in QEMU"
	@echo "  sim-debug  - Run kernel in QEMU with GDB server enabled"

-include $(DEP_FILES)

.PHONY: all clean directories doxygen armstub format help sim sim-debug