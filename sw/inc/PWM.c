// PWM.c
// Runs on TM4C123
// Use PWM0A/PB6 and PWM0B/PB7 to generate pulse-width modulated outputs.
// Daniel Valvano
// March 28, 2014

/* This example accompanies the book
   "Embedded Systems: Real Time Interfacing to Arm Cortex M Microcontrollers",
   ISBN: 978-1463590154, Jonathan Valvano, copyright (c) 2015
   Program 6.8, section 6.3.2

   "Embedded Systems: Real-Time Operating Systems for ARM Cortex M Microcontrollers",
   ISBN: 978-1466468863, Jonathan Valvano, copyright (c) 2015
   Program 8.4, Section 8.3

 Copyright 2015 by Jonathan W. Valvano, valvano@mail.utexas.edu
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
#include "../inc/tm4c123gh6pm.h"
#define PWM_0_GENA_ACTCMPAD_ONE 0x000000C0  // Set the output signal to 1
#define PWM_0_GENA_ACTLOAD_ZERO 0x00000008  // Set the output signal to 0
#define PWM_0_GENB_ACTCMPBD_ONE 0x00000C00  // Set the output signal to 1
#define PWM_0_GENB_ACTLOAD_ZERO 0x00000008  // Set the output signal to 0

#define SYSCTL_RCC_USEPWMDIV    0x00100000  // Enable PWM Clock Divisor
#define SYSCTL_RCC_PWMDIV_M     0x000E0000  // PWM Unit Clock Divisor
#define SYSCTL_RCC_PWMDIV_2     0x00000000  // /2


// period is 16-bit number of PWM clock cycles in one period (3<=period)
// period for PB6 and PB7 must be the same
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
// PWM clock rate = processor clock rate/SYSCTL_RCC_PWMDIV
//                = BusClock/2
//                = 80 MHz/2 = 40 MHz (in this example)
// Output on PB6/M0PWM0
void PWM0A_Init(uint16_t period, uint16_t duty){
  SYSCTL_RCGCPWM_R |= 0x01;             // 1) activate PWM0
  SYSCTL_RCGCGPIO_R |= 0x02;            // 2) activate port B
  while((SYSCTL_PRGPIO_R&0x02) == 0){};
  GPIO_PORTB_AFSEL_R |= 0x40;           // enable alt funct on PB6
  GPIO_PORTB_PCTL_R &= ~0x0F000000;     // configure PB6 as PWM0
  GPIO_PORTB_PCTL_R |= 0x04000000;
  GPIO_PORTB_AMSEL_R &= ~0x40;          // disable analog functionality on PB6
  GPIO_PORTB_DEN_R |= 0x40;             // enable digital I/O on PB6
  SYSCTL_RCC_R = 0x00100000 |           // 3) use PWM divider
      (SYSCTL_RCC_R & (~0x000E0000));   //    configure for /2 divider
  PWM0_0_CTL_R = 0;                     // 4) re-loading down-counting mode
  PWM0_0_GENA_R = 0xC8;                 // low on LOAD, high on CMPA down
  // PB6 goes low on LOAD
  // PB6 goes high on CMPA down
  PWM0_0_LOAD_R = period - 1;           // 5) cycles needed to count down to 0
  PWM0_0_CMPA_R = duty - 1;             // 6) count value when output rises
  PWM0_0_CTL_R |= 0x00000001;           // 7) start PWM0
  PWM0_ENABLE_R |= 0x00000001;          // enable PB6/M0PWM0
}
// change duty cycle of PB6
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
void PWM0A_Duty(uint16_t duty){
  PWM0_0_CMPA_R = duty - 1;             // 6) count value when output rises
}
// period is 16-bit number of PWM clock cycles in one period (3<=period)
// period for PB6 and PB7 must be the same
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
// PWM clock rate = processor clock rate/SYSCTL_RCC_PWMDIV
//                = BusClock/2
//                = 80 MHz/2 = 40 MHz (in this example)
// Output on PB7/M0PWM1
void PWM0B_Init(uint16_t period, uint16_t duty){
  volatile unsigned long delay;
  SYSCTL_RCGCPWM_R |= 0x01;             // 1) activate PWM0
  SYSCTL_RCGCGPIO_R |= 0x02;            // 2) activate port B
  delay = SYSCTL_RCGCGPIO_R;            // allow time to finish activating
  GPIO_PORTB_AFSEL_R |= 0x80;           // enable alt funct on PB7
  GPIO_PORTB_PCTL_R &= ~0xF0000000;     // configure PB7 as M0PWM1
  GPIO_PORTB_PCTL_R |= 0x40000000;
  GPIO_PORTB_AMSEL_R &= ~0x80;          // disable analog functionality on PB7
  GPIO_PORTB_DEN_R |= 0x80;             // enable digital I/O on PB7
  SYSCTL_RCC_R |= SYSCTL_RCC_USEPWMDIV; // 3) use PWM divider
  SYSCTL_RCC_R &= ~SYSCTL_RCC_PWMDIV_M; //    clear PWM divider field
  SYSCTL_RCC_R += SYSCTL_RCC_PWMDIV_2;  //    configure for /2 divider
  PWM0_0_CTL_R = 0;                     // 4) re-loading down-counting mode
  PWM0_0_GENB_R = (PWM_0_GENB_ACTCMPBD_ONE|PWM_0_GENB_ACTLOAD_ZERO);
  // PB7 goes low on LOAD
  // PB7 goes high on CMPB down
  PWM0_0_LOAD_R = period - 1;           // 5) cycles needed to count down to 0
  PWM0_0_CMPB_R = duty - 1;             // 6) count value when output rises
  PWM0_0_CTL_R |= 0x00000001;           // 7) start PWM0
  PWM0_ENABLE_R |= 0x00000002;          // enable PB7/M0PWM1
}
// change duty cycle of PB7
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
void PWM0B_Duty(uint16_t duty){
  PWM0_0_CMPB_R = duty - 1;             // 6) count value when output rises
}

// period is 16-bit number of PWM clock cycles in one period (3<=period)
// period for PD1 and PD0 must be the same
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
// PWM clock rate = processor clock rate/SYSCTL_RCC_PWMDIV
//                = BusClock/2
//                = 80 MHz/2 = 40 MHz (in this example)
// Output on PD0/M0PWM6
void PWM0G_Init(uint16_t period, uint16_t duty){
  SYSCTL_RCGCPWM_R |= 0x01;             // 1) activate PWM0
  SYSCTL_RCGCGPIO_R |= 0x08;            // 2) activate port D
  while((SYSCTL_PRGPIO_R&0x08) == 0){};
  GPIO_PORTD_AFSEL_R |= 0x01;           // enable alt funct on PD0
  GPIO_PORTD_PCTL_R &= ~0x0000000F;     // configure PD0 as PWM6
  GPIO_PORTD_PCTL_R |= 0x00000004;
  GPIO_PORTD_AMSEL_R &= ~0x01;          // disable analog functionality on PD0
  GPIO_PORTD_DEN_R |= 0x01;             // enable digital I/O on PD0
  SYSCTL_RCC_R = 0x00100000 |           // 3) use PWM divider
      (SYSCTL_RCC_R & (~0x000E0000));   //    configure for /2 divider
  PWM0_3_CTL_R = 0;                     // 4) re-loading down-counting mode
  PWM0_3_GENA_R = 0xC8;                 // low on LOAD, high on CMPA down
  // PB6 goes low on LOAD
  // PB6 goes high on CMPA down
  PWM0_3_LOAD_R = period - 1;           // 5) cycles needed to count down to 0
  PWM0_3_CMPA_R = duty - 1;             // 6) count value when output rises
  PWM0_3_CTL_R |= 0x00000001;           // 7) start PWM0
  PWM0_ENABLE_R |= 0x00000040;          // enable PD0/M0PWM6
}
// change duty cycle of PB6
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
void PWM0G_Duty(uint16_t duty){
  PWM0_3_CMPA_R = duty - 1;             // 6) count value when output rises
}

// period is 16-bit number of PWM clock cycles in one period (3<=period)
// duty is number of PWM clock cycles output is high (2<=duty<=period-1)
// PWM clock rate = processor clock rate/SYSCTL_RCC_PWMDIV
//                = BusClock/2
//                = 80 MHz/2 = 40 MHz (in this example)
// Output on PD1/M1PWM1
void PWM1B_Init(uint16_t period, uint16_t duty){
  SYSCTL_RCGCPWM_R |= 0x02;             // 1) activate PWM1
  SYSCTL_RCGCGPIO_R |= 0x08;            // 2) activate port D
  while((SYSCTL_PRGPIO_R&0x08) == 0){}; // wait for port D to be ready
  GPIO_PORTD_AFSEL_R |= 0x02;           // enable alt funct on PD1
  GPIO_PORTD_PCTL_R &= ~0x000000F0;     // configure PD1 as M1PWM1
  GPIO_PORTD_PCTL_R |= 0x00000050;      // 5 specifies M1PWM1 function
  GPIO_PORTD_AMSEL_R &= ~0x02;          // disable analog functionality on PD1
  GPIO_PORTD_DEN_R |= 0x02;             // enable digital I/O on PD1
  
  // Configure PWM settings with same pattern as PWM0B_Init
  SYSCTL_RCC_R |= SYSCTL_RCC_USEPWMDIV; // use PWM divider
  SYSCTL_RCC_R &= ~SYSCTL_RCC_PWMDIV_M; // clear PWM divider field
  SYSCTL_RCC_R += SYSCTL_RCC_PWMDIV_2;  // configure for /2 divider
  
  PWM1_0_CTL_R = 0;                     // re-loading down-counting mode
  PWM1_0_GENB_R = (PWM_0_GENB_ACTCMPBD_ONE|PWM_0_GENB_ACTLOAD_ZERO);
  // PD1 goes low on LOAD
  // PD1 goes high on CMPB down
  PWM1_0_LOAD_R = period - 1;           // cycles needed to count down to 0
  PWM1_0_CMPB_R = duty - 1;             // count value when output rises
  PWM1_0_CTL_R |= 0x00000001;           // start PWM1 generator 0
  PWM1_ENABLE_R |= 0x00000002;          // enable PD1/M1PWM1 output
}

// change duty cycle of PD1/M1PWM1
// duty is number of PWM clock cycles output is high (2<=duty<=period-1)
void PWM1B_Duty(uint16_t duty){
  PWM1_0_CMPB_R = duty - 1;             // count value when output rises
}

// period is 16-bit number of PWM clock cycles in one period (3<=period)
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
// PWM clock rate = processor clock rate/SYSCTL_RCC_PWMDIV
//                = BusClock/2
//                = 80 MHz/2 = 40 MHz (in this example)
// Output on PE4/M0PWM4
void PWM0E_Init(uint32_t period, uint32_t duty){
  SYSCTL_RCGCPWM_R |= 0x01;             // 1) activate PWM0
  SYSCTL_RCGCGPIO_R |= 0x10;            // 2) activate port E
  while((SYSCTL_PRGPIO_R&0x10) == 0){}; // wait for port E to be ready
  GPIO_PORTE_AFSEL_R |= 0x10;           // enable alt funct on PE4
  GPIO_PORTE_PCTL_R &= ~0x000F0000;     // configure PE4 as PWM4
  GPIO_PORTE_PCTL_R |= 0x00040000;      // 4 specifies M0PWM4 function
  GPIO_PORTE_AMSEL_R &= ~0x10;          // disable analog functionality on PE4
  GPIO_PORTE_DEN_R |= 0x10;             // enable digital I/O on PE4
  SYSCTL_RCC_R = 0x00100000 |           // 3) use PWM divider
      (SYSCTL_RCC_R & (~0x000E0000));   //    configure for /2 divider
  PWM0_2_CTL_R = 0;                     // 4) re-loading down-counting mode
  PWM0_2_GENA_R = 0xC8;                 // low on LOAD, high on CMPA down
  // PE4 goes low on LOAD
  // PE4 goes high on CMPA down
  PWM0_2_LOAD_R = period - 1;           // 5) cycles needed to count down to 0
  PWM0_2_CMPA_R = duty - 1;             // 6) count value when output rises
  PWM0_2_CTL_R |= 0x00000001;           // 7) start PWM0
  PWM0_ENABLE_R |= 0x00000010;          // enable PE4/M0PWM4
}

// change duty cycle of PE4
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
void PWM0E_Duty(uint16_t duty){
  PWM0_2_CMPA_R = duty - 1;             // count value when output rises
}

// period is 16-bit number of PWM clock cycles in one period (3<=period)
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
// PWM clock rate = processor clock rate/SYSCTL_RCC_PWMDIV
//                = BusClock/2
//                = 80 MHz/2 = 40 MHz (in this example)
// Output on PE5/M1PWM3
void PWM1F_Init(uint16_t period, uint16_t duty){
  SYSCTL_RCGCPWM_R |= 0x02;             // 1) activate PWM1
  SYSCTL_RCGCGPIO_R |= 0x10;            // 2) activate port E
  while((SYSCTL_PRGPIO_R&0x10) == 0){}; // wait for port E to be ready
  GPIO_PORTE_AFSEL_R |= 0x20;           // enable alt funct on PE5
  GPIO_PORTE_PCTL_R &= ~0x00F00000;     // configure PE5 as M1PWM3
  GPIO_PORTE_PCTL_R |= 0x00500000;      // 5 specifies M1PWM3 function
  GPIO_PORTE_AMSEL_R &= ~0x20;          // disable analog functionality on PE5
  GPIO_PORTE_DEN_R |= 0x20;             // enable digital I/O on PE5
  
  SYSCTL_RCC_R |= SYSCTL_RCC_USEPWMDIV; // 3) use PWM divider
  SYSCTL_RCC_R &= ~SYSCTL_RCC_PWMDIV_M; //    clear PWM divider field
  SYSCTL_RCC_R += SYSCTL_RCC_PWMDIV_2;  //    configure for /2 divider
  
  PWM1_1_CTL_R = 0;                     // 4) re-loading down-counting mode
  PWM1_1_GENB_R = 0xC8;                 // low on LOAD, high on CMPB down
  // PE5 goes low on LOAD
  // PE5 goes high on CMPB down
  PWM1_1_LOAD_R = period - 1;           // 5) cycles needed to count down to 0
  PWM1_1_CMPB_R = duty - 1;             // 6) count value when output rises
  PWM1_1_CTL_R |= 0x00000001;           // 7) start PWM1 generator 1
  PWM1_ENABLE_R |= 0x00000008;          // enable PE5/M1PWM3
}

// change duty cycle of PE5/M1PWM3
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
void PWM1F_Duty(uint16_t duty){
  PWM1_1_CMPB_R = duty - 1;             // count value when output rises
}

// period is 16-bit number of PWM clock cycles in one period (3<=period)
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
// PWM clock rate = processor clock rate/SYSCTL_RCC_PWMDIV
//                = BusClock/2
//                = 80 MHz/2 = 40 MHz (in this example)
// Output on PE5/M0PWM5
void PWM0F_Init(uint32_t period, uint32_t duty){
  SYSCTL_RCGCPWM_R |= 0x01;             // 1) activate PWM0
  SYSCTL_RCGCGPIO_R |= 0x10;            // 2) activate port E
  while((SYSCTL_PRGPIO_R&0x10) == 0){}; // wait for port E to be ready
  GPIO_PORTE_AFSEL_R |= 0x20;           // enable alt funct on PE5
  GPIO_PORTE_PCTL_R &= ~0x00F00000;     // configure PE5 as M0PWM5
  GPIO_PORTE_PCTL_R |= 0x00400000;      // 4 specifies M0PWM5 function
  GPIO_PORTE_AMSEL_R &= ~0x20;          // disable analog functionality on PE5
  GPIO_PORTE_DEN_R |= 0x20;             // enable digital I/O on PE5
  
  SYSCTL_RCC_R |= SYSCTL_RCC_USEPWMDIV; // 3) use PWM divider
  SYSCTL_RCC_R &= ~SYSCTL_RCC_PWMDIV_M; //    clear PWM divider field
  SYSCTL_RCC_R += SYSCTL_RCC_PWMDIV_2;  //    configure for /2 divider
  
  PWM0_2_CTL_R = 0;                     // 4) re-loading down-counting mode
  PWM0_2_GENB_R = (PWM_0_GENB_ACTCMPBD_ONE|PWM_0_GENB_ACTLOAD_ZERO);
  // PE5 goes low on LOAD
  // PE5 goes high on CMPB down
  PWM0_2_LOAD_R = period - 1;           // 5) cycles needed to count down to 0
  PWM0_2_CMPB_R = duty - 1;             // 6) count value when output rises
  PWM0_2_CTL_R |= 0x00000001;           // 7) start PWM0 generator 2
  PWM0_ENABLE_R |= 0x00000020;          // enable PE5/M0PWM5
}

// change duty cycle of PE5/M0PWM5
// duty is number of PWM clock cycles output is high  (2<=duty<=period-1)
void PWM0F_Duty(uint16_t duty){
  PWM0_2_CMPB_R = duty - 1;             // count value when output rises
}