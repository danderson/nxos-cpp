/* Copyright (c) 2007,2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#include "at91sam7s256.h"

#include "base/types.h"


/* nxos_appkernel_main() is the entry point into the custom payload,
 * not included in the NxOS baseplate.
 */
extern void nxos_appkernel_main(void);


namespace {

void init() {
//   nx__aic_init();
//   nx_interrupts_enable();
//   nx__systick_init();
//   nx__sound_init();
//   nx__avr_init();

//   /* Delay a little post-init, to let all the drivers settle down. */
//   nx_systick_wait_ms(100);
}

void halt() {
//   nx__avr_power_down();
}

}  // namespace


extern "C" {

void nxos__baseplate_main(void) {
  init();
  nxos_appkernel_main();
  halt();
}

}  // extern "C"
