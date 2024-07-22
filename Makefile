RPI_VERSION ?= 4
ARMGNU ?= aarch64-linux-gnu

BOOTMNT ?= /mnt/e

C_FLAGS = -DRPI_VERSION=$(RPI_VERSION) -Wall -nostdlib -nostartfiles -ffreestanding -Iinc -mgeneral-regs-only
ASM_FLAGS = -Iinc

BUILD_DIR = build
OBJ_DIR = $(BUILD_DIR)/obj
DEP_DIR = $(BUILD_DIR)/dep
SRC_DIR = src

all: $(BUILD_DIR)/kernel8.img

clean:
	rm -rf $(BUILD_DIR)

$(OBJ_DIR)/%_c.o: $(SRC_DIR)/%.c
	mkdir -p $(OBJ_DIR) $(DEP_DIR)
	$(ARMGNU)-gcc $(C_FLAGS) -MMD -MF $(DEP_DIR)/$*_c.d -c $< -o $@

$(OBJ_DIR)/%_s.o: $(SRC_DIR)/%.S
	mkdir -p $(OBJ_DIR) $(DEP_DIR)
	$(ARMGNU)-gcc $(C_FLAGS) -MMD -MF $(DEP_DIR)/$*_s.d -c $< -o $@

C_FILES = $(wildcard $(SRC_DIR)/*.c)
ASM_FILES = $(wildcard $(SRC_DIR)/*.S)
OBJ_FILES = $(C_FILES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%_c.o)
OBJ_FILES += $(ASM_FILES:$(SRC_DIR)/%.S=$(OBJ_DIR)/%_s.o)

DEP_FILES = $(OBJ_FILES:$(OBJ_DIR)/%.o=$(DEP_DIR)/%.d)
-include $(DEP_FILES)

$(BUILD_DIR)/kernel8.img: $(SRC_DIR)/linker.ld $(OBJ_FILES)
	@echo ""
	@echo "Building for RPI $(value RPI_VERSION)"
	@echo "Deploying to $(value BOOTMNT)"
	@echo ""
	$(ARMGNU)-ld -T $(SRC_DIR)/linker.ld -o $(BUILD_DIR)/kernel8.elf $(OBJ_FILES)
	$(ARMGNU)-objcopy $(BUILD_DIR)/kernel8.elf -O binary $(BUILD_DIR)/kernel8.img
# ifeq ($(RPI_VERSION), 4)
# 	cp $(BUILD_DIR)/kernel8.img $(BOOTMNT)/kernel8-rpi4.img
# else
# 	cp $(BUILD_DIR)/kernel8.img $(BOOTMNT)/
# endif
# 	cp config.txt $(BOOTMNT)/
# 	sync
