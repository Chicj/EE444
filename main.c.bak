#include <msp430.h>
#include <stdio.h>

extern int IncrementVcore(void);
extern int DecrementVcore(void);

enum  {AM=0,PM0,PM1,PM2,PM3}; // list power modes 
unsigned int state; // keep track of power mode 

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
      else{
        LPM3;   /* Enter Low Power Mode 3 */ // we seem not to go into LPM3 ?? 
      }
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
      else{
        LPM3_EXIT;  /* Exit Low Power Mode 3 */
        state = 0; // return state to AM 
      }
      

    break;

    case P2IV_P2IFG7: // sw2 flag 

    break;

    default:  // catch all 
    break;
    }
}
