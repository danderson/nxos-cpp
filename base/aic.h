/** @file aic.h
 *  @brief Advanced Interrupt Controller driver.
 */

/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_BASE_DRIVERS_AIC_H__
#define __NXOS_BASE_DRIVERS_AIC_H__

#include "base/at91sam7s256.h"

#include "base/asm_decls.h"
#include "base/driver_decls.h"
#include "base/types.h"

extern "C" {
// Defined in interrupts.S
extern void nxos__interrupts_mask(U8* mask_counter);
extern void nxos__interrupts_unmask(U8* mask_counter);
}

namespace nxos {

class AICDriver {
 public:
  enum aic_priority {
    PRIO_LOW = 1,         /**< User and soft real time tasks. */
    AIC_PRIO_DRIVER = 3,  /**< Most drivers go in here. */
    AIC_PRIO_SOFTMAC = 4, /**< Drivers that have no hardware controller. */
    AIC_PRIO_SCHED = 5,   /**< The scheduler.
                           *
                           * It doesn't displace maintaining the AVR
                           * link, but can preempt everything else.
                           */
    AIC_PRIO_RT = 6,      /**< Hard real time tasks.
                           *
                           * This is basically the TWI driver.
                           */
    AIC_PRIO_TICK = 7,    /* Hard real time tasks (system time, AVR link). */
  };

  enum aic_trigger_mode {
    AIC_TRIG_LEVEL = 0,   /**< Level-triggered interrupt. */
    AIC_TRIG_EDGE = 1,    /**< Edge-triggered interrupt. */
  };

  void Initialize();

  void InstallHandler(U32 irq_id, enum aic_priority priority,
                      enum aic_trigger_mode trigger_mode,
                      closure_t handler);

  void Enable(U32 irq_id) { *AT91C_AIC_IECR = (1 << irq_id); }
  void Disable(U32 irq_id) { *AT91C_AIC_IDCR = (1 << irq_id); }
  void Trigger(U32 irq_id) { *AT91C_AIC_ISCR = (1 << irq_id); }
  void Clear(U32 irq_id) { *AT91C_AIC_ICCR = (1 << irq_id); }

  void MaskAll() { nxos__interrupts_mask(&mask_nesting_level_); }
  void UnmaskAll() { nxos__interrupts_unmask(&mask_nesting_level_); }

 private:
  U8 mask_nesting_level_;

  // Constructors
  DRIVER(AICDriver)
      : mask_nesting_level_(1) { }
};

DRIVER_ACCESSOR(AICDriver, AIC);

}  // namespace nxos

#endif  /* __NXOS_BASE_DRIVERS_AIC_H__ */
