/** @file screen.h
 *  @brief Screen driver
 */

/* Copyright (C) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_BASE_SCREEN_H__
#define __NXOS_BASE_SCREEN_H__

#include "base/at91sam7s256.h"

#include "base/aic.h"
#include "base/cpp_decls.h"
#include "base/types.h"

namespace nxos {

class Screen {
 public:
  void Initialize();

  // Screen dimensions.
  static const U32 kScreenWidthPixels = 100;
  static const U32 kScreenHeightPixels = 64;

  // Screen dimensions, in bytes.
  static const U32 kScreenWidthBytes = kScreenWidthPixels;
  static const U32 kScreenHeightBytes = kScreenHeightPixels / 8;

  // Return a raw handle to the display buffer, of type
  // char[kScreenHeightBytes][kScreenWidthBytes].
  char* GetBuffer() { return screen_buffer_; }

  // Set the given pixel to the given value.
  inline void SetPixel(U32 x, U32 y, bool on);

 private:
  typedef enum {
    COMMAND,
    DATA
  } Mode;

  static void ISR();
  void InitializeSPI();
  void SetTXMode(Mode mode);

  static const U32 kExtraBufferPadding = 13;

  U8 screen_buffer_[kScreenHeightBytes * kScreenWidthBytes
                    + kExtraBufferPadding];

  Mode mode_;

  U32 page_;
};

extern Screen g_screen;

void Screen::SetPixel(U32 x, U32 y, bool on) {
  const U32 y_byte = y / 8;
  const U32 y_bit = y % 8;
  if (on)
    screen_buffer_[y_byte * kScreenHeightBytes + x] |= 1 << y_bit;
  else
    screen_buffer_[y_byte * kScreenHeightBytes + x] &= ~(1 << y_bit);
}

}  // namespace nxos

#endif  // __NXOS_BASE_SCREEN_H__
