/** @file memmap.h
 *  @brief Memory layout description.
 *
 * Kernels often need to know where things are in RAM and how they
 * started up, if only to know where the code expects the stack to be,
 * or where there is free space that can be used for user
 * applications.
 */

/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_BASE_MEMMAP_H__
#define __NXOS_BASE_MEMMAP_H__

#include "base/types.h"

#define SYMADDR(Symbol) static_cast<const U8*>(&Symbol)

#define DECLSYMS(Symbol, ConstName)					\
  namespace {								\
    extern "C" {							\
      extern const U8 __Symbol_start__;					\
      extern const U8 __Symbol_end__;					\
    }									\
  }									\
  const U8* const kConstNameStart = SYMADDR(__Symbol_start__);		\
  const U8* const kConstNameEnd = SYMADDR(__Symbol_end__);		\
  const U32 kConstNameSize =						\
    static_cast<const U32>(kConstNameEnd - kConstNameStart)

namespace nxos {
  namespace memmap {
    // Indicates the source of the boot. This is more informative than
    // the other constants here, but it can be interesting to know.
    namespace {
      extern "C" {
	extern const U8 __boot_from_samba__;
      }
    }
    const bool kBootedFromSamba =
      static_cast<const bool>(SYMADDR(__boot_from_samba__));
    const bool kBootedFromRom = !kBootedFromSamba;

    // The following constants are defined by GNU ld at the linking
    // phase. They describe the memory map of the NXT in terms of
    // symbols.
    DECLSYMS(ramtext, RAMText);
    DECLSYMS(text, Text);
    DECLSYMS(global_ctors, GlobalCtors);
    DECLSYMS(global_dtors, GlobalDtors);
    DECLSYMS(data, Data);
    DECLSYMS(bss, BSS);
    DECLSYMS(stack, Stack);
    DECLSYMS(rom_userspace, ROMUserspace);
    DECLSYMS(ram_userspace, RAMUserspace);

  } // namespace memmap
}  // namespace nxos

#undef DECLSYMS
#undef SYMADDR

#endif /* __NXOS_BASE_MEMMAP_H__ */
