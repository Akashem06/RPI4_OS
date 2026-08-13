#pragma once

/*******************************************************************************************************************************
 * @file   page.h
 *
 * @brief  Single-page allocation facade over the buddy allocator
 *
 * @date   2026-08-02
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */
#include "common.h"

/* Intra-component Headers */

/**
 * @defgroup MemoryManager Memory Manager
 * @{
 */

/**
 * @brief   Allocate a single zeroed-capable page from the buddy allocator
 * @return  Virtual address of the page, or NULL if none are free
 */
void *get_free_page(void);

/**
 * @brief   Return a page previously handed out by get_free_page
 * @param   p Virtual address of the page to free
 */
void free_page(u64 p);

/** @} */
