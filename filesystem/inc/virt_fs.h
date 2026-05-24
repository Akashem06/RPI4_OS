#pragma once

/*******************************************************************************************************************************
 * @file   virt_fs.h
 *
 * @brief  Virtual File System interface definitions
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
  const char *name;                                                                 /**< File system name */
  struct Superblock *(*mount)(struct FilesystemType *, const char *, const void *); /**< */
  void (*kill_sb)(struct Superblock *);                                             /**< Delete and cleanup superblock */
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

typedef int64_t loff_t;  /**< Large file offset type */
typedef int64_t ssize_t; /**< Signed size type */

/* VFS Global State */
extern struct FilesystemType *fs_types;  // List of registered filesystems
extern struct Mount *mount_list;         // List of mounted filesystems

/**
 * @brief   Register a filesystem type
 * @param   fs_type Filesystem type structure
 * @return  0 on success, negative error code on failure
 */
int register_filesystem(struct FilesystemType *fs_type);

/**
 * @brief   Unregister a filesystem type
 * @param   fs_type Filesystem type structure
 * @return  0 on success, negative error code on failure
 */
int unregister_filesystem(struct FilesystemType *fs_type);

/**
 * @brief   Find a filesystem type by name
 * @param   name Name of the filesystem type
 * @return  Pointer to the filesystem type, or NULL if not found
 */
struct FilesystemType *find_filesystem(const char *name);

/**
 * @brief   Mount a filesystem
 * @param   dev_name Device name
 * @param   dir_name Directory name (mount point)
 * @param   type Filesystem type name
 * @param   flags Mount flags
 * @param   data Mount options
 * @return  0 on success, negative error code on failure
 */
int vfs_mount(const char *dev_name, const char *dir_name, const char *type, uint32_t flags, const void *data);

/**
 * @brief   Unmount a filesystem
 * @param   dir_name Directory name (mount point)
 * @return  0 on success, negative error code on failure
 */
int vfs_unmount(const char *dir_name);

/**
 * @brief   Open a file
 * @param   path Path to the file
 * @param   flags Open flags
 * @param   mode File mode for creation
 * @return  File descriptor on success, negative error code on failure
 */
int vfs_open(const char *path, int flags, uint32_t mode);

/**
 * @brief   Close a file
 * @param   fd File descriptor
 * @return  0 on success, negative error code on failure
 */
int vfs_close(int fd);

/**
 * @brief   Read from a file
 * @param   fd File descriptor
 * @param   buf Buffer to read into
 * @param   count Number of bytes to read
 * @return  Number of bytes read, or negative error code on failure
 */
ssize_t vfs_read(int fd, void *buf, size_t count);

/**
 * @brief   Write to a file
 * @param   fd File descriptor
 * @param   buf Buffer to write from
 * @param   count Number of bytes to write
 * @return  Number of bytes written, or negative error code on failure
 */
ssize_t vfs_write(int fd, const void *buf, size_t count);

/**
 * @brief   Seek in a file
 * @param   fd File descriptor
 * @param   offset Offset to seek to
 * @param   whence SEEK_SET, SEEK_CUR, or SEEK_END
 * @return  New file position, or negative error code on failure
 */
loff_t vfs_seek(int fd, loff_t offset, int whence);

/**
 * @brief   Create a directory
 * @param   path Path to the directory
 * @param   mode Directory mode
 * @return  0 on success, negative error code on failure
 */
int vfs_mkdir(const char *path, uint32_t mode);

/**
 * @brief   Remove a directory
 * @param   path Path to the directory
 * @return  0 on success, negative error code on failure
 */
int vfs_rmdir(const char *path);

/**
 * @brief   Create a hard link
 * @param   oldpath Path to the existing file
 * @param   newpath Path to the new link
 * @return  0 on success, negative error code on failure
 */
int vfs_link(const char *oldpath, const char *newpath);

/**
 * @brief   Remove a link
 * @param   path Path to the link
 * @return  0 on success, negative error code on failure
 */
int vfs_unlink(const char *path);

/**
 * @brief   Get file status
 * @param   path Path to the file
 * @param   buf Buffer to fill with status information
 * @return  0 on success, negative error code on failure
 */
int vfs_stat(const char *path, struct inode *buf);

/**
 * @brief   Lookup a path and return its dentry
 * @param   path Path to look up
 * @return  Dentry pointer on success, NULL on failure
 */
struct dentry *path_lookup(const char *path);

/** @} */
