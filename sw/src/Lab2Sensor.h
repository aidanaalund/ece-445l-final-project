#ifndef LAB2SENSOR_H_
#define LAB2SENSOR_H_

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/PLL.h"
#include "../inc/CortexM.h"
#include "../inc/LaunchPad.h"

void ADC_Init(void);
int32_t ADC_In(void);

#endif