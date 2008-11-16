/** @file types.h
 *  @brief Basic type definitions for the Arm7 platform.
 */

/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_BASE_TYPES_H__
#define __NXOS_BASE_TYPES_H__

/** @addtogroup typesAndUtils */
/*@{*/

typedef unsigned char U8; /**< Unsigned 8-bit integer. */
typedef signed char S8; /**< Signed 8-bit integer. */
typedef unsigned short U16; /**< Unsigned 16-bit integer. */
typedef signed short S16; /**< Signed 16-bit integer. */
typedef unsigned long U32; /**< Unsigned 32-bit integer. */
typedef signed long S32; /**< Signed 32-bit integer. */

#ifndef NULL
/** Definition of the NULL pointer. */
#define NULL 0
#endif

/** A function that takes no arguments and returns nothing. */
typedef void (*closure_t)(void);

/*@}*/

#endif
