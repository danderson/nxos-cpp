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

#include "base/cpp_decls.h"
#include "base/types.h"

#define SYMADDR(Symbol) static_cast<const U8*>(&Symbol)

#define DECLSYMS_INTERNAL(SymStart, SymEnd, ConstStart,                 \
                          ConstEnd, ConstSize)                          \
  namespace {                                                           \
    extern "C" {                                                        \
    extern const U8 SymStart;                                           \
    extern const U8 SymEnd;                                             \
    }                                                                   \
  }                                                                     \
  const U8* const ConstStart UNUSED = SYMADDR(SymStart);                \
  const U8* const ConstEnd UNUSED = SYMADDR(SymEnd);                    \
  const U32 ConstSize UNUSED =                                          \
      static_cast<const U32>(ConstEnd - ConstStart)


#define DECLSYMS(Symbol, ConstName)                                     \
  DECLSYMS_INTERNAL(__ ## Symbol ## _start__,                           \
                    __ ## Symbol ## _end__,                             \
                    k ## ConstName ## Start,                            \
                    k ## ConstName ## End,                              \
                    k ## ConstName ## Size)

namespace nxos {
namespace memmap {

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
#undef DECLSYMS_INTERNAL
#undef SYMADDR

#endif /* __NXOS_BASE_MEMMAP_H__ */
