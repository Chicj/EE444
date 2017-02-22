#include <msp430.h>
#include "hal_LCD.h"

unsigned int ADC12_15V_30; //30 C temp reff look @ pg 92 datasheet 
unsigned int ADC12_15V_85; //85 C temp reff
 
volatile float finaltempconverted;// var to hold final temp data


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

//*************************************************************** pin set up ***************************
  // setup breakout board MCLK and SMCLK test pins for scope testing
  P11DIR  |= BIT2;  // Set Port 11 pin 2 direction as an output
  P11SEL |= BIT2;   // Set port 11 pin 2 as SMCLK signal

  //setting up pins for SW1 --> P2.6
  P2REN |= BIT6 + BIT7;  // turn on internal resistor to combat bounce (this pulls down)
  P2OUT |= BIT6 + BIT7;  // Pull up when P2REN is high for both switches ( this pulls up), pg 410 fam
  P2IE |= BIT6 + BIT7;   // Enable IR on P2.6 and P2.7

  P1OUT |= BIT0 + BIT1;  // setup test LED's
  P1DIR |= BIT0 + BIT1;  // Set LED High on start up   

  //P5.0/A8/VREF+/VeREF+
  P5SEL |= BIT0;  // select Peripheral module mode 
  P5DIR |= BIT0;

//*************************************************************** ADC setup *****************************
// when using the ADC to sample internal temp sensor sample time must be greater then 30 us
  REFCTL0 |= REFON + REFOUT ;  // Ref available externally,Enable reference in ADC,
// change REFVSEL as needed. default 1.5V

// ADC12 setup reg
// turn on ADC,set sample hold time to 1024, multiple sample conversion, allow for 30 us btwn samples
  ADC12CTL0 |= ADC12ON + ADC12OVIE + ADC12MSC + ADC12SHT0_10; 

  ADC12CTL1 |= ADC12SHP + ADC12SSEL_3 + ADC12CONSEQ_1; // use signal timer, use SMCLK, single conversion mode

  ADC12MCTL0 |= ADC12INCH_10 + ADC12SREF_1;  // read temp, set vref
  ADC12MCTL1 |= ADC12INCH_10 + ADC12SREF_1;  // read temp, set vref
  ADC12MCTL2 |= ADC12INCH_10 + ADC12SREF_1;  // read temp, set vref
  ADC12MCTL3 |= ADC12INCH_10 + ADC12SREF_1;  // read temp, set vref
  ADC12MCTL4 |= ADC12INCH_10 + ADC12SREF_1;  // read temp, set vref
  ADC12MCTL5 |= ADC12INCH_10 + ADC12SREF_1;  // read temp, set vref
  ADC12MCTL6 |= ADC12INCH_10 + ADC12SREF_1;  // read temp, set vref
  ADC12MCTL7 |= ADC12INCH_10 + ADC12SREF_1 + ADC12EOS;  // read temp, set vref , end of sequence

  ADC12IE |= ADC12IE7; // enable ADC12IE0 IR


  _EINT();  // set global IR enable 
  while(1){
  }
}

 //button IR code
void Button_IR(void) __interrupt[PORT2_VECTOR]{
int i=0 ,k=0;
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

             ADC12CTL0 ^= ADC12ENC + ADC12SC;  // enable conversion,start conversion
             __no_operation(); // for debug 
              
          break; 
          case P2IV_P2IFG7:// check button , SW2 on breakout board 
          
               P1DIR ^= BIT1;  // toggle LED when button push
                  
                  halLcdInit(); 
                  halLcdBackLightInit();
                  halLcdClearScreen();
                  halLcdSetContrast(90);// ContrastLevel = 70 - 127
                  halLcdSetBackLight(5); // BackLightLevel = 0 - 16                
                  if (k==0){                           
                     //for(i=0;i<5;i++)
                    char myString[100];
                    // sprintf(myString, "The temperature\n in the room is\n %d degrees C\n",finaltempconverted);
                    sprintf(myString, " The Temperature in the room is %f degrees ",finaltempconverted);
                    k=k+1;                 
                    halLcdClearScreen();
                    halLcdPrintXY(myString, 5, 5, GRAYSCALE_TEXT);                 
                  }
                  else if (k==1)
                  {
                    k=0;
                    halLcdClearScreen();
                    halLcdShutDownBackLight();
                   }
          break; 
        }
}

// ADC IR code
void ADC12_IR(void) __interrupt[ADC12_VECTOR]{
  int i= 0; 
  volatile float data30Cconverted ,data85Cconverted;
  volatile unsigned int  sample = 0, rawtemp[8];
        switch(ADC12IV)
        {
          case ADC12IV_NONE: // ADC12IFGx flags are not reset by an ADC12IV access
          break;
          case ADC12IV_ADC12TOVIFG: // handle ADC conversion look at how these flags clear
          break;
          case ADC12IV_ADC12IFG0: 
          break;
          case ADC12IV_ADC12IFG1: 
          break;
          case ADC12IV_ADC12IFG2: 
          break;
          case ADC12IV_ADC12IFG3: 
          break;
          case ADC12IV_ADC12IFG4: 
          break;
          case ADC12IV_ADC12IFG5: 
          break;
          case ADC12IV_ADC12IFG6: 
          break;
          case ADC12IV_ADC12IFG7:

            for(i;i<8;i++){ // sum all samples
              sample += *(&ADC12MEM0 + i);// ADC12MEMx is a dereferenced pointer  
              }
            sample = sample>>3; // divide all samples to get an average, note this truncates the remainder
            finaltempconverted = sample * 1.47/4096; // pg 82 fam 
            data30Cconverted = ADC12_15V_30 * 1.5/4096;
            data85Cconverted = ADC12_15V_85 * 1.5/4096; 
            finaltempconverted = finaltempconverted - data30Cconverted; 
            finaltempconverted = finaltempconverted / (data85Cconverted-data30Cconverted)*(85-30)+30;
          break;
          case ADC12IV_ADC12IFG8: 
          break;
          case ADC12IV_ADC12IFG9: 
          break;
          case ADC12IV_ADC12IFG10: 
          break;
          case ADC12IV_ADC12IFG11: 
          break;
          case ADC12IV_ADC12IFG12: 
          break;
          case ADC12IV_ADC12IFG13: 
          break;
         }
}
