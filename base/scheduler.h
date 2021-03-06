/** @file scheduler.h
 *  @brief Scheduler hooking.
 */

/* Copyright (C) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_BASE_SCHEDULER_H__
#define __NXOS_BASE_SCHEDULER_H__

#include "base/at91sam7s256.h"

#include "base/aic.h"
#include "base/cpp_decls.h"
#include "base/types.h"

namespace nxos {

class Scheduler {
 public:
  void Install(closure_ref_t scheduler);
  void Uninstall();
  inline void Call();

 private:
  static const U32 kSchedulerIrqLine = AT91C_ID_PWMC;
  closure_t scheduler_;

  static void ISR();
};

void Scheduler::Call() {
  if (scheduler_)
    g_aic.Trigger(kSchedulerIrqLine);
}

extern Scheduler g_scheduler;

}  // namespace nxos

#endif  // __NXOS_BASE_SCHEDULER_H__
