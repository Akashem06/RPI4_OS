#pragma once

/*******************************************************************************************************************************
 * @file   kernel.h
 *
 * @brief  Shared kernel boot API used by the example entry points
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @brief   Bring up the core kernel services common to every entry point
 * @details Initializes the UART, logging, the page/heap allocator and the
 *          exception vectors, then logs the boot banner. Each example calls this
 *          first from its own kernel_main before doing demo-specific work.
 */
void kernel_boot(void);
