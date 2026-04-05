#include "main.h"
#include "ssd1306.h"
#include <stdio.h>

/* EXTERN */
extern ADC_HandleTypeDef hadc1;
extern I2C_HandleTypeDef hi2c1;

/* DHT PIN */
#define DHT_PORT GPIOA
#define DHT_PIN  GPIO_PIN_9

/* FUNCTION DECLARATIONS */
void SystemClock_Config(void);
void Error_Handler(void);
void MX_GPIO_Init(void);
void MX_I2C1_Init(void);
void MX_ADC1_Init(void);

/* MICROSECOND DELAY */
void delay_us(uint16_t us)
{
  uint32_t start = DWT->CYCCNT;
  uint32_t ticks = us * (HAL_RCC_GetHCLKFreq()/1000000);
  while ((DWT->CYCCNT - start) < ticks);
}

/* ===== DHT ===== */
uint8_t DHT_Read(void)
{
  uint8_t data = 0;

  for(int i=0;i<8;i++)
  {
    while(!HAL_GPIO_ReadPin(DHT_PORT,DHT_PIN));
    delay_us(40);

    if(HAL_GPIO_ReadPin(DHT_PORT,DHT_PIN))
      data |= (1<<(7-i));

    while(HAL_GPIO_ReadPin(DHT_PORT,DHT_PIN));
  }
  return data;
}

void DHT_Start(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  GPIO_InitStruct.Pin = DHT_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  HAL_GPIO_Init(DHT_PORT, &GPIO_InitStruct);

  HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_RESET);
  HAL_Delay(18);

  HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_SET);
  delay_us(30);

  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(DHT_PORT, &GPIO_InitStruct);
}

/* ===== MAIN ===== */
int main(void)
{
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_ADC1_Init();

  /* Enable DWT */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  SSD1306_Init();

  char buffer[32];

  while (1)
  {
    /* ===== LDR ===== */
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
    uint32_t ldr = HAL_ADC_GetValue(&hadc1);

    char *light = (ldr < 2000) ? "DARK" : "BRIGHT";

    /* ===== DHT ===== */
    DHT_Start();

    while(HAL_GPIO_ReadPin(DHT_PORT,DHT_PIN));
    while(!HAL_GPIO_ReadPin(DHT_PORT,DHT_PIN));
    while(HAL_GPIO_ReadPin(DHT_PORT,DHT_PIN));

    uint8_t hum_int = DHT_Read();
    uint8_t hum_dec = DHT_Read();
    uint8_t temp = DHT_Read();
    uint8_t temp_dec = DHT_Read();
    DHT_Read(); // checksum

    /* ===== OLED DISPLAY ===== */
    SSD1306_Clear();

    sprintf(buffer, "TEMP %dC", temp);
    SSD1306_GotoXY(0,0);
    SSD1306_Puts(buffer, NULL, 1);

    sprintf(buffer, "HUM  %d%%", hum_int);
    SSD1306_GotoXY(0,16);
    SSD1306_Puts(buffer, NULL, 1);

    sprintf(buffer, "LIGHT %s", light);
    SSD1306_GotoXY(0,32);
    SSD1306_Puts(buffer, NULL, 1);

    SSD1306_UpdateScreen();

    HAL_Delay(1000);
  }
}

/* ===== CLOCK CONFIG ===== */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;

  HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitTypeDef clk = {0};
  clk.ClockType = RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK |
                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;

  clk.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clk.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clk.APB1CLKDivider = RCC_HCLK_DIV2;
  clk.APB2CLKDivider = RCC_HCLK_DIV1;

  HAL_RCC_ClockConfig(&clk, FLASH_LATENCY_2);
}

void Error_Handler(void)
{
  while(1)
  {
  }
}
