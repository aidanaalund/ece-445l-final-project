// PulseWidthMeasure.c
// Runs on LM4F120/TM4C123
// Use Timer0A in 24-bit edge time mode to request interrupts on the falling
// edge of PB6 (T0CCP0). 
// Use Timer0B in 24-bit edge time mode rising edge of PB7(T0CCP1) is latched, and 
// the system measures pulse between pulses.
// Daniel Valvano
// April 2, 2018

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2018
   Example 6.5, Program 6.5

 Copyright 2018 by Jonathan W. Valvano, valvano@mail.utexas.edu
    You may use, edit, run or distribute this file
    as long as the above copyright notice remains
 THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 VALVANO SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL,
 OR CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/
 */


#include <stdint.h>
#include "PulseWidthMeasure.h"
#include "Motor.h"
#include "../inc/Timer0A.h"
#include "../inc/Timer2A.h" // USE THESE TWO WHEN YOU INTEGRATE
#include "../inc/Timer3A.h" // ................................
#include "../inc/tm4c123gh6pm.h"
#include "../inc/PLL.h"
#include "../inc/CortexM.h"
#include "../inc/LaunchPad.h"

uint32_t startTimeTubeOne = 0;
uint32_t startTimeTubeTwo = 0;
uint32_t startTimeTubeThree = 0;

volatile uint32_t distanceOne = 0;
volatile uint32_t distanceTwo = 0;
volatile uint32_t distanceThree = 0;

volatile uint32_t Count = 0;
volatile uint32_t Count2 = 0;

uint32_t firstClock = 0;

int tubeDoneOne = 0;                   // set each falling
int tubeDoneTwo = 0;
int tubeDoneThree = 0;

//void timeInMicro (void) {
//	Count += 1;
//}

void Timer2PWMMeasure_Init(void) {
    // Enable the clock for Timer 2
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R2;  
    while ((SYSCTL_PRTIMER_R & SYSCTL_PRTIMER_R2) == 0);  // Wait for Timer 2 to be ready

    // Disable Timer 2 before configuration
    TIMER2_CTL_R = 0;

    // Configure Timer 2 as a 32-bit timer
    TIMER2_CFG_R = TIMER_CFG_32_BIT_TIMER;

    // Set Timer 2 to periodic mode
    TIMER2_TAMR_R = TIMER_TAMR_TAMR_PERIOD;  // Periodic mode

    // Set the direction to count up (clear TACDIR for up-counting)
    TIMER2_TAMR_R &= ~TIMER_TAMR_TACDIR;  // Clear the TACDIR bit to make it count up

    // Set the maximum value for the timer (maximum 32-bit value)
    TIMER2_TAILR_R = 0xFFFFFFFF;  // Set to maximum 32-bit value (up to 4,294,967,295)

    // Enable Timer 2A
    TIMER2_CTL_R |= TIMER_CTL_TAEN;
}

void GPIOPortB_Handler(void){
	
	if (GPIO_PORTB_RIS_R & 0x80) {// pb7
		GPIO_PORTB_ICR_R = 0x80;
		if (tubeDoneOne == 0) {
			tubeDoneOne = 1;
			startTimeTubeOne = TIMER2_TAR_R;
			GPIO_PORTB_DATA_R |= 0x02;
		} else if (tubeDoneOne == 1) {
			GPIO_PORTB_DATA_R &= ~0x02;
			if (TIMER2_TAR_R < startTimeTubeOne) { 
				uint32_t test = TIMER2_TAR_R;
				distanceOne = startTimeTubeOne - TIMER2_TAR_R; /// 29; // Fixed point (units of 0.5cm) (moved to display.c)
			} else {
				uint32_t test = TIMER2_TAR_R;
				distanceOne = (startTimeTubeOne + 0xFFFFFFFF)- TIMER2_TAR_R ; // MIGHT NEED TO MAKE THIS VOLATILE
			}
			//if(distanceOne > 100){distanceOne = 0;} // 50cm cap
			tubeDoneOne = 0;
		}

	}
	
	if (GPIO_PORTB_RIS_R & 0x40) {// pb6
		GPIO_PORTB_ICR_R = 0x40;
		uint32_t test = TIMER2_TAR_R;
		if (tubeDoneTwo == 0) {
			tubeDoneTwo = 1;
			startTimeTubeTwo = TIMER2_TAR_R;
		} else if (tubeDoneTwo == 1) {
			if (TIMER2_TAR_R < startTimeTubeTwo) { 
				distanceTwo =  startTimeTubeTwo - TIMER2_TAR_R; /// 29; // Fixed point (units of 0.5cm) (moved to display.c)
			} else {
				distanceTwo = (startTimeTubeTwo + 0xFFFFFFFF)- TIMER2_TAR_R; // MIGHT NEED TO MAKE THIS VOLATILE
			}
			tubeDoneTwo = 0;
		}

	}
	
	if (GPIO_PORTB_RIS_R & 0x20) {// pb5
		GPIO_PORTB_ICR_R = 0x20;
		if (tubeDoneThree == 0) {
			tubeDoneThree = 1;
			startTimeTubeThree = TIMER2_TAR_R;
		} else if (tubeDoneThree == 1) {
			if (TIMER2_TAR_R < startTimeTubeThree) { 
				distanceThree = startTimeTubeThree - TIMER2_TAR_R; /// 29; // Fixed point (units of 0.5cm) (moved to display.c)
			} else {
				distanceThree = (startTimeTubeThree + 0xFFFFFFFF)- TIMER2_TAR_R; // MIGHT NEED TO MAKE THIS VOLATILE
			}
			tubeDoneThree = 0;
		}
		
	}
	
	//if (((tubeDoneOne == 1) || (tubeDoneTwo == 1) || (tubeDoneThree == 1)) && (firstClock == 0)) {
	//	firstClock = 1;
	//	Count = 0;
	//	Timer2A_Init(timeInMicro, 80, 7);
	//} 
	
	//if ((tubeDoneOne == 0) && (tubeDoneTwo == 0) && (tubeDoneThree == 0)){  // Added firstClock == 1
	//	Timer2A_Stop();
	//	firstClock = 0;
	//	Count = 0;
	//}	
}


void SendPulse(void) {
	if (Count2 == 6000) { // 60ms
		GPIO_PORTB_DATA_R |= 0x1C;
		Count2 = 0;
	} else {
		GPIO_PORTB_DATA_R &= ~0x1C;
	}
	Count2++;
}

void PWMeasure2_Init(void){ // TM4C123 code
  //SYSCTL_RCGCTIMER_R |= 0x01;      // activate timer0
  SYSCTL_RCGCGPIO_R |= 0x02;       // activate port B
	while ((SYSCTL_PRGPIO_R & 0x02) == 0) {}
  tubeDoneOne = 0;                        // allow time to finish activating
	tubeDoneTwo = 0;
	tubeDoneThree = 0;
	
  GPIO_PORTB_DIR_R &= ~0xE0;       // make PB5, PB6, PB7 inputs
  GPIO_PORTB_DEN_R |= 0xE0;        // enable digital PB7
  //GPIO_PORTB_AFSEL_R |= 0xE0;      // enable alt funct on PB7 #Not sure if necessary
	GPIO_PORTB_AFSEL_R &= ~0xE0;		//No alternate function
	GPIO_PORTB_IS_R &= ~0xE0; // make only detect edges
  GPIO_PORTB_IBE_R |= 0xE0; // Detect both edges
  //GPIO_PORTB_PCTL_R = (GPIO_PORTB_PCTL_R&0x00FFFFFF)+0x77000000; //Also not sure if necessary, only for alternate functions on PB6 and 7
	GPIO_PORTB_ICR_R = 0xE0; // clear interrupt flag
  GPIO_PORTB_IM_R |= 0xE0; // enable interrupts
	NVIC_PRI0_R = (NVIC_PRI0_R & 0xFFFFFF00) | 0x00000020;
	//NVIC_PRI0_R = (NVIC_PRI0_R & 0xFF1FFFFF) | (1 << 5);
  NVIC_EN0_R |= (1 << 1); // enable port b interrupt
	
  //TIMER0_CTL_R &= ~0x00000001;     // disable timers 0A and 0B
  //TIMER0_CFG_R = 0x00000004;       // configure for 16-bit timer mode
  // **** timer0A initialization ****
  //TIMER0_TAMR_R = 0x00000007;
  //TIMER0_CTL_R = (TIMER0_CTL_R&(~0x0C))+0x04; // falling edge TODO: should be rising
	//TIMER0_CTL_R = (TIMER0_CTL_R&(~0x0C))+0x00; //Rising
  //TIMER0_TAILR_R = 0x0000FFFF;     // start value
  //TIMER0_TAPR_R = 0xFF;            // activate prescale, creating 24-bit 
  //TIMER0_IMR_R |= 0x00000004;      // enable capture match interrupt
  //TIMER0_ICR_R = 0x00000004;       // clear timer0A capture match flag
  // **** timer0B initialization ****
//  TIMER0_TBMR_R = 0x00000007;
//  TIMER0_CTL_R = (TIMER0_CTL_R&(~0x0C00))+0x00; // rising edge TODO: should be falling
//  TIMER0_TBILR_R = 0x0000FFFF;     // start value
//  TIMER0_TBPR_R = 0xFF;            // activate prescale, creating 24-bit 
//  TIMER0_IMR_R &= ~0x0700;         // disable all interrupts for timer0B
	
  //TIMER0_CTL_R |= 0x00000001;      // enable timers 0A
  // **** interrupt initialization ****
	SYSCTL_RCGCGPIO_R |= 0x02; // enable clock for Port B
	while ((SYSCTL_PRGPIO_R & 0x02) == 0) {}
	GPIO_PORTB_DIR_R |= 0x1E; // set pb2,3,4 as output (connect to speaker)
	GPIO_PORTB_DEN_R |= 0x1E; // enable digital I/O on pb2,3,4
		
	Timer3A_Init(&SendPulse, 800, 7); // -> set this to PWM pin?
}

uint32_t GetDesiredPosition() {
	return distanceOne;
}

uint32_t GetLeftPosition() {
	return distanceTwo;
}

uint32_t GetRightPosition() {
	return distanceThree;
}

void SensorSetup() {
	Timer2PWMMeasure_Init();
	PWMeasure2_Init();      // initialize 24-bit timer0A in capture mode
}


//debug code
int mainDistance(void){           
  PLL_Init(Bus80MHz);     // 80 MHz clock
  LaunchPad_Init();       // activate port F
	SensorSetup();
	
  EnableInterrupts();
  while(1){
    WaitForInterrupt();
  }
}
