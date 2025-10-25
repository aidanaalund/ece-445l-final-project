#ifndef DISPLAY_H_
#define DISPLAY_H_

void SysTick_Init_30kHz(void);

void Display_Init();

// Makes a call to draw the background (ONLY CALL ON STARTUP)
void Draw_Background();

// draw call in main loop 
void Draw_Refresh();

//void ST7735_FillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color);


#endif // DISPLAY_H