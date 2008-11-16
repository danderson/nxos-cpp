/** @file cpp_decls.h
 *  @brief Preprocessor declarations for C++ code.
 */

/* Copyright (C) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_BASE_CPP_DECLS_H__
#define __NXOS_BASE_CPP_DECLS_H__

#define DISALLOW_COPY_AND_ASSIGN(TypeName)      \
  TypeName(const TypeName&);                    \
  void operator=(const TypeName&)

#define DISALLOW_CONSTRUCTION(TypeName)         \
  TypeName();                                   \
  DISALLOW_COPY_AND_ASSIGN(TypeName)

#endif  // __NXOS_BASE_CPP_DECLS_H__
