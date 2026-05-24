/*******************************************************************************************************************************
 * @file   dentry.c
 *
 * @brief  Data entry source code
 *
 * @date   2025-04-06
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */
#include "virt_fs.h"
#include "virt_fs_internal.h"

struct Dentry *alloc_dentry(const char *name, struct Dentry *parent) {
  // Allocate and initialize a new dentry
  // Set the name and parent
  // Return the new dentry or NULL on failure
}

void destroy_dentry(struct Dentry *dentry) {
  // Free the dentry's resources
  // Free the dentry itself
}

int dentry_add_child(struct Dentry *parent, struct Dentry *child) {
  // Add the child to the parent's child list
  // Return 0 on success, error code on failure
}

struct Dentry *dentry_lookup(struct Dentry *parent, const char *name) {
  // Search for a child dentry by name
  // Return the dentry if found, NULL if not found
}
