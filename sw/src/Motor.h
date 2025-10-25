#ifndef MOTOR_H_
#define MOTOR_H_

#include <stdint.h>
// Initializes the Motor drivers
void MotorInit(void);

// x is ball position (state), currentPWM is current PWM level output is PWM
void PIControlLoop(void);
#endif