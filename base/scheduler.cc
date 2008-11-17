/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#include "at91sam7s256.h"

#include "base/aic.h"
#include "base/types.h"
#include "base/scheduler.h"

namespace nxos {

void Scheduler::Install(closure_ref_t scheduler) {
  const closure_t old_scheduler = scheduler_;
  scheduler_ = &scheduler;
  if (!old_scheduler)
    g_aic.InstallHandler(kSchedulerIrqLine, AIC::PRIO_SCHED,
                         AIC::TRIG_EDGE, ISR);
}

void Scheduler::Uninstall() {
  if (scheduler_) {
    g_aic.UninstallHandler(kSchedulerIrqLine);
    scheduler_ = NULL;
  }
}

void Scheduler::ISR() {
  g_aic.Clear(kSchedulerIrqLine);
  g_scheduler.scheduler_();
}

Scheduler g_scheduler;

}  // namespace nxos
