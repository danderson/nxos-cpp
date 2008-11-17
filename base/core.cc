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
  static void main() {
    init();
    nxos_appkernel_main();
    halt();
  }

 private:
  static void init() {
    Power::Initialize();
    AIC::Initialize();
    g_time.Initialize();
    AVR::Initialize();
  }

  static void halt() {}
};

}  // namespace nxos


extern "C" {

void nxos__baseplate_main(void) {
  nxos::Core::main();
}

}  // extern "C"
