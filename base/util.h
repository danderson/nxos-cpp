/** @file util.h
 *  @brief Utility functions.
 */

/* Copyright (C) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_BASE_UTIL_H__
#define __NXOS_BASE_UTIL_H__

inline void memset(U8* dst, U8 val, U32 len) {
  while (--len)
    *dst++ = val;
}

#endif  // __NXOS_BASE_UTIL_H__
