/** @file power.h
 *  @brief Power management driver.
 */

/* Copyright (C) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_BASE_POWER_H__
#define __NXOS_BASE_POWER_H__

#include "base/at91sam7s256.h"

#include "base/cpp_decls.h"
#include "base/types.h"

namespace nxos {

class Power {
 public:
  static inline void EnablePeripheral(U32 peripheral_id);
  static inline void DisablePeripheral(U32 peripheral_id);

 private:
  static inline void Initialize();

  friend class Core;

  DISALLOW_CONSTRUCTION(Power);
};

void Power::Initialize() {
  // Power down all peripherals and clock signals except the CPU.
  *AT91C_PMC_PCDR = 0x7FF4;
  *AT91C_PMC_SCDR = (AT91C_PMC_UDP | AT91C_PMC_PCK0 |
                     AT91C_PMC_PCK1 | AT91C_PMC_PCK2);
}

void Power::EnablePeripheral(U32 peripheral_id) {
  *AT91C_PMC_PCER = peripheral_id;
  if (peripheral_id == AT91C_ID_UDP)
    *AT91C_PMC_SCER = AT91C_PMC_UDP;
}

void Power::DisablePeripheral(U32 peripheral_id) {
  *AT91C_PMC_PCDR = peripheral_id;
  if (peripheral_id == AT91C_ID_UDP)
    *AT91C_PMC_SCDR = AT91C_PMC_UDP;
}

}

#endif  // __NXOS_BASE_POWER_H__
