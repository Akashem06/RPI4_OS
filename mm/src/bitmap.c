/*******************************************************************************************************************************
 * @file   bitmap.c
 *
 * @brief  Bitmap memory manager source file
 *
 * @date   2025-04-07
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "mem_utils.h"

/* Intra-component Headers */
#include "bitmap.h"

ErrorCode bitmap_init(struct Bitmap *bitmap, u8 *data, u32 size_bits) {
    if (bitmap == NULL || data == NULL) {
        return ERR_GEN_INVALID_PARAM;
    }

    bitmap->data = data;
    bitmap->size_bits = size_bits;

    memzero((u64)data, size_bits * 8U);

    return SUCCESS;
}

ErrorCode bitmap_set(struct Bitmap *bitmap, u32 index) {
    if (bitmap == NULL) {
        return ERR_GEN_INVALID_PARAM;
    }

    if (index >= bitmap->size_bits) {
        return ERR_GEN_INVALID_PARAM;
    }

    bitmap->data[index / 8U] |= (1U << (index % 8U));

    return SUCCESS;
}

ErrorCode bitmap_clear(struct Bitmap *bitmap, u32 index) {
    if (bitmap == NULL) {
        return ERR_GEN_INVALID_PARAM;
    }

    if (index >= bitmap->size_bits) {
        return ERR_GEN_INVALID_PARAM;
    }

    bitmap->data[index / 8U] &= ~(1U << (index % 8U));

    return SUCCESS;
}

ErrorCode bitmap_get(struct Bitmap *bitmap, u32 index, bool *value) {
    if (bitmap == NULL || value == NULL) {
        return ERR_GEN_INVALID_PARAM;
    }

    if (index >= bitmap->size_bits) {
        return ERR_GEN_INVALID_PARAM;
    }

    *value = (bitmap->data[index / 8U] & (1U << (index % 8U))) != 0U;

    return SUCCESS;
}

ErrorCode bitmap_find_first_clear(struct Bitmap *bitmap, u32 start_index, u32 *found_index) {
    if (bitmap == NULL || found_index == NULL) {
        return ERR_GEN_INVALID_PARAM;
    }

    if (start_index >= bitmap->size_bits) {
        return ERR_GEN_INVALID_PARAM;
    }

    for (u32 i = start_index; i < bitmap->size_bits; i++) {
        bool value = false;
        bitmap_get(bitmap, i, &value);

        if (value == false) {
            *found_index = i;
            return SUCCESS; 
        }
    }

    return ERR_MEM_OUT_OF_MEMORY;
}
