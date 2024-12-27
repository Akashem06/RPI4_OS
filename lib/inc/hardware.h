#pragma once

/*******************************************************************************************************************************
 * @file   hardware.h
 *
 * @brief  Hardware-specific includes
 *
 * @date   2024-12-27
 * @author Aryan Kashem
 *******************************************************************************************************************************/

/* Standard library Headers */

/* Inter-component Headers */

/* Intra-component Headers */

/**
 * @defgroup Hardware Hardware includes
 * @{
 */

#if RPI_VERSION == 4U
#include "arm64_barrier.h"
#include "bcm2711.h"
#include "bcm2711_periph_io.h"
#endif

/** @} */
