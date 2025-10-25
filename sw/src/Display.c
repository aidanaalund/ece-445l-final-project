#include "Display.h"
#include <stdint.h>
#include "../inc/ST7735.h"
#include <stdio.h>
#include <string.h>
#include "PulseWidthMeasure.h"
#include "Buttons.h"

#define MAX_DISTANCE_CM 30
#define SCREEN_HEIGHT 160
#define BALL_RADIUS 5
// Tube vertical range
#define TUBE_Y_TOP    45   // Top of the tube 40 
#define TUBE_Y_BOTTOM 155  // Bottom of the tube 155
#define TUBE_HEIGHT 110 // difference of bottom to top/2 since measuring in 

uint32_t mode;

extern volatile uint8_t refreshFlag;

void SysTick_Init_30kHz(void){
	NVIC_ST_CTRL_R = 0;
	NVIC_ST_RELOAD_R = 2666-1; // 80Mhz/30khz = 2666
	NVIC_ST_CURRENT_R = 0;
	NVIC_ST_CTRL_R = NVIC_ST_CTRL_ENABLE | NVIC_ST_CTRL_CLK_SRC | NVIC_ST_CTRL_INTEN;
	NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R & 0x00FFFFFF) | 0x40000000; // Set priority to 2
}

void SysTick_Handler(void){
	refreshFlag = 1;
}

void Display_Init(){
	ST7735_InitR(INITR_GREENTAB);
	ST7735_FillScreen(0x0000);
	Draw_Background();
}

// Fill Circle function from chatgpt
void ST7735_FillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
    int16_t x, y;
    for (y = -r; y <= r; y++) {
        for (x = -r; x <= r; x++) {
            if (x * x + y * y <= r * r) {
                ST7735_DrawPixel(x0 + x, y0 + y, color);
            }
        }
    }
}

void Draw_Background(){
	// 128 pixels wide by 160 pixels high
	// NEEDS TO BE TESTED ON DISPLAY
	// Represent the tubes by drawing lines on the background
	// Left tube:
	ST7735_DrawFastVLine(30,40,160,ST7735_WHITE);
	ST7735_DrawFastVLine(50, 40, 160, ST7735_WHITE);
	ST7735_DrawFastHLine(30, 40, 20, ST7735_WHITE);

	// Right tube:
	ST7735_DrawFastVLine(78, 40, 160, ST7735_WHITE);
  ST7735_DrawFastVLine(98, 40, 160, ST7735_WHITE);
	ST7735_DrawFastHLine(78, 40, 20, ST7735_WHITE);
	

}
/*
int MapDistanceToY(uint32_t cm){
	if (cm > MAX_DISTANCE_CM) cm = MAX_DISTANCE_CM;
	//if (cm<=0) return SCREEN_HEIGHT-10;
	return SCREEN_HEIGHT-(cm*SCREEN_HEIGHT/MAX_DISTANCE_CM);
}*/


int MapDistanceToY(uint32_t cm){
    //if (cm > MAX_DISTANCE_CM) cm = MAX_DISTANCE_CM;
		//if (cm <= 0) return 155;

    //return TUBE_Y_BOTTOM - (cm * TUBE_HEIGHT / MAX_DISTANCE_CM);
	
	  if (cm > MAX_DISTANCE_CM) cm = MAX_DISTANCE_CM;
    if (cm < 0) cm = 0;

    return TUBE_Y_TOP + (cm * TUBE_HEIGHT / MAX_DISTANCE_CM);
}

uint32_t subtractTwo = 0;
uint32_t subtractThree = 0;

void Draw_Refresh(){
	static int prevLeftY = -1;
	static int prevRightY = -1;
	static int prevRefY = -1;
	uint32_t mode = getMode();
	
	char handDistance[30];
	uint32_t distanceOne = GetDesiredPosition() / 2320; //(80cycles per us * 29)
	if (distanceOne > 60) {distanceOne = 60;}
	uint32_t distanceTwo = GetLeftPosition() / 2320; //(80cycles per us * 29)
	if (distanceTwo > 60) {distanceTwo = 60;}
	uint32_t distanceThree = GetRightPosition() / 2320; //(80cycles per us * 29)
	if (distanceThree > 60) {distanceThree = 60;}
	
	if (distanceTwo == 59) {
		subtractTwo = 0;
	} else {
		subtractTwo = (60 - distanceTwo - ((distanceTwo%2)*2))>>1;
	}
	
	if (distanceThree == 59) {
		subtractThree = 0;
	} else {
		subtractThree = (60 - distanceThree - ((distanceThree%2)*2))>>1;
	}
	
	sprintf(handDistance, "hand: %02d.%02d cm", distanceOne>>1, (distanceOne%2)*50); //printf but returns instead of printing.
	ST7735_SetCursor(0, 0); // set writing on led to top left corner
	ST7735_OutString(handDistance);
	char ballDistance[30];
	sprintf(ballDistance, "ball: %02d.%02d cm", subtractTwo, (distanceTwo%2)*50); // TODO: make it distance from bottom of tube.
	ST7735_SetCursor(0, 1); // set writing on led to top left corner
	ST7735_OutString(ballDistance);
	char ballDistance2[30];
	sprintf(ballDistance2, "ball2: %02d.%02d cm", subtractThree, (distanceThree%2)*50); // TODO: make it distance from bottom of tube.
	ST7735_SetCursor(0, 2); // set writing on led to top left corner
	ST7735_OutString(ballDistance2);
	ST7735_SetCursor(0,3);
	char modeMsg[20];
	ST7735_SetCursor(0, 3);  // Line after ball2
	
	if (mode == 0) {
			ST7735_DrawFastHLine(0, 3, 4, ST7735_BLACK);
			sprintf(modeMsg, "Mode: hover      ");
	} else if (mode == 1) {
			ST7735_DrawFastHLine(0, 3, 4, ST7735_BLACK);
			sprintf(modeMsg, "Mode: magic wand!");
	} else if (mode == 2) {
			ST7735_DrawFastHLine(0, 3, 4, ST7735_BLACK);
			sprintf(modeMsg, "Mode: other      ");
	} else {
			sprintf(modeMsg, "Mode: unknown");
	}

	ST7735_OutString(modeMsg);
	
	// Draw ball in the left tube
	int currLeftY = MapDistanceToY(distanceTwo>>1);
	int currRightY = MapDistanceToY(distanceThree>>1);
	int currRefY = MapDistanceToY((60-distanceOne)>>1);
	//int currLeftY = MapDistanceToY(0);
	//int currRightY = MapDistanceToY(4);
	// erase 
	if(prevLeftY!=-1){
		ST7735_FillCircle(40, prevLeftY, BALL_RADIUS, ST7735_BLACK);
		
	}
	// Draw new left ball
	ST7735_FillCircle(40, currLeftY, BALL_RADIUS, ST7735_CYAN);
	//debug
	//ST7735_FillCircle(40, 100, BALL_RADIUS, ST7735_RED);
	prevLeftY = currLeftY;
	
	// Erase old right ball if not first frame
	if(prevRightY !=-1){
		ST7735_FillCircle(88, prevRightY, BALL_RADIUS, ST7735_BLACK);
	}
	// Draw new right ball
	ST7735_FillCircle(88, currRightY, BALL_RADIUS, ST7735_RED);
	//ST7735_FillCircle(88, 100, BALL_RADIUS, ST7735_GREEN);
	prevRightY = currRightY;
	
	if(prevRefY != -1){
		ST7735_DrawFastHLine(52,prevRefY,25,ST7735_BLACK);
	}
	ST7735_DrawFastHLine(52,currRefY,25,ST7735_YELLOW);
	prevRefY = currRefY;
	
	
}

