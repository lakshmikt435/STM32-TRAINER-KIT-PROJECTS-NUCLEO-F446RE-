#include "ssd1306.h"
#include "i2c.h"
#include "fonts.h"
#include <string.h>

#define WIDTH 128
#define HEIGHT 64

static uint8_t buffer[WIDTH * HEIGHT / 8];
static uint8_t currentX = 0;
static uint8_t currentY = 0;

void SSD1306_WriteCommand(uint8_t cmd)
{
  uint8_t data[2] = {0x00, cmd};
  HAL_I2C_Master_Transmit(&hi2c1, SSD1306_I2C_ADDR, data, 2, 100);
}

void SSD1306_Init(void)
{
  HAL_Delay(100);

  SSD1306_WriteCommand(0xAE);
  SSD1306_WriteCommand(0x20);
  SSD1306_WriteCommand(0x10);
  SSD1306_WriteCommand(0xB0);
  SSD1306_WriteCommand(0xC8);
  SSD1306_WriteCommand(0x00);
  SSD1306_WriteCommand(0x10);
  SSD1306_WriteCommand(0x40);
  SSD1306_WriteCommand(0x81);
  SSD1306_WriteCommand(0xFF);
  SSD1306_WriteCommand(0xA1);
  SSD1306_WriteCommand(0xA6);
  SSD1306_WriteCommand(0xA8);
  SSD1306_WriteCommand(0x3F);
  SSD1306_WriteCommand(0xA4);
  SSD1306_WriteCommand(0xD3);
  SSD1306_WriteCommand(0x00);
  SSD1306_WriteCommand(0xD5);
  SSD1306_WriteCommand(0xF0);
  SSD1306_WriteCommand(0xD9);
  SSD1306_WriteCommand(0x22);
  SSD1306_WriteCommand(0xDA);
  SSD1306_WriteCommand(0x12);
  SSD1306_WriteCommand(0xDB);
  SSD1306_WriteCommand(0x20);
  SSD1306_WriteCommand(0x8D);
  SSD1306_WriteCommand(0x14);
  SSD1306_WriteCommand(0xAF);

  SSD1306_Clear();
  SSD1306_UpdateScreen();
}

void SSD1306_Clear(void)
{
  memset(buffer, 0, sizeof(buffer));
}

void SSD1306_GotoXY(uint8_t x, uint8_t y)
{
  currentX = x;
  currentY = y;
}

extern const uint8_t font5x7[][5];

void SSD1306_DrawChar(char ch)
{
  uint8_t index;

  if (ch == ' ') index = 0;
  else if (ch >= '0' && ch <= '9') index = (ch - '0') + 1;
  else if (ch >= 'A' && ch <= 'Z') index = (ch - 'A') + 11;
  else return;

  for (int i = 0; i < 5; i++)
  {
    uint8_t line = font5x7[index][i];

    for (int j = 0; j < 8; j++)
    {
      if (line & (1 << j))
      {
        buffer[(currentX + i) + ((currentY + j)/8)*WIDTH] |= (1 << ((currentY + j)%8));
      }
    }
  }

  currentX += 6;
}

void SSD1306_Puts(char* str, void* Font, uint8_t color)
{
  while (*str)
  {
    SSD1306_DrawChar(*str);
    str++;
  }
}

void SSD1306_UpdateScreen(void)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    SSD1306_WriteCommand(0xB0 + i);
    SSD1306_WriteCommand(0x00);
    SSD1306_WriteCommand(0x10);

    uint8_t data[129];
    data[0] = 0x40;
    memcpy(&data[1], &buffer[WIDTH * i], WIDTH);

    HAL_I2C_Master_Transmit(&hi2c1, SSD1306_I2C_ADDR, data, 129, 100);
  }
}
