#ifndef DEBUG_H
#define DEBUG_H

inline void ChangeTone() {
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

  *AT91C_PMC_PCER = AT91C_ID_SSC | AT91C_ID_PIOA;
  *AT91C_SSC_IDR = ~0;
  *AT91C_SSC_CR = AT91C_SSC_SWRST;

  *AT91C_SSC_TCMR = (AT91C_SSC_CKS_DIV |
                     AT91C_SSC_CKO_CONTINOUS |
                     AT91C_SSC_START_CONTINOUS);
  *AT91C_SSC_TFMR =
      31 | AT91C_SSC_DATDEF | AT91C_SSC_MSBF | (7 << 8);

  *AT91C_PIOA_PDR = AT91C_PA17_TD;

  *AT91C_SSC_CR = AT91C_SSC_TXEN;

  *AT91C_SSC_CMR = ((96109714 / 1024) / 1000) + 1;

  int i = 0;
  while (true) {
    while (!(*AT91C_SSC_SR & AT91C_SSC_TXRDY)) {}
    *AT91C_SSC_THR = kSineWaveBytes[i];
    i++;
    if (i == 16)
      i = 0;
  }
}

#endif
