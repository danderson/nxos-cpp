/* Copyright (C) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#include "base/sound.h"
#include "base/time.h"

using nxos::g_sound;
using nxos::g_time;

void nxos_appkernel_main() {
  g_sound.Initialize();

  while (true) {
    g_sound.PlayToneSync(2000, 1000);
    g_time.Sleep(1000);
  }
}
