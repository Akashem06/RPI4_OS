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
  SUCCESS = 0, /**< Success */

  /* General errors */
  ERR_GEN_INVALID_PARAM = -1, /**< Invalid parameter passed */
  ERR_GEN_NO_MEMORY = -2,     /**< Memory allocation failed */
  ERR_GEN_TIMEOUT = -3,       /**< Operation timed out */

  /* File system errors */
  ERR_FS_NO_ENTRY = -4,       /**< No such file or directory */
  ERR_FS_IO = -5,             /**< I/O error */
  ERR_FS_ACCESS = -6,         /**< Permission denied */
  ERR_FS_EXISTS = -7,         /**< File exists */
  ERR_FS_NOT_DIR = -8,        /**< Not a directory */
  ERR_FS_IS_DIR = -9,         /**< Is a directory */
  ERR_FS_NOT_EMPTY = -10,     /**< Directory not empty */
  ERR_FS_NAME_TOO_LONG = -11, /**< Filename too long */

  /* Device errors */
  ERR_DEVICE_NO_DEVICE = -12,   /**< No such device */
  ERR_DEVICE_BUSY = -13,        /**< Device or resource busy */
  ERR_DEVICE_NO_RESPONSE = -14, /**< Device not responding */

  /* Memory errors */
  ERR_MEM_OUT_OF_MEMORY = -15, /**< Out of memory */
  ERR_MEM_INVALID_ADDR = -16,  /**< Invalid address */

  /* IPC errors */
  ERR_IPC_PIPE = -17,  /**< Broken pipe */
  ERR_IPC_AGAIN = -18, /**< Resource temporarily unavailable */

  /* System errors */
  ERR_SYS_INTERRUPTED = -19,   /**< System call interrupted */
  ERR_SYS_NOT_SUPPORTED = -20, /**< Operation not supported */
  ERR_SYS_INVALID_OP = -21,    /**< Invalid operation */
} ErrorCode;

/**
 * @brief   Check if a value is an error
 * @param   err Value to check
 * @return  True if value is an error code
 */
#define IS_ERROR(err) ((err) < 0)

/**
 * @brief   Return if a value is an error
 * @param   err Value to check
 * @return  Returns if value is an error code
 */
#define RETURN_IF_ERROR(err)           \
  do {                                 \
    ErrorCode __err = (err);           \
    if (IS_ERROR(__err)) return __err; \
  } while (0)

/** @} */
