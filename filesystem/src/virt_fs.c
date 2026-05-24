/*******************************************************************************************************************************
 * @file   virt_fs.c
 *
 * @brief  Virtual File System interface source code
 *
 * @date   2025-04-06
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "virt_fs.h"

struct FilesystemType *fs_types = NULL;  // List of registered filesystems
struct Mount *mount_list = NULL;         // List of mounted filesystems
struct File *fd_table[MAX_OPEN_FILES];

struct Mount *root_mount = NULL;
struct Dentry *root_dentry = NULL;

int register_filesystem(struct FilesystemType *fs_type) {
  // Add fs_type to the fs_types list
  // Return 0 on success, error code on failure
}

int unregister_filesystem(struct FilesystemType *fs_type) {
  // Remove fs_type from the fs_types list
  // Return 0 on success, error code on failure
}

struct FilesystemType *find_filesystem(const char *name) {
  // Search for fs_type in the fs_types list by name
  // Return fs_type if found, NULL if not found
}

int vfs_mount(const char *dev_name, const char *dir_name, const char *type, uint32_t flags, const void *data) {
  // Find filesystem type
  // Call mount operation of that filesystem type
  // Add new mount to mount_list
  // Return 0 on success, error code on failure
}

int vfs_unmount(const char *dir_name) {
  // Find mount by dir_name
  // Call unmount operation
  // Remove mount from mount_list
  // Return 0 on success, error code on failure
}

struct Dentry *path_lookup(const char *path) {
  // Start from root dentry or current directory
  // Walk the path components
  // Return the final dentry or NULL on failure
}
