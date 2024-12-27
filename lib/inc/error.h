#pragma once

/*******************************************************************************************************************************
 * @file   error.h
 *
 * @brief  Error definitions
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup Error Error definitions
 * @{
 */

typedef enum {
  SUCCESS,
  ERR_INVALID_PARAM = -1,
  ERR_NO_MEMORY = -2,
  ERR_TIMEOUT = -3,
} ErrorCode;

/** @} */
