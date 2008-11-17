/** @file time.h
 *  @brief Timekeeping and system housekeeping.
 */

/* Copyright (C) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_BASE_TIME_H__
#define __NXOS_BASE_TIME_H__

#include "base/at91sam7s256.h"

#include "base/aic.h"
#include "base/cpp_decls.h"
#include "base/types.h"

namespace nxos {

class Core;

class Time {
 public:
  U32 Get() { return time_; }
  inline void Sleep(U32 millis);

 private:
  volatile U32 time_;

  void Initialize();
  static void ISR();

  friend class Core;
};

void Time::Sleep(U32 millis) {
  const U32 final = time_ + millis;

  while (time_ < final) {}
}

extern Time g_time;

}  // namespace nxos

#endif  // __NXOS_BASE_SCHEDULER_H__
