# Kernel Memory Allocator Design Documentation

## Overview

My kernel memory allocator shall efficently manage both small and large memory allocations. It combines two techniques, inspired by Linux:

- **Buddy Allocation:** Manages memory in units of pages (4096 Kbytes for me), handling large contiguous allocations through power-of-two sized blocks
  - The buddy allocator splits the memory pool into powers of 2 when data of a given size is requested
  - The buddy allocator is responsible for merging contiguous blocks of memory
  - The buddy allocator uses a free list to store the heads of linked-lists, each of which contain free blocks of memory of a certain size
- **Slab Allocation:** Built on top of the buddy system, it subdivides pages into smaller, fixed-size objects to efficiently serve frequent small allocations

## Design Goals

- **Efficiency:** Quickly allocate and deallocate memory with minimal overhead.
- **Low Fragmentation:** Use the buddy system to maintain contiguous blocks and the slab system to minimize internal fragmentation for small allocations
- **Scalability:** Handle a wide range of allocation sizes—from small kernel objects to large contiguous memory blocks
- **Robustness:** Incorporate metadata (e.g., magic numbers) to validate allocations and help detect errors

## Theoretical Background

### Buddy Allocation

The buddy allocation system is used to manage memory at the page level:

- **Basic Idea:**  
  Memory is divided into blocks whose sizes are powers of two. A block of order _k_ represents \(2^k\) pages.
  
- **Splitting and Merging:**  
  - **Splitting:** When a smaller block is needed, a larger block is split into two buddies. One buddy is used, and the other is inserted back into the free list
  - **Merging:** When a block is freed, its buddy is checked. If the buddy is also free and of the same order, the two are merged into a larger block. This minimizes fragmentation
  
- **Free Lists:**  
  An array of free lists is maintained where each index corresponds to a block order (size). Blocks are added or removed from these lists during allocation and deallocation

### Slab Allocation

The slab allocator is used for managing small objects:

- **Slab Structure:**  
  - A **slab** is a contiguous memory region (usually at least one page) subdivided into fixed-size objects.
  - Each slab is designed for objects of a particular size class. This allows for multiple allocations of the same size to be done in the same blob of memory
  
- **Slab Object:**  
  Each allocated unit (object) within a slab has an associated header (`SlabObject`) that contains:
  - A **magic number** for validation
  - The **size** of the allocation
  - A pointer to its **parent slab**
  - A link for a free list of objects

- **Slab Cache:**  
  Slabs are organized into caches based on the object size. The cache index is determined by rounding the requested size up to the nearest multiple of a defined **MIN_SLAB_SIZE**. For example, if `MIN_SLAB_SIZE` is 8 bytes:
  - A request for 4 bytes is rounded up to 8 bytes
  - The index is computed as `(size / MIN_SLAB_SIZE) - 1`

- **Design Decision – MIN_SLAB_SIZE:**  
  Although slabs occupy at least one full page, the minimum object size is set to 8 bytes to ensure proper alignment (especially on 64-bit systems) and to provide enough room for metadata. Supporting objects smaller than 8 bytes is typically avoided due to potential alignment issues and excessive metadata overhead.

## Implementation Details

### Memory Pool Initialization

- **Memory Pool Setup:**  
  A memory pool is provided (or created as a dummy pool) to back the entire allocator. It is divided into pages
  
- **Buddy Allocator Initialization:**  
  The allocator divides the memory pool into pages and organizes them into free lists based on the maximum order possible given the pool size

### Allocation Flow

1. **Buddy Allocation for Large Requests:**
   - For allocations larger than `MAX_SLAB_SIZE` (typically a quarter page), the buddy system allocates a contiguous block of pages.
   - A header (`SlabObject`) is prepended to track allocation metadata
  
2. **Slab Allocation for Small Requests:**
   - The requested size is first rounded up to a multiple of `MIN_SLAB_SIZE`
   - The appropriate slab cache is selected using the computed index
   - If a suitable slab does not exist or is full, a new slab is created by allocating one or more pages via the buddy allocator
   - An object is then removed from the slab’s free list and returned to the user

### Freeing Memory

- **Freeing Buddy Allocations:**  
  If the allocated block was a direct buddy allocation (i.e., not part of a slab), it is returned to the buddy free list. The allocator checks for the buddy block to merge free blocks
  
- **Freeing Slab Allocations:**  
  If the allocation comes from a slab:
  - The object is returned to its parent slab’s free list
  - Optionally, if a slab becomes completely free, it could be reclaimed to reduce memory usage

## Design Decisions and Tradeoffs

- **Combining Buddy and Slab Allocators:**  
  - **Buddy System:**  
    Offers efficient management of large contiguous memory areas and reduces external fragmentation through merging
  - **Slab System:**  
    Minimizes internal fragmentation and speeds up allocations for small objects by reusing memory within pre-allocated slabs
  
- **Alignment and Minimum Object Size:**  
  - Setting `MIN_SLAB_SIZE` to 8 bytes ensures that even small allocations are aligned and have sufficient space for the metadata
  
- **Use of Magic Numbers:**  
  - A magic number (e.g., `0xDEADBEEF`) is used in each allocated object to validate memory and detect corruption during debugging
