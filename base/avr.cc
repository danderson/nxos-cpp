/* Copyright (c) 2008 the NxOS developers
 *
 * See AUTHORS for a full list of the developers.
 *
 * Redistribution of this file is permitted under
 * the terms of the GNU Public License (GPL) version 2.
 */

#include "at91sam7s256.h"

#include "base/aic.h"
#include "base/power.h"
#include "base/types.h"
#include "base/util.h"
#include "base/avr.h"

namespace nxos {
namespace {

const U32 kAvrI2CAddress = 1;
const U32 kAvrI2CWriteMode = ((kAvrI2CAddress << 16) |
                              AT91C_TWI_IADRSZ_NO | AT91C_TWI_MREAD);

// Sleep approximately the given number of nanoseconds. The timing of
// this function relies on CPU instruction timing, and may not be
// entirely accurate.
void NanoSleep(U32 ns) {
  volatile U32 x = (ns >> 7) + 1;

  while (x--) {}
}

class TWI {
 public:
  enum state {
    READY = 0,
    TX_BUSY,
    RX_BUSY
  };

  static void Initialize();
  static void Read(U8* buffer, U32 len);
  static void Write(const U8& buffer, U32 len);

  static bool IsReady() { return state_ == READY; };

 private:
  static volatile enum state state_;
  static volatile U8* buffer_;
  static volatile U32 buffer_len_;

  static void ISR();

  DISALLOW_CONSTRUCTION(TWI);
};

volatile enum TWI::state TWI::state_ = READY;
volatile U8* TWI::buffer_ = NULL;
volatile U32 TWI::buffer_len_ = 0;

void TWI::Initialize() {
  g_power.EnablePeripheral(AT91C_ID_TWI);
  g_power.EnablePeripheral(AT91C_ID_PIOA);
  *AT91C_TWI_IDR = ~0;

  /* If the system is rebooting, the coprocessor might believe that it
   * is in the middle of an I2C transaction. Furthermore, it may be
   * pulling the data line low, in the case of a read transaction.
   *
   * The TWI hardware has a bug that will lock it up if it is
   * initialized when one of the clock or data lines is low.
   *
   * So, before initializing the TWI, we manually take control of the
   * pins using the PIO controller, and manually drive a few clock
   * cycles, until the data line goes high.
   */
  *AT91C_PIOA_MDER = AT91C_PA3_TWD | AT91C_PA4_TWCK;
  *AT91C_PIOA_PER = AT91C_PA3_TWD | AT91C_PA4_TWCK;
  *AT91C_PIOA_ODR = AT91C_PA3_TWD;
  *AT91C_PIOA_OER = AT91C_PA4_TWCK;

  while (!(*AT91C_PIOA_PDSR & AT91C_PA3_TWD)) {
    *AT91C_PIOA_CODR = AT91C_PA4_TWCK;
    NanoSleep(1500);
    *AT91C_PIOA_SODR = AT91C_PA4_TWCK;
    NanoSleep(1500);
  }

  /* Now that the I2C lines are clean, hand them back to the TWI
   * controller.
   */
  *AT91C_PIOA_PDR = AT91C_PA3_TWD | AT91C_PA4_TWCK;
  *AT91C_PIOA_ASR = AT91C_PA3_TWD | AT91C_PA4_TWCK;

  /* Reset the controller and configure its clock. The clock setting
   * makes the TWI run at 380kHz.
   */
  *AT91C_TWI_CR = AT91C_TWI_SWRST | AT91C_TWI_MSDIS;
  *AT91C_TWI_CWGR = 0x020f0f;
  *AT91C_TWI_CR = AT91C_TWI_MSEN;

  /* Install the TWI interrupt handler. */
  g_aic.InstallHandler(AT91C_ID_TWI, AIC::PRIO_RT, AIC::TRIG_LEVEL, ISR);
}

void TWI::Read(U8* buffer, U32 len) {
  // TODO(dave): assert(state_ == READY)
  // TODO(dave): assert(buffer != NULL)
  // TODO(dave): assert(len > 0)

  state_ = RX_BUSY;
  buffer_ = buffer;
  buffer_len_ = len;

  *AT91C_TWI_MMR = ((kAvrI2CAddress << 16) |
                    AT91C_TWI_IADRSZ_NO |
                    AT91C_TWI_MREAD);
  *AT91C_TWI_CR = AT91C_TWI_START | AT91C_TWI_MSEN;
  *AT91C_TWI_IER = AT91C_TWI_RXRDY;
}

void TWI::Write(const U8& buffer, U32 len) {
  // TODO(dave): assert(state_ == READY)
  // TODO(dave): assert(len > 0)

  state_ = TX_BUSY;
  buffer_ = const_cast<U8*>(&buffer);
  buffer_len_ = len;

  *AT91C_TWI_MMR = (kAvrI2CAddress << 16) | AT91C_TWI_IADRSZ_NO;
  *AT91C_TWI_CR = AT91C_TWI_START | AT91C_TWI_MSEN;
  *AT91C_TWI_IER = AT91C_TWI_TXRDY;
}

void TWI::ISR() {
  /* Read the status register once to acknowledge all TWI interrupts. */
  U32 status = *AT91C_TWI_SR;

  /* Read mode and the status indicates a byte was received. */
  if (state_ == RX_BUSY && (status & AT91C_TWI_RXRDY)) {
    *buffer_++ = *AT91C_TWI_RHR;
    buffer_len_--;

    /* If only one byte is left to read, instruct the TWI to send a STOP
     * condition after the next byte.
     */
    if (buffer_len_ == 1) {
      *AT91C_TWI_CR = AT91C_TWI_STOP;
    }

    /* If the read is over, inhibit all TWI interrupts and return to the
     * ready state.
     */
    if (buffer_len_ == 0) {
      *AT91C_TWI_IDR = ~0;
      state_ = READY;
    }
  }

  /* Write mode and the status indicated a byte was transmitted. */
  if (state_ == TX_BUSY && (status & AT91C_TWI_TXRDY)) {
    /* If that was the last byte, inhibit TWI interrupts and return to
     * the ready state.
     */
    if (buffer_len_ == 0) {
      *AT91C_TWI_IDR = ~0;
      state_ = READY;
    } else {
      /* Instruct the TWI to send a STOP condition at the end of the
       * next byte if this is the last byte.
       */
      if (buffer_len_ == 1)
        *AT91C_TWI_CR = AT91C_TWI_STOP;

      /* Write the next byte to the transmit register. */
      *AT91C_TWI_THR = *buffer_++;
      buffer_len_--;
    }
  }
}

const char kAvrHandshake[] =
    "\xCC" "Let's samba nxt arm in arm, (c)LEGO Systems A/S";

inline U16 unmarshal_u16(const U8& buf) {
  return *reinterpret_cast<const U16*>(&buf);
}
}  // namespace

// Data coming from the AVR
volatile U16 AVR::sensor_value_[kNumSensors] = { 0, 0, 0, 0 };
volatile enum AVR::button AVR::button_ = AVR::NONE;
volatile U32 AVR::battery_voltage_ = 0;
volatile bool AVR::battery_is_pack_ = false;
volatile U8 AVR::version_major_ = 0;
volatile U8 AVR::version_minor_ = 0;

// Data going to the AVR
volatile bool AVR::power_off_ = false;
volatile bool AVR::reset_ = false;
volatile S8 AVR::motor_speed_[kNumMotors] = { 0, 0, 0 };
volatile U8 AVR::motor_brake_ = 0;  // One bit per motor.

enum AVR::state AVR::state_ = AVR::UNINITIALIZED;

U8 AVR::buffer_[kReadPacketSize] = { 0 };

void AVR::Initialize() {
  TWI::Initialize();
  state_ = LINK_DOWN;
}

void AVR::FastUpdate() {
  switch (state_) {
  case UNINITIALIZED:
    /* Because the system timer can call this update routine before
     * the driver is initialized, we have this safe state. It does
     * nothing and immediately returns.
     *
     * When the AVR driver initialization code runs, it will set the
     * state to AVR_LINK_DOWN, which will kickstart the state machine.
     */
    return;

  case LINK_DOWN:
    /* ARM-AVR link is not initialized. We need to send the hello
     * string to tell the AVR that we are alive. This will (among
     * other things) stop the "clicking brick" sound, and avoid having
     * the brick powered down after a few minutes by an AVR that
     * doesn't see us coming up.
     */
    TWI::Write(*kAvrHandshake, sizeof(kAvrHandshake)-1);
    state_ = INIT;
    break;

  case INIT:
    /* Once the transmission of the handshake is complete, go into a 2
     * millisecond wait, which is accomplished by the use of two
     * intermediate state machine states.
     */
    if (TWI::IsReady())
      state_ = WAIT_2MS;
    break;

  case WAIT_2MS:
    /* Wait another millisecond... */
    state_ = WAIT_1MS;
    break;

  case WAIT_1MS:
    /* Now switch the state to send mode. On the next refresh cycle,
     * the communication will be in "production" mode, and will start
     * by reading data back from the AVR.
     */
    state_ = SEND;
    break;

  case SEND:
    /* If the transmission is complete, switch to receive mode and
     * read the status structure from the AVR.
     */
    if (TWI::IsReady()) {
      state_ = RECV;
      TWI::Read(buffer_, kReadPacketSize);
    }

  case RECV:
    /* If the transmission is complete, unpack the read data into the
     * from_avr struct, pack the data in the to_avr struct into a raw
     * buffer, and shovel that over the i2c bus to the AVR.
     */
    if (TWI::IsReady()) {
      UpdateFromBuffer();
      state_ = SEND;
      SerializeToBuffer();
      TWI::Write(*buffer_, kWritePacketSize);
    }
    break;
  }
}

void AVR::SerializeToBuffer() {
  memset(buffer_, 0, kWritePacketSize);

  // Marshal the power mode configuration.
  if (power_off_) {
    buffer_[0] = 0x5A;
  } else if (reset_) {
    buffer_[0] = 0x5A;
    buffer_[1] = 0xA5;
  } else {
    // This is normal operation mode. The second transmitted byte is
    // the PWM frequency.
    buffer_[1] = 8;
  }

  // Marshal the motor speed settings.
  buffer_[2] = motor_speed_[0];
  buffer_[3] = motor_speed_[1];
  buffer_[4] = motor_speed_[2];

  // buffer_[5] is the value for the 4th motor, which doesn't
  // exist. This is probably a bug in the AVR firmware, but it is
  // required. So we just leave a byte of padding.
  //

  // Marshal the motor brake settings.
  buffer_[6] = motor_brake_;

  // Calculate the checksum.
  U8 checksum = 0;
  for (U32 i = 0; i < kWritePacketSize; ++i)
    checksum += buffer_[i];
  buffer_[8] = ~checksum;
}

void AVR::UpdateFromBuffer() {
  int checksum = 0;
  for (U32 i = 0; i < kReadPacketSize; ++i)
    checksum += buffer_[i];
  if (checksum != 0xFF)
    // Bad packet. TODO(dave): Abort here.
    return;

  // Unmarshal the sensor readings.
  sensor_value_[0] = unmarshal_u16(buffer_[0]);
  sensor_value_[1] = unmarshal_u16(buffer_[2]);
  sensor_value_[2] = unmarshal_u16(buffer_[4]);
  sensor_value_[3] = unmarshal_u16(buffer_[6]);

  // Unmarshal the button state.
  const U16 button = unmarshal_u16(buffer_[8]);
  if (button > 1023)
    button_ = OK;
  else if (button > 720)
    button_ = CANCEL;
  else if (button > 270)
    button_ = RIGHT;
  else if (button > 60)
    button_ = LEFT;
  else
    button_ = NONE;

  // Unmarshal version and battery info
  const U16 version_and_battery = unmarshal_u16(buffer_[10]);
  version_major_ = (version_and_battery >> 13) & 0x3;
  version_minor_ = (version_and_battery >> 10) & 0x7;
  battery_is_pack_ = (version_and_battery & 0x8000) ? false : true;
  // The voltage value is given in units of 13.848mV. To avoid pulling
  // in slow floating point emulation, we approximate this by a fixed
  // point multiplication by 3545, followed by a division by 256.
  battery_voltage_ = version_and_battery & 0x3FF;
  battery_voltage_ = (battery_voltage_ * 3545) >> 9;
}

}  // namespace nxos
