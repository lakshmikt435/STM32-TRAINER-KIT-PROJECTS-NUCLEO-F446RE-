#ifndef SSD1306_H
#define SSD1306_H

#include "main.h"

#define SSD1306_I2C_ADDR (0x3C << 1)

void SSD1306_Init(void);
void SSD1306_UpdateScreen(void);
void SSD1306_Clear(void);
void SSD1306_GotoXY(uint8_t x, uint8_t y);
void SSD1306_Puts(char* str, void* Font, uint8_t color);

#endif
