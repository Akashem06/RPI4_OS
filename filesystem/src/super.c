/*******************************************************************************************************************************
 * @file   super.c
 *
 * @brief  Superblock source code
 *
 * @date   2025-04-06
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "virt_fs.h"
#include "virt_fs_internal.h"

struct Superblock *alloc_super(struct FilesystemType *fs_type) {
  // Allocate and initialize a new superblock
  // Set the filesystem type
  // Return the new superblock or NULL on failure
}

void destroy_super(struct Superblock *sb) {
  // Call the filesystem type's kill_sb operation
  // Free the superblock's resources
  // Free the superblock itself
}

// Generic implementation for sync_fs operation
int generic_sync_fs(struct Superblock *sb) {
  // Sync filesystem data
  // Return 0 on success, error code on failure
}

// Generic implementation for write_super operation
void generic_write_super(struct Superblock *sb) {
  // Write superblock data to storage
}

// Helper function to read a superblock from disk
struct Superblock *read_super(struct FilesystemType *fs_type, const char *dev_name, const void *data) {
  // Allocate a new superblock
  // Read superblock data from device
  // Initialize superblock fields
  // Return the superblock or NULL on failure
}
