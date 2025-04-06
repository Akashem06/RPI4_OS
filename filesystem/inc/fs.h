#pragma once

/*******************************************************************************************************************************
 * @file   fs.h
 *
 * @brief  File system global definitions
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */
#include <stddef.h>
#include <stdint.h>

/* Inter-component Headers */
#include "common.h"

/* Intra-component Headers */

/**
 * @defgroup FileSystem File system interface
 * @{
 */

#define FS_MAXPATH 256 /**< Maximum length of a full path */
#define FS_MAXNAME 255 /**< Maximum length of a single file name */

/**
 * @brief   File Type Bits (stored in file_metadata)
 * @details These bits define what type of file we're dealing with
 */

#define S_FILETYPE_BITFIELD 00170000        /**< Bit mask for the file type bit fields*/
#define S_FILETYPE_REGULAR 0100000          /**< Regular file */
#define S_FILETYPE_DIRECTORY 0040000        /**< Directory */
#define S_FILETYPE_CHARACTER_DEVICE 0020000 /**< Character device */
#define S_FILETYPE_BLOCK_DEVICE 0060000     /**< Block device */

/** @brief  True if it is a regular file */
#define S_IS_REG_FILE(m) (((m) & S_FILETYPE_BITFIELD) == S_FILETYPE_REGULAR)
/** @brief  True if it is a directory file */
#define S_IS_DIR_FILE(m) (((m) & S_FILETYPE_BITFIELD) == S_FILETYPE_DIRECTORY)
/** @brief  True if it is a character device file */
#define S_IS_CHR_FILE(m) (((m) & S_FILETYPE_BITFIELD) == S_FILETYPE_CHARACTER_DEVICE)
/** @brief  True if it is a block device file */
#define S_IS_BLK_FILE(m) (((m) & S_FILETYPE_BITFIELD) == S_FILETYPE_BLOCK_DEVICE)

/**
 * @brief   Permission Bits (stored in file_metadata)
 * @details These define who can read, write, or execute the file
 */

#define S_FILEUSER_RWX 00700 /**< User (owner) has read, write, and execute permission */
#define S_FILEUSER_R 00400   /**< User has read permission */
#define S_FILEUSER_W 00200   /**< User has write permission */
#define S_FILEUSER_X 00100   /**< User has execute permission */

#define S_FILEGROUP_RWX 00070 /**< Group has read, write, and execute permission */
#define S_FILEGROUP_R 00040   /**< Group has read permission */
#define S_FILEGROUP_W 00020   /**< Group has write permission */
#define S_FILEGROUP_X 00010   /**< Group has execute permission */

#define S_FILEOTHERS_RWX 00007 /**< Others have read, write, and execute permission */
#define S_FILEOTHERS_R 00004   /**< Others have read permission */
#define S_FILEOTHERS_W 00002   /**< Others have write permission */
#define S_FILEOTHERS_X 00001   /**< Others have execute permission */

/**
 * @brief   File open flags
 * @details Used when opening files to specify access mode
 */

#define O_RDONLY 00000000 /**< Open for reading only */
#define O_WRONLY 00000001 /**< Open for writing only */
#define O_RDWR 00000002   /**< Open for reading and writing */
#define O_CREAT 00000100  /**< Create file if it doesn't exist */
#define O_EXCL 00000200   /**< Fail if file already exists */
#define O_TRUNC 00001000  /**< Truncate file to zero length */
#define O_APPEND 00002000 /**< Append to file */

/**
 * @brief   Forward declarations of key filesystem structures
 */

struct File;       /**< Represents an open file */
struct Inode;      /**< Represents a file on disk */
struct Dentry;     /**< Represents a directory entry */
struct Mount;      /**< Represents a mounted filesystem */
struct Superblock; /**< Represents a mounted filesystem's control info */

/**
 * @brief   Time specification structure
 * @details Used for file timestamps (access, modify, change times)
 */
struct Timespec {
  u64 tv_sec;  /**< Seconds */
  u64 tv_nsec; /**< Nanoseconds */
};

/** @} */
