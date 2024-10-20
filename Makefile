RPI_VERSION ?= 4
ARMGNU ?= aarch64-linux-gnu

BOOTMNT ?= /mnt/d

C_FLAGS = -DRPI_VERSION=$(RPI_VERSION) -Wall -nostdlib -nostartfiles -ffreestanding -Iinc -Iutils/inc -Idrivers/inc -mgeneral-regs-only
ASM_FLAGS = -Iinc

BUILD_DIR = build
DRIVER_DIR = drivers
UTILS_DIR = utils
UTILS_SRC_DIR = $(UTILS_DIR)/src
UTILS_INC_DIR = $(UTILS_DIR)/inc
DRIVER_SRC_DIR = $(DRIVER_DIR)/src
DRIVER_INC_DIR = $(DRIVER_DIR)/inc
OBJ_DIR = $(BUILD_DIR)/obj
DEP_DIR = $(BUILD_DIR)/dep
LIB_DIR = lib
KERNEL_DIR = kernel

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

C_FILES = $(wildcard $(LIB_DIR)/*.c)
DRIVER_FILES = $(wildcard $(DRIVER_SRC_DIR)/*.c)
UTILS_FILES = $(wildcard $(UTILS_SRC_DIR)/*.c)
KERNEL_FILES = $(wildcard $(KERNEL_DIR)/*.c)
ASM_FILES = $(wildcard $(LIB_DIR)/*.S)
OBJ_FILES = $(C_FILES:$(LIB_DIR)/%.c=$(OBJ_DIR)/%_c.o)
OBJ_FILES += $(ASM_FILES:$(LIB_DIR)/%.S=$(OBJ_DIR)/%_s.o)
OBJ_FILES += $(DRIVER_FILES:$(DRIVER_SRC_DIR)/%.c=$(OBJ_DIR)/%_c.o)
OBJ_FILES += $(KERNEL_FILES:$(KERNEL_DIR)/%.c=$(OBJ_DIR)/%_c.o)
OBJ_FILES += $(UTILS_FILES:$(UTILS_SRC_DIR)/%.c=$(OBJ_DIR)/%_c.o)

DEP_FILES = $(OBJ_FILES:$(OBJ_DIR)/%.o=$(DEP_DIR)/%.d)
-include $(DEP_FILES)

$(BUILD_DIR)/kernel8.img: linker.ld $(OBJ_FILES)
	@echo ""
	@echo "Building for RPI $(value RPI_VERSION)"
	@echo "Deploying to $(value BOOTMNT)"
	@echo ""
	$(ARMGNU)-ld -T linker.ld -o $(BUILD_DIR)/kernel8.elf $(OBJ_FILES)
	$(ARMGNU)-objcopy $(BUILD_DIR)/kernel8.elf -O binary $(BUILD_DIR)/kernel8.img
ifeq ($(RPI_VERSION), 4)
	# cp $(BUILD_DIR)/kernel8.img $(BOOTMNT)/kernel8-rpi4.img
else
	# cp $(BUILD_DIR)/kernel8.img $(BOOTMNT)/
endif
	# cp config.txt $(BOOTMNT)/
	sync

armstub/build/armstub_s.o: armstub/src/armstub.S
	mkdir -p $(@D)
	$(ARMGNU)-gcc $(C_FLAGS) -MMD -c $< -o $@

armstub: armstub/build/armstub_s.o
	$(ARMGNU)-ld --section-start=.text=0 -o armstub/build/armstub.elf armstub/build/armstub_s.o
	$(ARMGNU)-objcopy armstub/build/armstub.elf -O binary armstub/build/armstub-new.bin
	# cp armstub-new.bin $(BOOTMNT)/
	# sync