#include <msp430.h>
#include <stdio.h>

extern int IncrementVcore(void);
extern int DecrementVcore(void);

unsigned int globalcounter = 0;

enum  {AM=0,PM0,PM1,PM2,PM3,PM4}; // list power modes 
unsigned int state; // keep track of power mode 
#define count 62500;

void main(void)
{
//*************************************************************** pin set up ***************************
  // setup breakout board MCLK and SMCLK test pins for scope testing
  //P11DIR  |= BIT2 + BIT3;  // Set Port 11 pin 2 direction as an output
  //P11SEL |= BIT2 + BIT3;   // Set port 11 pin 2 as SMCLK signal pin 3 as ACLK

  //setting up pins for SW1 --> P2.6
  P2REN |= BIT6 + BIT7;  // turn on internal resistor to combat bounce (this pulls down)
  P2OUT |= BIT6 + BIT7;  // Pull up when P2REN is high for both switches ( this pulls up), pg 410 fam
  P2IE |= BIT6 + BIT7;   // Enable IR on P2.6 and P2.7

  // turn on some LEDs on pwr up 
  P1DIR |= BIT1;
  P1OUT |= BIT2;

//************************************************************** Clock setup****************************
  UCSCTL1 = DCORSEL_2; // selects DCO btwn 2.5 and 54 MHz
  //UCSCTL2 = 30;       // for 1 MHZ clkc
  UCSCTL2 = 239;       // for 8 MHZ clkc
  UCSCTL4 = SELA_0|SELS_3|SELM_3;//SELA_0 SELECTS XT1|SELS_3 SELECTS DCO| SELM_3 CHOOSES THE DCOCLK
  UCSCTL8 |=SMCLKREQEN|MCLKREQEN|ACLKREQEN;//THINGS WILL BE OFF UNLESS SOMETHING SPECIFICALLY NEEDS THE CLOCK
  //SET UP TIMER 
  TA0CTL = TASSEL__SMCLK + ID__8;// use SMCLK at 1MHz in cont. mode 
  TA0CCTL1 = CCIE;              // enable interrupts from the timer
  TA0CCR1 = count;              // set capture compare reg


   state = AM;// set starting state
  _EINT();  // set global IR enable 
  while(1){ // start in AM 
      if (state == AM){
        __no_operation();
      }
      else if (state == PM0){
        LPM0;  /* Enter Low Power Mode 0 */
      }
      else if (state == PM1){
        LPM1;  /* Enter Low Power Mode 1 */
      }
      else if (state == PM2){
        LPM2;  /* Enter Low Power Mode 2 */
      }
      else if (state == PM3){
        LPM3;   /* Enter Low Power Mode 3 */
      }
      else if (state == PM4){
        LPM4;  /* Enter Low Power Mode 4 */
      }
   }
}

void TimerA (void) __interrupt [TIMER0_A1_VECTOR] {
  switch (TA0IV) {
    case TA0IV_TA0CCR1:
      P5OUT ^= BIT0;          // Toggle P5.0
      TA0CCR1 += count;  // increment after we reach count val
      break;

    default:
      break;
  }
}

void button_interrupt(void) __interrupt[PORT2_VECTOR]{

  switch(P2IV){
    case P2IV_P2IFG6: // sw1 flag
      if(state == AM){
        state++; // increment state to go into LMP0 
      }
      else if (state == PM0){
        LPM0_EXIT;  /* Exit Low Power Mode 0 */
        state++; // increment state to go into LMP1
      }
      else if (state == PM1){
        LPM1_EXIT;  /* Exit Low Power Mode 1 */
        state++; // increment state to go into LMP2
      }
      else if (state == PM2){
        LPM2_EXIT;  /* Exit Low Power Mode 2 */
        state++; // increment state to go into LMP3
      }
      else if (state == PM3){
        LPM3_EXIT;  /* Exit Low Power Mode 3 */
        state++; // return state to AM 
      }
      else{
        LPM4_EXIT;  /* Exit Low Power Mode 4 */
        state = 0; // return state to AM 
      }

    break;

    case P2IV_P2IFG7:                   // sw2
      if (TA0CTL & MC__CONTINUOUS) {      // Toggle timer on/off
        TA0CTL &= ~MC__CONTINUOUS;
        TA0CTL |= TACLR;
        TA0CCR1 = count;
      }
      else {
        TA0CTL |= MC__CONTINUOUS;
      }
    break;

    break;

    default:  // catch all 
    break;
    }
}
