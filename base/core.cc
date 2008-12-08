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
#include "base/memmap.h"
#include "base/power.h"
#include "base/time.h"
#include "base/types.h"


// nxos_appkernel_main() is the entry point into the custom payload,
// not included in the NxOS baseplate.
extern void nxos_appkernel_main(void);

namespace nxos {

class Core {
 public:
  static void Init() {
    g_power.Initialize();
    g_aic.Initialize();
    g_time.Initialize();
    g_avr.Initialize();

    g_aic.UnmaskAll();
  }

  static void Halt() {
    g_avr.PowerOff();
  }

  static void RunGlobalCtors() {
    const closure_t* ctors =
        reinterpret_cast<const closure_t*>(memmap::kGlobalCtorsStart);
    const closure_t* ctors_end =
        reinterpret_cast<const closure_t*>(memmap::kGlobalCtorsEnd);

    while (ctors != ctors_end) {
      (*ctors)();
      ++ctors;
    }
  }

  static void RunGlobalDtors() {
    const closure_t *dtors =
        reinterpret_cast<const closure_t*>(memmap::kGlobalDtorsStart);
    const closure_t *dtors_end =
        reinterpret_cast<const closure_t*>(memmap::kGlobalDtorsEnd);

    while (dtors != dtors_end) {
      (*dtors)();
      ++dtors;
    }
  }
};

}  // namespace nxos


extern "C" {

void nxos__baseplate_main(void) {
  nxos::Core::Init();
  nxos::Core::RunGlobalCtors();
  nxos_appkernel_main();
  nxos::Core::RunGlobalDtors();
  nxos::Core::Halt();
}

}  // extern "C"
