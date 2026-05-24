#pragma once

/*******************************************************************************************************************************
 * @file   assert.h
 *
 * @brief  Runtime assertion and bug-trap macros
 *
 * @date   2026-05-24
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "log.h"

/* Intra-component Headers */

/**
 * @defgroup Debug Debug helpers
 * @{
 */

/** @brief  Fatal hang, defined in entry.S, dumps nothing and loops forever */
void err_hang(void);

/** @brief  Trap an unconditional bug with a message */
#define BUG(msg)                                            \
  do {                                                      \
    log("BUG: %s at %s:%d\n\r", (msg), __FILE__, __LINE__); \
    err_hang();                                             \
  } while (0)

/** @brief  Trap if a condition does not hold */
#define ASSERT(cond)                                                      \
  do {                                                                    \
    if (!(cond)) {                                                        \
      log("ASSERT FAILED: %s at %s:%d\n\r", #cond, __FILE__, __LINE__);   \
      err_hang();                                                         \
    }                                                                     \
  } while (0)

/** @} */
