/** @file avr.h
 *  @brief AVR driver.
 */

/* Copyright (C) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#ifndef __NXOS_BASE_AVR_H__
#define __NXOS_BASE_AVR_H__

namespace nxos {

const U32 kNumSensors = 4;
const U32 kNumMotors = 3;

class AVR {
 public:

  enum button {
    NONE,
    OK,
    CANCEL,
    LEFT,
    RIGHT
  };

  static U16 sensor_value(U8 sensor) { return sensor_value_[sensor]; }
  static enum button button() { return button_; }
  static U32 battery_voltage() { return battery_voltage_; }
  static bool battery_is_pack() { return battery_is_pack_; }
  static U8 version_major() { return version_major_; }
  static U8 version_minor() { return version_minor_; }

  static U8 PowerOff();
  static U8 ResetMode();

 private:
  // Data coming from the AVR
  static volatile U16 sensor_value_[kNumSensors];
  static volatile enum button button_;
  static volatile U32 battery_voltage_;
  static volatile bool battery_is_pack_;
  static volatile U8 version_major_;
  static volatile U8 version_minor_;

  // Data going to the AVR
  static volatile bool power_off_;
  static volatile bool reset_;
  static volatile S8 motor_speed_[kNumMotors];
  static volatile U8 motor_brake_;  // One bit per motor.

  enum state {
    UNINITIALIZED,      /* Not initialized yet. */
    LINK_DOWN,          /* No handshake has been sent. */
    INIT,               /* Handshake send in progress. */
    WAIT_2MS,           /* Timed wait after the handshake. */
    WAIT_1MS,           /* More timed wait. */
    SEND,               /* Sending of to_avr in progress. */
    RECV,               /* Reception of from_avr in progress. */
  };

  static enum state state_;

  static const U32 kReadPacketSize = 13;
  static const U32 kWritePacketSize = 9;
  // This buffer is used for both sending and receiving.
  static U8 buffer_[kReadPacketSize];

  static void Initialize();
  static void FastUpdate();

  static void SerializeToBuffer();
  static void UpdateFromBuffer();

  friend class Core;
  friend class Time;
};

}  // namespace nxos

#endif  // __NXOS_BASE_AVR_H__
