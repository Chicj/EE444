#include <msp430.h>

int IncrementVcore(void); // include function prototype for IncrementVcore 

void main(void)
{  
  // min core voltage must be 2.4 to 3.6 V <-- f3 basically, (pg16 datasheet)
  IncrementVcore(); // up the voltage of the MSP430 in order to facilitate higher MCLK output. default val PMMCOREV_0
  IncrementVcore(); // up to PMMCOREV_2
  IncrementVcore(); // bring up to PMMCOREV_3 note:the IncrementVcore(); function will not set past PMMCOREV_3

  // set MCLK to 25 MHz (default SELREF for FLLREFCLK is XT1CLK = 32*1024 = 32768 Hz = 32.768 KHz)
  UCSCTL2 = 762; // Setting the freq multiplication factor * 1024
  UCSCTL1 |= DCORSEL_6;     // This sets the DCO frequency pg26 data sheet
  UCSCTL4 |= SELM__DCOCLKDIV + SELM__DCOCLKDIV;  // set output of FLLREFCLK --> DCOCLKDIV to input of MCLK and SMCLK  <-- This is default 
  
  // setup breakout board MCLK and SMCLK test pins for scope testing
  P11DIR  |= BIT1;  // Set Port 11 pin 1 direction as an output
  P11SEL  |= BIT1; // Set port 11 pin 1 as MCLK signal 

  P11DIR  |= BIT2;  // Set Port 11 pin 2 direction as an output
  P11SEL  |= BIT2; // Set port 11 pin 2 as SMCLK signal 

  // board PWM pin setups
  P8DIR |= BIT6;  // set up P8.6 as an OUT1 in TimerA1  25%
  P7DIR |= BIT3;  // set up P7.3 as an OUT2 TimerA1 75%

  P8SEL |= BIT6;  // set up P8.6 as an Output mode 
  P7SEL |= BIT3;  // set up P7.3 as an Output mode 

  // disable SMCLK requests
  UCSCTL8 |= ~SMCLKREQEN;

  // Set up TA0
  TA1CTL |= TASSEL__SMCLK + MC__UPDOWN; // sets Timer_A to use SMCLK default /Set Mode Control to up/down mode
  
  TA1CCTL1  |= OUTMOD_4; // set enable IR / set output mode to "Toggle"
  TA1CCTL2  |= OUTMOD_4; // set enable IR / set output mode to "Toggle"
  
  //  TAxCCRn = Content of capture/compare register n will set the count up to val
  TA1CCR0 = 125; // set count up val 
  TA1CCR1 = 94;   //P8.6  75%
  TA1CCR2 = 32;   //P7.3  25%

  // Diff loops for testing SMCLK system response   
  // while(1);  // SMCLK and MCLK both on   
  LPM0;        //  SMCLK still on MCLK off
  //LPM3;      // SMCLK and MCLK off
}

