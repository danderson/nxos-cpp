/** @file driver_decls.h
 *  @brief Preprocessor declarations for device drivers.
 */

/* Copyright (C) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_BASE_DRIVER_DECLS_H__
#define __NXOS_BASE_DRIVER_DECLS_H__

#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&);               \
  void operator=(const TypeName&)

#define DRIVER(TypeName)                          \
  public:                                         \
  static TypeName& get() {                        \
    static TypeName driver;                       \
    return driver;                                \
  }                                               \
  private:                                        \
  DISALLOW_COPY_AND_ASSIGN(TypeName);             \
  TypeName()

#define DRIVER_ACCESSOR(TypeName, AccessorName) \
  inline TypeName& AccessorName() {             \
    return TypeName::get();                     \
  }

#endif  // __NXOS_BASE_DRIVER_DECLS_H__
