#pragma once

/*******************************************************************************************************************************
 * @file   virt_fs.h
 *
 * @brief  Virtual File System interface definitions
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

#include <stddef.h>
#include <stdint.h>

/**
 * @brief   Forward declarations of key filesystem structures
 */

struct File;       /**< Represents an open file */
struct Inode;      /**< Represents a file on disk */
struct Dentry;     /**< Represents a directory entry */
struct Mount;      /**< Represents a mounted filesystem */
struct Superblock; /**< Represents a mounted filesystem's control info */

/**
 * @brief   File system type registration
 */
struct FilesystemType {
  const char *name; /**< File system name */
  struct Superblock *(*mount)(struct FilesystemType *, const char *, const void *); /**< */
  void (*kill_sb)(struct Superblock *); /**< Delete and cleanup superblock */
  struct FilesystemType *next;
};

/**
 * @brief   Superblock operations
 */
struct SuperOperations {
  struct Inode *(*alloc_inode)(struct Superblock *sb);
  void (*destroy_inode)(struct Inode *inode);
  void (*write_super)(struct Superblock *sb);
  int (*sync_fs)(struct Superblock *sb);
};

/**
 * @brief   Superblock structure
 */
struct Superblock {
  struct FilesystemType *fs_type;    /**< File system type */
  struct SuperOperations *super_ops; /**< Pointer to the Superblock operations */
  struct Dentry *s_root;             /**< */
  void *s_fs_info;                   /**< Filesystem specific info */
  uint32_t s_magic;                  /**< Filesystem magic number */
  struct Mount *s_mounts;            // List of mounts
};

/**
 * @brief   Inode operations
 */
struct InodeOperations {
  struct Dentry *(*lookup)(struct Inode *, struct Dentry *);
  int (*create)(struct Inode *, struct Dentry *, int);
  int (*link)(struct Dentry *, struct Inode *, struct Dentry *);
  int (*unlink)(struct Inode *, struct Dentry *);
  int (*mkdir)(struct Inode *, struct Dentry *, int);
  int (*rmdir)(struct Inode *, struct Dentry *);
};

/**
 * @brief Extended inode structure
 */
struct Inode {
  uint32_t i_mode;          // File mode
  uint32_t i_uid;           // User ID
  uint32_t i_gid;           // Group ID
  uint32_t i_nlink;         // Number of links
  uint64_t i_size;          // File size
  struct timespec i_atime;  // Access time
  struct timespec i_mtime;  // Modification time
  struct timespec i_ctime;  // Creation time

  struct InodeOperations *i_op;  // Inode operations
  struct FileOperations *i_fop;  // File operations
  struct Superblock *i_sb;       // Superblock
  void *i_private;               // Private data

  struct list_head i_dentry;  // Dentry list
};

/**
 * @brief Directory entry cache
 */
struct Dentry {
  struct Inode *d_inode;       // Associated inode
  struct Dentry *d_parent;     // Parent directory
  const char *d_name;          // Entry name
  struct list_head d_child;    // Child dentries
  struct list_head d_subdirs;  // Subdirectories
};

/**
 * @brief Mount point
 */
struct Mount {
  struct Dentry *mnt_root;    // Root dentry
  struct Superblock *mnt_sb;  // Superblock
  const char *mnt_devname;    // Device name
  struct list_head mnt_list;  // Mount list
};

/* VFS Global State */
extern struct FilesystemType *fs_types;  // List of registered filesystems
extern struct Mount *mount_list;         // List of mounted filesystems

/* Filesystem Registration */
int register_filesystem(struct FilesystemType *fs_type);
int unregister_filesystem(struct FilesystemType *fs_type);

/* Mount Operations */
struct Mount *do_mount(const char *dev_name, const char *dir_name, const char *type,
                       unsigned long flags);
int do_umount(struct Mount *mnt);

/* Path Resolution */
struct Dentry *path_lookup(const char *path, unsigned int flags);
struct Dentry *lookup_create(struct Dentry *dir, const char *name, int flags);