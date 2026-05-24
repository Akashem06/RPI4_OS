#pragma once

/*******************************************************************************************************************************
 * @file   bitmap.h
 *
 * @brief  Bitmap memory manager header file
 *
 * @date   2025-04-07
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */
#include <stdbool.h>
#include <stddef.h>

/* Inter-component Headers */
#include "common.h"
#include "error.h"
#include "hardware.h"

/* Intra-component Headers */

/**
 * @defgroup MemoryManager OS Memory Manager
 * @brief    OS Memory Manager using Buddy Allocator and Slab Allocator
 * @{
 */


/**
 * @brief   Memory allocator Bitmap
 */
struct Bitmap {
    u8 *data;       /**< Pointer to the bitmap data */
    u32 size_bits;  /**< Size of the bitmap in bits */
};

/**
 * @brief   Initialize the bitmap
 * @param   bitmap Pointer to the bitmap object
 * @param   data Pointer to the start of bitmap data
 * @param   size_bits Size of the bitmap in bits
 * @return  SUCCESS if bitmap is initialized succesfully
 *          ERR_GEN_INVALID_PARAM if an invalid parameter is provided
 */
ErrorCode bitmap_init(struct Bitmap *bitmap, u8 *data, u32 size_bits);

/**
 * @brief   Set an index in the bitmap
 * @param   bitmap Pointer to the bitmap object
 * @param   index Index to update
 * @return  SUCCESS if the index is set succesfully
 *          ERR_GEN_INVALID_PARAM if an out of range index is provided
 */
ErrorCode bitmap_set(struct Bitmap *bitmap, u32 index);

/**
 * @brief   Clear an index in the bitmap
 * @param   bitmap Pointer to the bitmap object
 * @param   index Index to remove
 * @return  SUCCESS if the index is cleared succesfully
 *          ERR_GEN_INVALID_PARAM if an out of range index is provided
 */
ErrorCode bitmap_clear(struct Bitmap *bitmap, u32 index);

/**
 * @brief   Clear an index in the bitmap
 * @param   bitmap Pointer to the bitmap object
 * @param   index Index to remove
 * @param   value Pointer to the value to be updated
 * @return  SUCCESS if the index is retrieved succesfully
 *          ERR_GEN_INVALID_PARAM if an out of range index is provided
 */
ErrorCode bitmap_get(struct Bitmap *bitmap, u32 index, bool *value);

/**
 * @brief   Finds the first clear index in the bitmap
 * @param   bitmap Pointer to the bitmap object
 * @param   start_index Index to start the search from
 * @param   found_index Pointer to the value to be updated with the index of first cleared bit
 * @return  SUCCESS if the index is retrieved succesfully
 *          ERR_GEN_INVALID_PARAM if an out of range index is provided
 */
ErrorCode bitmap_find_first_clear(struct Bitmap *bitmap, u32 start_index, u32 *found_index);

/** @} */
