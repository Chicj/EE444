#include <msp430.h>

void main(void)
{
    int i;

  // Set MCLK to 4 MHz
  // MCLK = 32x1024
  // FLLN = 4,000,000 / MCLK = 122.0703125 ~ 122

 UCSCTL4 |= SELM__DCOCLK; // 
  
  P5DIR |= BIT0;

  while(1) {
    P5OUT ^= BIT0;
    for (i=0; i<40; i++);
  }
}

