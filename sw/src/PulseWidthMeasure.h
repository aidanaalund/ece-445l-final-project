#ifndef PULSEWIDTHMEASURE_H_
#define PULSEWIDTHMEASURE_H_

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/PLL.h"
#include "../inc/CortexM.h"
#include "../inc/LaunchPad.h"

void PWMeasure2_Init(void);
void Timer2PWMMeasure_Init(void);
void SensorSetup(void);
uint32_t GetDesiredPosition(void);
uint32_t GetLeftPosition(void);
uint32_t GetRightPosition(void);

#endif