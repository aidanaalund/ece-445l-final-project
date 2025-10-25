// Lab10.c
/*	
	Authors: Aidan Aalund, 
	Alexa Aguilar Izquierdo, Eric Zang, 
	Brian Ahn, Paul Schnoebelen
*/
// March 10th, 2025

// TODO: add includes
#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/PLL.h"
#include "../inc/CortexM.h"
#include "../inc/LaunchPad.h"

#include "PulseWidthMeasure.h" // code for HC-SR40 sensor (could rename to DistanceSensor)
#include "Motor.h" // code for interfacing with the motor, PWM-ing a MOSFET (here for now, remove later)
#include "Display.h" // Adafruit display will show important info
#include "Buttons.h" // code for the mode switch button

// Globals
volatile uint8_t refreshFlag=0;


int main(){
	DisableInterrupts();
	
	// TODO: initialize drivers and devices here
	PLL_Init(Bus80MHz);     // 80 MHz clock
  LaunchPad_Init();       // activate port F
	Display_Init();
	//ButtonsInit();
	SensorSetup();
	MotorInit(); // Inits motors and control loop
	SysTick_Init_30kHz(); // 30Hz refresh rate
	
	
	EnableInterrupts();
	while(1){
		if (refreshFlag==1){
			refreshFlag=0;
			Draw_Refresh();
		}
	}
}