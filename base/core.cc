/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#include "at91sam7s256.h"

#include "base/aic.h"
#include "base/avr.h"
#include "base/power.h"
#include "base/time.h"
#include "base/types.h"


/* nxos_appkernel_main() is the entry point into the custom payload,
 * not included in the NxOS baseplate.
 */
extern void nxos_appkernel_main(void);

namespace nxos {

class Core {
 public:
  void main() {
    init();
    nxos_appkernel_main();
    halt();
  }

 private:
  void init() {
    g_power.Initialize();
    g_aic.Initialize();
    g_time.Initialize();
    g_avr.Initialize();

    g_aic.UnmaskAll();
  }

  void halt() {
    g_avr.PowerOff();
  }
};

Core g_core;

}  // namespace nxos


extern "C" {

#include "debug.h"

void nxos__baseplate_main(void) {
  //ChangeTone();
  nxos::g_core.main();
}

}  // extern "C"
