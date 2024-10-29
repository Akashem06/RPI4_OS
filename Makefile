RPI_VERSION ?= 4
ARMGNU ?= aarch64-linux-gnu

C_FLAGS = -DRPI_VERSION=$(RPI_VERSION) -Wall -nostdlib -nostartfiles -ffreestanding -Iinc -Iutils/inc -Idrivers/inc -IRPI_Bluetooth/inc -mgeneral-regs-only
ASM_FLAGS = -Iinc

BUILD_DIR = build
BCM4345C0_DIR = BCM4345C0
OBJ_DIR = $(BUILD_DIR)/obj
DEP_DIR = $(BUILD_DIR)/dep
LIB_DIR = lib
KERNEL_DIR = kernel

UTILS_DIR = utils
UTILS_SRC_DIR = $(UTILS_DIR)/src
UTILS_INC_DIR = $(UTILS_DIR)/inc

DRIVER_DIR = drivers
DRIVER_SRC_DIR = $(DRIVER_DIR)/src
DRIVER_INC_DIR = $(DRIVER_DIR)/inc

BLUETOOTH_DIR = RPI_Bluetooth
BLUETOOTH_SRC_DIR = $(BLUETOOTH_DIR)/src
BLUETOOTH_INC_DIR = $(BLUETOOTH_DIR)/inc

all: $(BUILD_DIR)/kernel8.img

clean:
	rm -rf $(BUILD_DIR)

$(OBJ_DIR)/%_c.o: $(LIB_DIR)/%.c
	mkdir -p $(OBJ_DIR) $(DEP_DIR)
	$(ARMGNU)-gcc $(C_FLAGS) -MMD -MF $(DEP_DIR)/$*_c.d -c $< -o $@

$(OBJ_DIR)/%_s.o: $(LIB_DIR)/%.S
	mkdir -p $(OBJ_DIR) $(DEP_DIR)
	$(ARMGNU)-gcc $(C_FLAGS) -MMD -MF $(DEP_DIR)/$*_s.d -c $< -o $@

$(OBJ_DIR)/%_c.o: $(UTILS_SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR) $(DEP_DIR)
	$(ARMGNU)-gcc $(C_FLAGS) -MMD -MF $(DEP_DIR)/$*_c.d -c $< -o $@

$(OBJ_DIR)/%_c.o: $(DRIVER_SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR) $(DEP_DIR)
	$(ARMGNU)-gcc $(C_FLAGS) -MMD -MF $(DEP_DIR)/$*_c.d -c $< -o $@

$(OBJ_DIR)/%_c.o: $(KERNEL_DIR)/%.c
	mkdir -p $(OBJ_DIR) $(DEP_DIR)
	$(ARMGNU)-gcc $(C_FLAGS) -MMD -MF $(DEP_DIR)/$*_c.d -c $< -o $@
	
$(OBJ_DIR)/%_c.o: $(BLUETOOTH_SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR) $(DEP_DIR)
	$(ARMGNU)-gcc $(C_FLAGS) -MMD -MF $(DEP_DIR)/$*_c.d -c $< -o $@

$(OBJ_DIR)/BCM4345C0.o: $(BCM4345C0_DIR)/BCM4345C0.hcd
	mkdir -p $(OBJ_DIR)
	cp $(BCM4345C0_DIR)/BCM4345C0.hcd $(OBJ_DIR)/
	cd $(OBJ_DIR) && $(ARMGNU)-objcopy -I binary -O elf64-littleaarch64 -B aarch64 --rename-section .data=.rodata,alloc,load,readonly,data,contents BCM4345C0.hcd BCM4345C0.o
	rm $(OBJ_DIR)/BCM4345C0.hcd

C_FILES = $(wildcard $(LIB_DIR)/*.c)
DRIVER_FILES = $(wildcard $(DRIVER_SRC_DIR)/*.c)
UTILS_FILES = $(wildcard $(UTILS_SRC_DIR)/*.c)
KERNEL_FILES = $(wildcard $(KERNEL_DIR)/*.c)
BLUETOOTH_FILES = $(wildcard $(BLUETOOTH_SRC_DIR)/*.c)
ASM_FILES = $(wildcard $(LIB_DIR)/*.S)
OBJ_FILES = $(C_FILES:$(LIB_DIR)/%.c=$(OBJ_DIR)/%_c.o)
OBJ_FILES += $(ASM_FILES:$(LIB_DIR)/%.S=$(OBJ_DIR)/%_s.o)
OBJ_FILES += $(DRIVER_FILES:$(DRIVER_SRC_DIR)/%.c=$(OBJ_DIR)/%_c.o)
OBJ_FILES += $(KERNEL_FILES:$(KERNEL_DIR)/%.c=$(OBJ_DIR)/%_c.o)
OBJ_FILES += $(UTILS_FILES:$(UTILS_SRC_DIR)/%.c=$(OBJ_DIR)/%_c.o)
OBJ_FILES += $(BLUETOOTH_FILES:$(BLUETOOTH_SRC_DIR)/%.c=$(OBJ_DIR)/%_c.o)

DEP_FILES = $(OBJ_FILES:$(OBJ_DIR)/%.o=$(DEP_DIR)/%.d)
-include $(DEP_FILES)

$(BUILD_DIR)/kernel8.img: linker.ld $(OBJ_FILES) $(OBJ_DIR)/BCM4345C0.o
	$(ARMGNU)-ld $(OBJ_DIR)/BCM4345C0.o $(OBJ_FILES) -T linker.ld -o $(BUILD_DIR)/kernel8.elf
	$(ARMGNU)-objcopy $(BUILD_DIR)/kernel8.elf -O binary $(BUILD_DIR)/kernel8.img
	@echo ""
	@echo "Building for RPI $(value RPI_VERSION)"
	@echo "Kernel Build Done"
	@echo ""
	
armstub/build/armstub_s.o: armstub/src/armstub.S
	mkdir -p $(@D)
	$(ARMGNU)-gcc $(C_FLAGS) -MMD -c $< -o $@

armstub: armstub/build/armstub_s.o
	$(ARMGNU)-ld --section-start=.text=0 -o armstub/build/armstub.elf armstub/build/armstub_s.o
	$(ARMGNU)-objcopy armstub/build/armstub.elf -O binary armstub/build/armstub-new.bin
	@echo "Armstub Build Done"