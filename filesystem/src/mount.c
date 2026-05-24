/*******************************************************************************************************************************
 * @file   mount.c
 *
 * @brief  Mount source code
 *
 * @date   2025-04-06
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "virt_fs.h"
#include "virt_fs_internal.h"

struct Mount *alloc_mount(struct Dentry *root, struct Superblock *sb, const char *dev_name) {
  // Allocate and initialize a new mount
  // Set the root, superblock, and device name
  // Return the new mount or NULL on failure
}

void destroy_mount(struct Mount *mnt) {
  // Remove the mount from the mount list
  // Free the mount's resources
  // Free the mount itself
}

struct Mount *lookup_mount(const char *path) {
  // Search for a mount by path
  // Return the mount if found, NULL if not found
}

// Helper for vfs_mount
int do_mount(struct FilesystemType *fs_type, const char *dev_name, const char *dir_name, uint32_t flags, const void *data) {
  // Call the filesystem type's mount operation
  // Create a new Mount structure
  // Add the mount to the mount list
  // Return 0 on success, error code on failure
}

// Helper for vfs_unmount
int do_unmount(struct Mount *mnt) {
  // Call the filesystem type's kill_sb operation
  // Remove the mount from the mount list
  // Destroy the mount
  // Return 0 on success, error code on failure
}
