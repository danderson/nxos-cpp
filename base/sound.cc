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
#include "base/time.h"
#include "base/types.h"
#include "base/sound.h"

namespace {

// A digitized sine wave, used for tone generation.
const U32 kSineWaveBytes[16] = {
  0xF0F0F0F0,0xF0F0F0F0,
  0xFCFCFCFC,0xFCFCFCFC,
  0xFFFFFFFF,0xFFFFFFFF,
  0xFCFCFCFC,0xFCFCFCFC,
  0xF0F0F0F0,0xF0F0F0F0,
  0xC0C0C0C0,0xC0C08080,
  0x00000000,0x00000000,
  0x8080C0C0,0xC0C0C0C0
};

}

namespace nxos {

void Sound::Initialize() {
  g_power.EnablePeripheral(AT91C_ID_SSC);
  *AT91C_SSC_IDR = ~0;
  *AT91C_SSC_CR = AT91C_SSC_SWRST;

  *AT91C_SSC_TCMR = (AT91C_SSC_CKS_DIV |
                     AT91C_SSC_CKO_CONTINOUS |
                     AT91C_SSC_START_CONTINOUS);

  /* Configure the framing mode for transmission: 32-bit data words, 8
   * words per frame, most significant bit first. Also set the default
   * driven value (when there is no data being streamed) to 1.
   */
  *AT91C_SSC_TFMR =
    31 | AT91C_SSC_DATDEF | AT91C_SSC_MSBF | (7 << 8);

  /* Idle the output data pin of the SSC. The value on the pin will
   * now be whatever the SSC pumps out.
   */
  *AT91C_PIOA_PDR = AT91C_PA17_TD;

  /* Start transmitting. */
  *AT91C_SSC_CR = AT91C_SSC_TXEN;

  /* Install the interrupt routine that will feed data to the DMA
   * controller when we are outputting data.
   */
  g_aic.InstallHandler(AT91C_ID_SSC, AIC::PRIO_DRIVER,
                       AIC::TRIG_EDGE, ISR);
}

void Sound::PlayTone(U32 frequency, U32 ms) {
  // Any sound under 100ms will cause a failure to compute the
  // required output frequency. Don't make any sound if that happens.
  if (ms < 100)
    ms = 100;

  *AT91C_SSC_CMR = ((96109714 / 1024) / frequency) + 1;
  tone_cycles_ = (frequency * ms) / 2000;

  /* Enable handling of the transmit end interrupt. */
  *AT91C_SSC_IER = AT91C_SSC_ENDTX;

  /* Tell the DMA controller to start transmitting. This will cause an
   * interrupt, and the interrupt handler will point the DMA
   * controller at the data.
   */
  *AT91C_SSC_PTCR = AT91C_PDC_TXTEN;
}

void Sound::ISR() {
  if (--g_sound.tone_cycles_) {
    /* Tell the DMA controller to stream the static sine wave, 16
     * words of data.
     */
    *AT91C_SSC_TNPR = (U32)kSineWaveBytes;
    *AT91C_SSC_TNCR = 16;
  } else {
    /* Transmit complete, disable sound again. */
    *AT91C_SSC_IDR = AT91C_SSC_ENDTX;
  }
}

Sound g_sound;

}  // namespace nxos
