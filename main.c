#include <msp430.h>
#include <stdio.h>
#include <string.h>

extern int IncrementVcore(void);
extern int DecrementVcore(void);

unsigned int ADC12_15V_30; //30 C temp reff look @ pg 92 datasheet 
unsigned int ADC12_15V_85; //85 C temp reff
unsigned int globali=1;   //global counter
 
volatile float finaltempconverted;// var to hold final temp data

char mystring[50];  // save ADC data
char rxbuff[42];     // save UART input data [make this arbitrarily large]
char taketempstr[] = "Taking temp data.\r\n";  // print string on temp cmnd
char stopstr[] = "Stopped taking temp data.\r\n";

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
  UCSCTL4 |= SELS__DCOCLKDIV + SELA_0; // set output of FLLREFCLK --> DCOCLKDIV to input of SMCLK | setup ACLK 

//*************************************************************** pin set up ***************************
  // setup breakout board MCLK and SMCLK test pins for scope testing
  P11DIR  |= BIT2 + BIT3;  // Set Port 11 pin 2 direction as an output
  P11SEL |= BIT2 + BIT3;   // Set port 11 pin 2 as SMCLK signal pin 3 as ACLK

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
  UCA1MCTL  |= UCBRF_4|UCBRS_7|UCOS16; //set modulation | enabled oversampling mode <-- note UCBRF 5 dosent work when using DCORSEL_4
  UCA1CTL1  &= ~UCSWRST; // enable UART lock UART settings 
  UCA1IE |= UCRXIE; // Enable RX interrupts 
//************************************************************ setup timer for the ADC
  TA0CTL |= TASSEL_1|MC_0; // use ACLK | stop mode
  TA0CCTL1 |= CCIE; // enables interrupts on capture compare mode 
  TA0CCR1 = 16384;  //set ACLK capture point, ACLK = 32.84 kHz, used half that value to make a more accurate clock

  _EINT();  // set global IR enable 
  while(1){};
  //LPM0;
}

// UCA1 UART interrupt service routine 
void UART_IR(void) __interrupt[USCI_A1_VECTOR]{

  switch(UCA1IV){
    case  USCI_UCRXIFG:
      P1DIR |= BIT0;  // latch LED on RX
        //save UART input
            rxbuff[globali-1]=UCA1RXBUF;    // save UART into buffer to check for "temp" or "stop"
            UCA1TXBUF = rxbuff[globali-1];  // loop input chars back to terminal 
            while(!(UCA1IFG & UCTXIFG));  // delay for UART TX

          if(rxbuff[globali-1] == '\r' ||rxbuff[globali-1] == '\n'){ // check input string for enter 
            UCA1TXBUF = '\n';             // start a new line
            while(!(UCA1IFG & UCTXIFG));  // delay for UART TX

            sprintf(mystring, "The number of chars entered is %i. \n\r",globali-1); // populate string for UART char count | sprintf ends with a null 
            
            globali = 0;        // reset global counter       
            while(mystring[globali] != '\0'){ //print # of chars input 
              UCA1TXBUF = mystring[globali]; // spit out # of entered chars
              while(!(UCA1IFG & UCTXIFG));  // delay for UART TX
              globali++;  // increment counter
            }
            globali=0;  // reset global counter 

            if(!strcmp(rxbuff,"TEMP\r") || !strcmp(rxbuff,"temp\r")){ // check for temp
            
            globali = 0;        // reset global counter       
            while(taketempstr[globali] != '\0'){ //print starting message for temp measure 
              UCA1TXBUF = taketempstr[globali]; 
              while(!(UCA1IFG & UCTXIFG));  // delay for UART TX
              globali++;  // increment counter
            }
            globali=0;  // reset global counter <-- this will count time until "stop"

              TA0CTL |= MC__CONTINOUS; // start timer --> ADC conversion  
            }
            else if(!strcmp(rxbuff,"STOP\r") || !strcmp(rxbuff,"stop\r")){  // check for stop
              globali = 0;        // reset global counter       
              while(stopstr[globali] != '\0'){ //print starting message for temp measure 
              UCA1TXBUF = taketempstr[globali]; 
              while(!(UCA1IFG & UCTXIFG));  // delay for UART TX
              globali++;  // increment counter
              }
              globali=0;  // reset global counter 
              TA0CTL &= ~MC__CONTINOUS; // stop timer --> ADC conversion 
            }
          }
        globali++;  // increment counter
   }
}

// ADC IR code
void ADC12_IR(void) __interrupt[ADC12_VECTOR]{
  int i= 0; 
  volatile float data30Cconverted ,data85Cconverted;
  volatile unsigned int  sample = 0;
        switch(ADC12IV)
        {
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

            // note Crossworks dose not print floating point numbers unless you set it to in solution properties 
            // populate string for UART Tx | sprintf ends with a null 
            sprintf(mystring, "Temp of the MSP is %2.2f degrees C. \n\rTime: %i min and %i sec\r\n",finaltempconverted,globali/60,globali%60); 

            i = 0;  // set counter to 0
            while(mystring[i] != '\0'){ // single quotes tells the loops to look at a char "\0" <-- is a string (*char) ! '\0' <-- char
              UCA1TXBUF = mystring[i]; // clock out ADC temp string 
              while(!(UCA1IFG & UCTXIFG));  // delay for UART TX
              i++; // increment counter
            }
          break;
         }
}

void TIMER_A0_ISR(void)__interrupt[TIMER0_A1_VECTOR]
{
  switch(TA0IV){
    case TA0IV_TA0CCR1:    // dont use TA0IV_TA0CCR0 
      //start making temperature measurments every 2x per sec.
      P1OUT ^=BIT0; // blink a led
      TA0CCR1 += 16384; // for a one second timer
      globali++;  // increment for time info 
      ADC12CTL0 ^= ADC12ENC|ADC12SC;//enables ADC conversion and turns off shi
    break;
    default:
    break;
   }
}

