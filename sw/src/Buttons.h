#ifndef BUTTONS_H_
#define BUTTONS_H_

#include <stdint.h>
#include "../inc/tm4c123gh6pm.h"
#include "../inc/PLL.h"
#include "../inc/CortexM.h"
#include "../inc/LaunchPad.h"

void ButtonsInit(void); // initialize the button(s)

void OnPress(void); // turn off the fans, happens on a button press

uint32_t getMode(void);





#endif