#pragma once

// SCTLR_EL1, System Control Register (EL1), Page 2654 of AArch64-Reference-Manual

#define SCTLR_RESERVED                  (3 << 28) | (3 << 22) | (1 << 20) | (1 << 11)
#define SCTLR_EE_LITTLE_ENDIAN          (0 << 25)
#define SCTLR_EOE_LITTLE_ENDIAN         (0 << 24)
#define SCTLR_I_CACHE_DISABLED          (0 << 12)
#define SCTLR_D_CACHE_DISABLED          (0 << 2)
#define SCTLR_MMU_DISABLED              (0 << 0) // Disable memory management unit (MMU)
#define SCTLR_MMU_ENABLED               (1 << 0) // Enable memory management unit (MMU)

#define SCTLR_VALUE_MMU_DISABLED	(SCTLR_RESERVED | SCTLR_EE_LITTLE_ENDIAN | SCTLR_I_CACHE_DISABLED | SCTLR_D_CACHE_DISABLED | SCTLR_MMU_DISABLED)

// HCR_EL2, Hypervisor Configuration Register (EL2), Page 2487 of AArch64-Reference-Manual

#define HCR_RW	    		(1 << 31) // When this is set to 0, it puts EL2 in AArch32
#define HCR_VALUE			HCR_RW // When this is set to 1, it puts EL2 in AArch64 which is what we want

// SCR_EL3, Secure Configuration Register (EL3), Page 2648 of AArch64-Reference-Manual

#define SCR_RESERVED	    (3 << 4) // 
#define SCR_RW				(1 << 10) // When this is set to 0, EL3 is in AArch32, when it is set to 1, EL3 is in AArch64
#define SCR_NS				(1 << 0) // When 0 EL0 and EL1 are in secure state
#define SCR_VALUE	    	(SCR_RESERVED | SCR_RW | SCR_NS)

// SPSR_EL3, Saved Program Status Register (EL3) Page 389 of AArch64-Reference-Manual

#define SPSR_MASK_ALL 		(7 << 6) // We want to mask/disable all interrupts in EL1
#define SPSR_EL1h			(5 << 0) // We want to jump to EL1h for the kernel, so set this as our SP
#define SPSR_VALUE			(SPSR_MASK_ALL | SPSR_EL1h)
