#include <msp430.h>
#include <stdio.h>

extern int IncrementVcore(void);
extern int DecrementVcore(void);

unsigned int ADC12_15V_30; //30 C temp reff look @ pg 92 datasheet 
unsigned int ADC12_15V_85; //85 C temp reff
 
volatile float finaltempconverted;// var to hold final temp data

// UART global constants
const unsigned char *UART_TX_Buffer;
unsigned char UART_TX_Index;
unsigned char UART_TX_Length;
unsigned char UART_TX_Pkt_Complete;

unsigned char UART_RX_Buffer[25];
unsigned char UART_RX_Pkt_Complete;
unsigned char UART_RX_Index;

void main(void)
{
//  Temp sensor calibration data (TVL)
ADC12_15V_30 = *(unsigned int *)0x1A1A;
ADC12_15V_85 = *(unsigned int *)0x1A1C;


//***************************************************************Clock set up*****************************
// set PM core to acceptable value for 17 MHz pg16 data sheet
  DecrementVcore();
  DecrementVcore();
  DecrementVcore();

  IncrementVcore();
  IncrementVcore();  // ensure operation in PMM lvl 2 in in order to run 17 MHz clk

// set SMCLK to 17 MHz (default SELREF for FLLREFCLK is XT1CLK = 32*1024 = 32768 Hz = 32.768 KHz)
  UCSCTL2 = 511;               // Setting the freq multiplication factor * 32768 for final clk freq @ 16744448 Hz
  UCSCTL1 |= DCORSEL_4;        // This sets the DCO frequency pg26 data sheet 12.3 -- 28.3 MHz
  UCSCTL4 |= SELS__DCOCLKDIV ; // set output of FLLREFCLK --> DCOCLKDIV to input of SMCLK  <-- This is default 

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
  P5SEL |= BIT0;  // select Peripheral module mode for UART 
  P5DIR |= BIT0;

  // P5.6 - UCA1TXD, P5.7 UCA1RXD
  P5SEL |= BIT6|BIT7;//UCA1STE|UCA1TXD|UCA1RXD

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
//************************************************************* UART setup (UCA1)
  //SET UP UART 
   UCA1CTL0   |= UCPEN;// --> a good idea but works w/out
   //selected parity enable |odd parity| LSB| 8 bit| one stop bit| UART| async mode

  UCA1CTL1   |= UCSSEL__SMCLK|UCSWRST;
  // use SMCLK| no  erroneous RX will IR| break chars do no IR| not in sleep mode (UCDORM)| next frame transmitted is data| next frame transmitted is not a break| software rest enabled 

  //Set UART bit rate from pg 955 fam guide "commonly used baud rate settings"  
  UCA1BR0 = 2;

  // Modulation control register pg 951 fam guide 
  //17MHZ/460800 = 36.89 = 32 + 4.89 = 2*16 + 4 + .89*8 = 7.12 = 7 
  UCA1MCTL |= UCBRF_4|UCBRS_7|UCOS16; //<-- why does this work ???? | enabled oversampling mode 

  UCA1CTL1   &= ~UCSWRST; // enable UART lock UART settings 

  UCA1IE |= UCRXIE; // Enable RX interrupts 
 
  _EINT();  // set global IR enable 
  while(1){};
  //LPM0;
}

// UCA1 UART interrupt service routine 
void UART_IR(void) __interrupt[USCI_A1_VECTOR]{
  
  switch(UCA1IV){
    case  USCI_NONE:
    break;
    case  USCI_UCRXIFG:
      P1DIR |= BIT0;  // latch LED on RX
      while(!(UCA0IFG & UCTXIFG)); {
      UCA1TXBUF = UCA1RXBUF;
      }
    break;
    case  USCI_UCTXIFG:
    break;
    case  USCI_I2C_UCSTTIFG:
    break;
    case  USCI_I2C_UCSTPIFG:
    break;
    case  USCI_I2C_UCRXIFG:
    break;
    case USCI_I2C_UCTXIFG:
    break;
  }
}

 //button IR code
  void Button_IR(void) __interrupt[PORT2_VECTOR]{
 static int i=0 ,k=0;
  char myString[100];

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

                // note Crossworks dose not print floating point numbers unless you set it to in solution properties 
                sprintf(myString, "Temp of the MSP is %2.2f degrees C",finaltempconverted); 


          break; 
        }
}

// ADC IR code
void ADC12_IR(void) __interrupt[ADC12_VECTOR]{
  int i= 0; 
  volatile float data30Cconverted ,data85Cconverted;
  volatile unsigned int  sample = 0;
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

