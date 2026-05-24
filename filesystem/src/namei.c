/*******************************************************************************************************************************
 * @file   namei.c
 *
 * @brief  Name source code
 *
 * @date   2025-04-06
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "virt_fs.h"
#include "virt_fs_internal.h"

char *get_next_path_component(const char **path) {
  // Extract the next component from the path
  // Update the path pointer
  // Return the component or NULL if end of path
}

struct Dentry *follow_path(struct Dentry *root, const char *path) {
  // Walk the path components starting from root
  // Return the final dentry or NULL on failure
}

int vfs_mkdir(const char *path, uint32_t mode) {
  // Split path into parent path and new directory name
  // Look up the parent path
  // Call the mkdir operation on the parent directory
  // Return 0 on success, error code on failure
}

int vfs_rmdir(const char *path) {
  // Look up the path
  // Call the rmdir operation
  // Return 0 on success, error code on failure
}

int vfs_link(const char *oldpath, const char *newpath) {
  // Look up both paths
  // Call the link operation
  // Return 0 on success, error code on failure
}

int vfs_unlink(const char *path) {
  // Look up the path
  // Call the unlink operation
  // Return 0 on success, error code on failure
}
