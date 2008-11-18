/** @file sound.h
 *  @brief Sound driver.
 */

/* Copyright (C) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_BASE_SOUND_H__
#define __NXOS_BASE_SOUND_H__

#include "base/time.h"
#include "base/types.h"

namespace nxos {

class Sound {
 public:
  void Initialize();

  void PlayTone(U32 frequency, U32 ms);
  inline void PlayToneSync(U32 frequency, U32 ms);

 private:
  volatile U32 tone_cycles_;

  static void ISR();
};

void Sound::PlayToneSync(U32 frequency, U32 ms) {
  PlayTone(frequency, ms);
  g_time.Sleep(ms);
}

extern Sound g_sound;

}  // namespace nxos

#endif  // __NXOS_BASE_SOUND_H__
