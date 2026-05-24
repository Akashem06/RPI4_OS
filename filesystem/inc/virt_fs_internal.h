#pragma once

/*******************************************************************************************************************************
 * @file   virt_fs_internal.h
 *
 * @brief  Virtual File System internal interface definitions
 *
 * @date   2025-04-06
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
 * @brief    OS File system
 * @{
 */

/* Inode operations */
int init_inode(struct Inode *inode, struct Superblock *sb, uint32_t mode);
struct Inode *alloc_inode(struct Superblock *sb, uint32_t mode);
void destroy_inode(struct Inode *inode);
int inode_lookup(struct Inode *dir, struct Dentry *dentry);

/* Dentry operations */
struct Dentry *alloc_dentry(const char *name, struct Dentry *parent);
void destroy_dentry(struct Dentry *dentry);
int dentry_add_child(struct Dentry *parent, struct Dentry *child);
struct Dentry *dentry_lookup(struct Dentry *parent, const char *name);

/* Path operations */
char *get_next_path_component(const char **path);
struct Dentry *follow_path(struct Dentry *root, const char *path);

/* Mount operations */
struct Mount *alloc_mount(struct Dentry *root, struct Superblock *sb, const char *dev_name);
void destroy_mount(struct Mount *mnt);
struct Mount *lookup_mount(const char *path);

/* Superblock operations */
struct Superblock *alloc_super(struct FilesystemType *fs_type);
void destroy_super(struct Superblock *sb);

/* File descriptor operations */
int fd_alloc(void);
int fd_assign(int fd, struct File *file);
struct File *fd_to_file(int fd);
void fd_release(int fd);

/** @} */
