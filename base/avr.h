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

  U16 sensor_value(U8 sensor) { return sensor_value_[sensor]; }
  enum button button() { return button_; }
  U32 battery_voltage() { return battery_voltage_; }
  bool battery_is_pack() { return battery_is_pack_; }
  U8 version_major() { return version_major_; }
  U8 version_minor() { return version_minor_; }

  U8 PowerOff();
  U8 ResetMode();

 private:
  // Data coming from the AVR
  volatile U16 sensor_value_[kNumSensors];
  volatile enum button button_;
  volatile U32 battery_voltage_;
  volatile bool battery_is_pack_;
  volatile U8 version_major_;
  volatile U8 version_minor_;

  // Data going to the AVR
  volatile bool power_off_;
  volatile bool reset_;
  volatile S8 motor_speed_[kNumMotors];
  volatile U8 motor_brake_;  // One bit per motor.

  enum state {
    UNINITIALIZED,      /* Not initialized yet. */
    LINK_DOWN,          /* No handshake has been sent. */
    INIT,               /* Handshake send in progress. */
    WAIT_2MS,           /* Timed wait after the handshake. */
    WAIT_1MS,           /* More timed wait. */
    SEND,               /* Sending of to_avr in progress. */
    RECV,               /* Reception of from_avr in progress. */
  };

  enum state state_;

  static const U32 kReadPacketSize = 13;
  static const U32 kWritePacketSize = 9;
  // This buffer is used for both sending and receiving.
  U8 buffer_[kReadPacketSize];

  //TWI twi_;

  void Initialize();
  void FastUpdate();

  void SerializeToBuffer();
  void UpdateFromBuffer();

  friend class Core;
  friend class Time;
};

extern AVR g_avr;

}  // namespace nxos

#endif  // __NXOS_BASE_AVR_H__
