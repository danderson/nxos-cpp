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
#include "base/scheduler.h"
#include "base/types.h"
#include "base/time.h"

namespace {
const U32 kTimeIrqLine = AT91C_ID_SYS;
const U32 kClockFrequency = 48000000;  // Hz
const U32 kTimerBaseFrequency = kClockFrequency / 16;
const U32 kTimerIrqFrequency = 1000;  // Hz
}  // namespace

namespace nxos {

void Time::Initialize() {
  AIC::InstallHandler(kTimeIrqLine, AIC::PRIO_TICK,
                      AIC::TRIG_EDGE, ISR);

  *AT91C_PITC_PIMR = (((kTimerBaseFrequency / kTimerIrqFrequency) - 1) |
                      AT91C_PITC_PITEN | AT91C_PITC_PITIEN);
}

void Time::ISR() {
  const U32 status UNUSED = *AT91C_PITC_PIVR;
  ++g_time.time_;
  Scheduler::Call();
  AVR::FastUpdate();
}

Time g_time;

}  // namespace nxos
