// Lab2Sensor.c

#include "Lab2Sensor.h"
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/PLL.h"
#include "../inc/CortexM.h"
#include "../inc/LaunchPad.h"

void ADC_Init(void){
  SYSCTL_RCGCADC_R |= 0x0001;   // 1) activate ADC0
  SYSCTL_RCGCGPIO_R |= 0x10;    // 2) activate clock for Port E
  while((SYSCTL_PRGPIO_R&0x10) != 0x10){};  // 3 for stabilization
  GPIO_PORTE_DIR_R &= ~0x20;    // 4) make PE5 input
  GPIO_PORTE_AFSEL_R |= 0x20;   // 5) enable alternate function on PE5
  GPIO_PORTE_DEN_R &= ~0x20;    // 6) disable digital I/O on PE5
  GPIO_PORTE_AMSEL_R |= 0x20;   // 7) enable analog functionality on PE5
  while((SYSCTL_PRADC_R&0x0001) != 0x0001){}; // good code, but not implemented in simulator
  ADC0_PC_R &= ~0xF;
  ADC0_PC_R |= 0x1;             // 8) configure for 125K samples/sec
  ADC0_SSPRI_R = 0x0123;        // 9) Sequencer 3 is highest priority
  ADC0_ACTSS_R &= ~0x0008;      // 10) disable sample sequencer 3
  ADC0_EMUX_R &= ~0xF000;       // 11) seq3 is software trigger
  ADC0_SSMUX3_R &= ~0x000F;
  ADC0_SSMUX3_R += 8;           // 12) set channel 8 (PE5's channel)
  ADC0_SSCTL3_R = 0x0006;       // 13) no TS0 D0, yes IE0 END0
  ADC0_IM_R &= ~0x0008;         // 14) disable SS3 interrupts
  ADC0_ACTSS_R |= 0x0008;       // 15) enable sample sequencer 3
}
// Aidan: Code modified from book
int32_t ADC_In(void){
  uint32_t result;
  ADC0_PSSI_R = 0x0008;            // 1) initiate SS3
  while((ADC0_RIS_R&0x08)==0){};   // 2) wait for conversion done
  result = ADC0_SSFIFO3_R&0xFFF;   // 3) read result
  ADC0_ISC_R = 0x0008;             // 4) acknowledge completion
  return result;
}

#define A 99159
#define B -507
#define IRmax 900
#define Dmax 400
int32_t IR_Convert(int32_t n){  // returns distance in mm
	int32_t dist = A/(n+B);
	return dist;
}

// Unit test main
void main1(){
	PLL_Init(Bus80MHz);     // 80 MHz clock
  LaunchPad_Init();       // activate port F
  ADC_Init();
 
	while(1){
    uint32_t d = IR_Convert(ADC_In());
		//Clock_Delay1ms(1000); // TODO: uncommenting this doesn't compile
  }
	
}