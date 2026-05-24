/*******************************************************************************************************************************
 * @file   inode.c
 *
 * @brief  Inode source code
 *
 * @date   2025-04-06
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "virt_fs.h"
#include "virt_fs_internal.h"

int init_inode(struct Inode *inode, struct Superblock *sb, uint32_t mode) {
  // Initialize inode fields
  // Set default operations
  // Return 0 on success, error code on failure
}

struct Inode *alloc_inode(struct Superblock *sb, uint32_t mode) {
  // Use superblock's alloc_inode if available
  // Otherwise allocate a new inode
  // Initialize the inode
  // Return the new inode or NULL on failure
}

void destroy_inode(struct Inode *inode) {
  // Use superblock's destroy_inode if available
  // Otherwise free the inode
}

int inode_lookup(struct Inode *dir, struct Dentry *dentry) {
  // Call the directory's lookup operation
  // Return 0 on success, error code on failure
}

// Implementation of vfs_stat from the header
int vfs_stat(const char *path, struct inode *buf) {
  // Look up the path
  // Copy inode data to the buffer
  // Return 0 on success, error code on failure
}
