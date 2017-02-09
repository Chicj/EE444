#include <msp430.h>
<<<<<<< HEAD
//#include "ChangeVcore.h" 
=======
#include "ChangeVcore.h" 
>>>>>>> 97b992404a296c80bccee1f9b1219263972fa97d


unsigned int time; // button push time for part 2

void main(void)
{  
<<<<<<< HEAD
/*
=======

>>>>>>> 97b992404a296c80bccee1f9b1219263972fa97d
// PART one of lab
  // set MCLK to 25 MHz (default SELREF for FLLREFCLK is XT1CLK = 32*1024 = 32768 Hz = 32.768 KHz)
  UCSCTL2 = 762; // Setting the freq multiplication factor * 1024
  UCSCTL1 |= DCORSEL_5;     // This sets the DCO frequency pg26 data sheet
  UCSCTL4 |= SELM__DCOCLKDIV;  // set output of FLLREFCLK --> DCOCLKDIV to input of MCLK  <-- This is default 
  
  // setup breakout board MCLK pin for scope testing
  P11DIR  |= BIT1;  // Set Port 11 pin 1 direction as an output
  P11SEL  |= BIT1; // Set port 11 pin 1 as MCLK signal 

  // min core voltage must be 2.4 to 3.6 V <-- f3 basically, (pg16 datasheet)
  IncrementVcore(); // up the voltage of the MSP430 in order to facilitate higher MCLK output. default val PMMCOREV_0
  IncrementVcore(); // up to PMMCOREV_2
  IncrementVcore(); // bring up to PMMCOREV_3 note:the IncrementVcore(); function will not set past PMMCOREV_3

  DecrementVcore(); // for plots 
  DecrementVcore();
  DecrementVcore();
  DecrementVcore(); //note: powering down Vcore to PMMCOREV = 01 does not kill the clock .... this is troublsome 
<<<<<<< HEAD
*/
// Part two of lab

  //setting up pins for SW1 --> P2.6
 // P2IE  |= BIT6; //Enable IR on P2.6
   P1IE  |= BIT6; //Enable IR on P2.6


=======

// Part two of lab

  //setting up pins for SW1 --> P2.6
  P2IE  |= BIT6; //Enable IR on P2.6

>>>>>>> 97b992404a296c80bccee1f9b1219263972fa97d
  // Divide ACLK
  UCSCTL5 |=  DIVA__32; // slow down the clock to allow for lots of time betwn buttn pushes ACLK = 32.768 KHz /32

  // Set up TA0
  TA0CTL |= TASSEL__ACLK + ID__8 + MC__UPDOWN; // sets Timer_A to use ACLK default ACLK = 32.768 KHz \ further divide the ACLK/8 \ Set Mode Control to updown

  TA0CCTL0 |= CM1;  // sets Timer_A capture mode to trigger on rising edge note: CCIS is default CCI0A
  TA0CCTL0 |= CCIS_2; // Set the IR enable for TA0 \ set capture compare to GND

            
 _EINT();
  LPM0;
<<<<<<< HEAD
}
/*
void Button_IR(void) __interrupt[PORT2_VECTOR]{
        switch(P2IV)
        {
          case P2IV_P2IFG6: // check button flag
            TA0CCTL0 ^= CCIS0;
            if(TA0CCTL0 != 0x0342){ // dont set if TA0CCTL0  is defalt val
              time=TA0CCTL0; // save time val
            }
          break;       
        }
        */
void Button_IR(void) __interrupt[PORT1_VECTOR]{
  switch(P1IV){
      case P1IV_NONE: // handle the interrupts on port 1 
      break;
      case P1IV_P1IFG0:
        P7OUT ^= BIT0;
      break;
      case P1IV_P1IFG1:
        P7OUT ^= BIT1;
      break;
      case P1IV_P1IFG2:
        P7OUT ^= BIT2;
      break;
      case P1IV_P1IFG3:
        P7OUT ^= BIT3;
      break;
      case P1IV_P1IFG4:
        P7OUT ^= BIT4;
      break;
      case P1IV_P1IFG5:
        P7OUT ^= BIT5;
      break;
      case P1IV_P1IFG6:
       TA0CCTL0 ^= CCIS0;
              if(TA0CCTL0 != 0x0342){ // dont set if TA0CCTL0  is defalt val
                time=TA0CCTL0; // save time val
                }
        P7OUT ^&= BIT6;
      break;
      case P1IV_P1IFG7:
        P7OUT ^= BIT7;
      break;
    }
=======
>>>>>>> 97b992404a296c80bccee1f9b1219263972fa97d
}

void Button_IR(void) __interrupt[PORT2_VECTOR]{
        switch(P2IV)
        {
          case P2IV_P2IFG6: // check button flag
            TA0CCTL0 ^= CCIS0;
            if(TA0CCTL0 != 0x0342){ // dont set if TA0CCTL0  is defalt val
              time=TA0CCTL0; // save time val
            }
          break;       
        }
}


