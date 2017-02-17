#include <msp430.h>
#include "hal_lcd.h"

unsigned int ADC12_15V_30; //30 C temp reff look @ pg 92 datasheet 
unsigned in ADC12_15V_85; //85 C temp reff

void main(void)
{
//  Temp sensor calibration data (TVL)
ADC12_15V_30 = *(unsigned int *)0x1A1A;
ADC12_15V_85 = *(unsigned int *)0x1A1C;

//***************************************************************Clock set up*****************************
// set PM core to acceptable value for 16 MHz pg16 data sheet
DecrementVcore();
DecrementVcore();
DecrementVcore();

IncrementVcore();
IncrementVcore();  // ensure operation in PMM lvl 2 in in order to run 16 MHz clk

// set SMCLK to 16 MHz (default SELREF for FLLREFCLK is XT1CLK = 32*1024 = 32768 Hz = 32.768 KHz)
  UCSCTL2 = 488;                  // Setting the freq multiplication factor * 1024 for final clk freq @ 16 MHz
  UCSCTL1 |= DCORSEL_4;         // This sets the DCO frequency pg26 data sheet 12.3 -- 28.3 MHz
  UCSCTL4 |= SELS__DCOCLKDIV ;  // set output of FLLREFCLK --> DCOCLKDIV to input of SMCLK  <-- This is default 

//***************************************************************pin up************************************
// test clk output on P11.1
  // setup breakout board MCLK and SMCLK test pins for scope testing
  P11DIR  |= BIT2;  // Set Port 11 pin 2 direction as an output
  P11SEL |= BIT2;   // Set port 11 pin 2 as SMCLK signal

  //setting up pins for SW1 --> P2.6
  P2REN |= BIT6;  // turn on internal resistor to combat bounce (this pulls down)
  P2OUT |= BIT6;  // Pull up when P2REN is high
  P2IE |= BIT6;   // Enable IR on P2.6
  P2REN |= BIT7;  // turn on internal resistor to combat bounce (this pulls down)
  P2OUT |= BIT7;  // Pull up when P2REN is high
  P2IE |= BIT7;   // Enable IR on P2.7

  P1OUT |= BIT1;  // setup test LED's
  P1DIR |= BIT1;  // Set LED High on start up   
  P1OUT |= BIT0;  // setup test LED's
  P1DIR |= BIT0;  // Set LED High on start up 

  //P5.0/A8/VREF+/VeREF+
  P5SEL |= BIT0;  // select Peripheral module mode 
  P5DIR |= BIT0;

//***************************************************************ADC setup*****************************
// when using the ADC to sample internal temp sensor sample time must be greater then 30 us
  REFCTL0 |= REFON + REFOUT ;  // Ref available externally,Enable reference in ADC,
// change REFVSEL as needed. default 1.5V

  ADC12CTL0 |= ADC12ON + 0xF00 + ADC12REFON;  // turn on ADC,set sample hold time to 1024,REF on
    
  ADC12CTL1 |= ADC12SREF1; // set ref bounds Vr+ and Vr- 

  ADC12IE |= ADC12IE0; // enable mem1

  ADC12MCTL1 |= ADC12INCH_10;  // set temp as ADC input

  ADC12CTL0 |= ADC12ENC;  // enable conversion 

  // Lock ADC after we are done writing things 
  _EINT();  // set global IR enable 
  while(1);
}

//button IR code
void Button_IR(void) __interrupt[PORT2_VECTOR]{
int sample;
        switch(P2IV)
        {
          case P2IV_NONE: 
          break; 
          case P2IV_P2IFG0:
          break; 
          case P2IV_P2IFG1: 
          break; 
          case P2IV_P2IFG2: 
          break; 
          case P2IV_P2IFG3: 
          break; 
          case P2IV_P2IFG4:
          break; 
          case P2IV_P2IFG5: 
          break; 
          case P2IV_P2IFG6: // check button flag, SW1 on breakout board
            P1DIR ^= BIT0;  // toggle LED when button push
              while(1);
              { 
                ADC12CTL0 |= ADC12SC; //  start conversion
                sample = ADC12MEM1; // save data
                 __bis_SR_register(LPM0_bits);  // GIE is redundant here
                __no_operation(); // for debug 
              }
          break; 
          case P2IV_P2IFG7:// check button , SW2 on breakout board
               P1DIR ^= BIT1;  // toggle LED when button push
          break; 
        }
}

// ADC IR code
void ADC12_IR(void) __interrupt[ADC12_VECTOR]{
        switch(ADC12IV)
        {
          case ADC12IFG0: 
          break;
          case ADC12IFG1: // handle ADC conversion look at how these flags clear
            //sample[0] = *ADC12MEM1;
          break;
          case ADC12IFG2: 

          break;
          case ADC12IFG3: 

          break;
          case ADC12IFG4: 

          break;
          case ADC12IFG5: 

          break;
          case ADC12IFG6: 

          break;
          case ADC12IFG7: // check button flag

          break;
          case ADC12IFG8: // check button flag

          break;
          case ADC12IFG9: // check button flag

          break;
          case ADC12IFG10: // check button flag

          break;
          case ADC12IFG11: // check button flag

          break;
          case ADC12IFG12: // check button flag

          break;
          case ADC12IFG13: // check button flag

          break;
          case ADC12IFG14: // check button flag

          break;
          case ADC12IFG15: // check button flag

          break;
         }
}
