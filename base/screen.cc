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
#include "base/screen.h"

// Internal command bytes implementing part of the basic commandset of
// the UC1601.
#define SET_COLUMN_ADDR0(addr) (0x00 | (addr & 0xF))
#define SET_COLUMN_ADDR1(addr) (0x10 | ((addr >> 4) & 0xF))
#define SET_MULTIPLEX_RATE(rate) (0x20 | (rate & 3))
#define SET_SCROLL_LINE(sl) (0x40 | (sl & 0x3F))
#define SET_PAGE_ADDR(page) (0xB0 | (page & 0xF))
#define SET_BIAS_POT0() (0x81)
#define SET_BIAS_POT1(pot) (pot & 0xFF)
#define SET_RAM_ADDR_CONTROL(auto_wrap, page_first, neg_inc, write_only_inc) \
           (0x88 | (auto_wrap << 0) | (page_first << 1) |                    \
            (neg_inc << 2) | (write_only_inc << 3))
#define SET_ALL_PIXELS_ON(on) (0xA4 | (on & 1))
#define SET_INVERSE_DISPLAY(on) (0xA6 | (on & 1))
#define ENABLE(on) (0xAE | (on & 1))
#define SET_MAP_CONTROL(mx, my) (0xC0 | (mx << 1) | (my << 2))
#define RESET() (0xE2)
#define SET_BIAS_RATIO(bias) (0xE8 | (bias & 3))


namespace nxos {

void Screen::Initialize() {
  InitializeSPI();

  
}

void Screen::InitializeSPI() {
  g_power.EnablePeripheral(AT91C_ID_SPI);

  /* Configure the PIO controller: Hand the MOSI (Master Out, Slave
   * In) and SPI clock pins over to the SPI controller, but keep MISO
   * (Master In, Slave Out) and PA10 (Chip Select in this case) and
   * configure them for manually driven output.
   *
   * The initial configuration is command mode (sending LCD commands)
   * and the LCD controller chip not selected.
   */
  *AT91C_PIOA_PDR = AT91C_PA13_MOSI | AT91C_PA14_SPCK;
  *AT91C_PIOA_ASR = AT91C_PA13_MOSI | AT91C_PA14_SPCK;

  *AT91C_PIOA_PER = AT91C_PA12_MISO | AT91C_PA10_NPCS2;
  *AT91C_PIOA_OER = AT91C_PA12_MISO | AT91C_PA10_NPCS2;
  *AT91C_PIOA_CODR = AT91C_PA12_MISO;
  *AT91C_PIOA_SODR = AT91C_PA10_NPCS2;

  /* Disable all SPI interrupts, then configure the SPI controller in
   * master mode, with the chip select locked to chip 0 (UC1601 LCD
   * controller), communication at 2MHz, 8 bits per transfer and an
   * inactive-high clock signal.
   */
  *AT91C_SPI_CR = AT91C_SPI_SWRST;
  *AT91C_SPI_CR = AT91C_SPI_SPIEN;
  *AT91C_SPI_IDR = ~0;
  *AT91C_SPI_MR = (6 << 24) | AT91C_SPI_MSTR;
  AT91C_SPI_CSR[0] = ((0x18 << 24) | (0x18 << 16) | (0x18 << 8) |
                      AT91C_SPI_BITS_8 | AT91C_SPI_CPOL);

  /* Now that the SPI bus is initialized, pull the Chip Select line
   * low, to select the uc1601. For some reason, letting the SPI
   * controller do this fails. Therefore, we force it once now.
   */
  *AT91C_PIOA_CODR = AT91C_PA10_NPCS2;

  /* Install an interrupt handler for the SPI controller, and enable
   * DMA transfers for SPI data. All SPI-related interrupt sources are
   * inhibited, so it won't bother us until we're ready.
   */
  g_aic.InstallHandler(AT91C_ID_SPI, AIC::PRIO_DRIVER, AIC::TRIG_LEVEL, Screen::ISR)
  *AT91C_SPI_PTCR = AT91C_PDC_TXTEN;
}

void Screen::ISR() {
  
}

void Screen::SetTXMode(Mode mode) {
  if (mode_ == mode)
    return;

  while (!(*AT91C_SPI_SR & AT91C_SPI_TXEMPTY));

  mode_ = mode;
  if (mode == COMMAND) {
    *AT91C_PIOA_CODR = AT91C_PA12_MISO;
  } else {
    *AT91C_PIOA_SODR = AT91C_PA12_MISO;
  }
}

}  // namespace nxos
