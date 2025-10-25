#include "Buttons.h"
#include "../inc/SysTick.h"

volatile uint32_t Mode = 0;
volatile uint32_t oldMode = 0;
uint32_t first = 0;

void ButtonsInit(void){
    // Initializing PF2 for button input instead of PF0
    SYSCTL_RCGCGPIO_R |= 0x20;    // clock for port F
    GPIO_PORTF_DIR_R &= ~0x04;     // PF2 as input (bit 2 = 0)
    GPIO_PORTF_DEN_R |= 0x04;      // enable digital I/O on PF2
    GPIO_PORTF_PUR_R |= 0x04;      // enable pull-up resistor on PF2
    GPIO_PORTF_IS_R &= ~0x04;      // edge-sensitive
    GPIO_PORTF_IBE_R &= ~0x04;     // not both edges
    GPIO_PORTF_IEV_R &= ~0x04;     // falling edge trigger
    GPIO_PORTF_ICR_R = 0x04;       // clear interrupt flag
    GPIO_PORTF_IM_R |= 0x04;       // arm interrupt

    // Set interrupt priority
    NVIC_PRI7_R = (NVIC_PRI7_R&0xFFFFFF00)|0x00000020; // priority 1
    NVIC_EN0_R |= (1 << 30);       // enable interrupt 30 in NVIC (Port F)
}

void GPIOPortF_Handler(void){
    GPIO_PORTF_ICR_R = 0x04;     // clear interrupt flag for PF2
    // Increment mode by 1 and resets on 3.
    // We have 3 modes (mode 0,1,2) to have different functionalities for our system
    if (GPIO_PORTF_RIS_R & 0x04) {  // PF2
            if ((GPIO_PORTF_DATA_R & 0x04)==0) {
                return;
            }
    } else {
        return;
    }
    GPIO_PORTF_IM_R &= ~0x04;
    SysTick80_Wait10ms(3);
    GPIO_PORTF_IM_R |= 0x04;
  if (GPIO_PORTF_RIS_R & 0x04) {  // PF2
        if ((GPIO_PORTF_DATA_R & 0x04) == 0) {
            return;
        }
            Mode = (Mode+1)%3;
    }
}

uint32_t getMode(){
    return Mode;
}