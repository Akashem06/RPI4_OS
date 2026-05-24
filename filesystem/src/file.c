/*******************************************************************************************************************************
 * @file   file.c
 *
 * @brief  File source code
 *
 * @date   2025-04-06
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "virt_fs.h"
#include "virt_fs_internal.h"

int vfs_open(const char *path, int flags, uint32_t mode) {
  // Look up the path
  // Allocate a file descriptor
  // Create a new file structure
  // Return the file descriptor or error code
}

int vfs_close(int fd) {
  // Get the file from the file descriptor
  // Call the close operation
  // Free the file descriptor
  // Return 0 on success, error code on failure
}

ssize_t vfs_read(int fd, void *buf, size_t count) {
  // Get the file from the file descriptor
  // Call the read operation
  // Return the number of bytes read or error code
}

ssize_t vfs_write(int fd, const void *buf, size_t count) {
  // Get the file from the file descriptor
  // Call the write operation
  // Return the number of bytes written or error code
}

loff_t vfs_seek(int fd, loff_t offset, int whence) {
  // Get the file from the file descriptor
  // Adjust the file position
  // Return the new position or error code
}
