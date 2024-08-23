// armstub is almost like a bootloader that initializes the OS for us
// 

.globl _start
_start:
    ldr w0, kernel_entry32
    br x0 // Call kernel entry 32

.ltorg

.org 0xf0
.globl stub_magic
stub_magic:
    .word 0x5afe570b // Firmware needs to find this exact value at memory 0xf0, otherwise it is not a valid armstub

.org 0xf4
.globl stub_version
stub_version:
    .word 0

.org 0xfc
.globl kernel_entry32
kernel_entry32:
    .word 0x0 // Jumps to 0x0 (Top of bianry file) to run kernel