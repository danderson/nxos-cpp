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

void nxos_appkernel_main() {
  g_sound.Initialize();

  for (U32 frequency = 600; frequency <= 2400; frequency += 200)
    g_sound.PlayToneSync(frequency, 200);
}
