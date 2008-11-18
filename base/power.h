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
  inline void EnablePeripheral(U32 peripheral_id);
  inline void DisablePeripheral(U32 peripheral_id);

 private:
  inline void Initialize();

  friend class Core;
};

void Power::Initialize() {
  // Power down all peripherals and clock signals except the CPU.
  *AT91C_PMC_PCDR = 0x7FF4;
  *AT91C_PMC_SCDR = (AT91C_PMC_UDP | AT91C_PMC_PCK0 |
                     AT91C_PMC_PCK1 | AT91C_PMC_PCK2);
}

void Power::EnablePeripheral(U32 peripheral_id) {
  *AT91C_PMC_PCER = (1 << peripheral_id);
  if (peripheral_id == AT91C_ID_UDP)
    *AT91C_PMC_SCER = AT91C_PMC_UDP;
}

void Power::DisablePeripheral(U32 peripheral_id) {
  *AT91C_PMC_PCDR = (1 << peripheral_id);
  if (peripheral_id == AT91C_ID_UDP)
    *AT91C_PMC_SCDR = AT91C_PMC_UDP;
}

extern Power g_power;

}

#endif  // __NXOS_BASE_POWER_H__
