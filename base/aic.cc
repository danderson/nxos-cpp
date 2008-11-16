/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#include "at91sam7s256.h"

#include "base/types.h"

#include "base/aic.h"

extern "C" {
// Defined in aic_asm.h
extern void nxos__unhandled_exception();
}

namespace nxos {

void AICDriver::Initialize() {
  MaskAll();

  // If we're coming from a warm boot, the AIC may be in a weird
  // state. Do some cleaning up to bring the AIC back into a known
  // state:
  // - All interrupt lines disabled,
  // - No interrupt lines handled by the FIQ handler,
  // - No pending interrupts,
  // - AIC idle, not handling an interrupt.
  *AT91C_AIC_IDCR = 0xFFFFFFFF;
  *AT91C_AIC_FFDR = 0xFFFFFFFF;
  *AT91C_AIC_ICCR = 0xFFFFFFFF;
  *AT91C_AIC_EOICR = 1;

  // Enable debug protection. This is necessary for JTAG debugging, so
  // that the hardware debugger can read AIC registers without
  // triggering side-effects.
  *AT91C_AIC_DCR = 1;

  // Set default handlers for all interrupt lines.
  for (int i = 0; i < 32; i++) {
    AT91C_AIC_SMR[i] = 0;
    AT91C_AIC_SVR[i] = (U32) nxos__unhandled_exception;
  }
  AT91C_AIC_SVR[AT91C_ID_FIQ] = (U32) nxos__unhandled_exception;
  *AT91C_AIC_SPU = (U32) nxos__unhandled_exception;

  UnmaskAll();
}

void AICDriver::InstallHandler(U32 irq_id, enum aic_priority priority,
                               enum aic_trigger_mode trigger_mode,
                               closure_t handler) {
  Disable(irq_id);
  Clear(irq_id);

  AT91C_AIC_SMR[irq_id] = (trigger_mode << 5) | priority;
  AT91C_AIC_SVR[irq_id] = reinterpret_cast<U32>(handler);

  Enable(irq_id);
}

}  // namespace nxos
