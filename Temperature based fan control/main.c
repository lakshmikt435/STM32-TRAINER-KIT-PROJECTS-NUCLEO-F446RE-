#include "main.h"

/* Function declarations */
void SystemClock_Config(void);
void Error_Handler(void);
void MX_GPIO_Init(void);

/* ===== DHT PIN ===== */
#define DHT_PORT GPIOA
#define DHT_PIN  GPIO_PIN_9

/* ===== DELAY ===== */
void delay_us(uint16_t us)
{
  uint32_t start = DWT->CYCCNT;
  uint32_t ticks = us * (HAL_RCC_GetHCLKFreq() / 1000000);
  while ((DWT->CYCCNT - start) < ticks);
}

/* ===== DHT READ ===== */
uint8_t DHT_Read(void)
{
  uint8_t data = 0;

  for(int i = 0; i < 8; i++)
  {
    while(!HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN));
    delay_us(40);

    if(HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN))
      data |= (1 << (7 - i));

    while(HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN));
  }

  return data;
}

/* ===== DHT START ===== */
void DHT_Start(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* Output */
  GPIO_InitStruct.Pin = DHT_PIN;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(DHT_PORT, &GPIO_InitStruct);

  HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_RESET);
  HAL_Delay(18);

  HAL_GPIO_WritePin(DHT_PORT, DHT_PIN, GPIO_PIN_SET);
  delay_us(30);

  /* Input */
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(DHT_PORT, &GPIO_InitStruct);
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();

  /* Enable DWT */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

  uint8_t temp;

  while (1)
  {
    DHT_Start();

    /* Response */
    while(HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN));
    while(!HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN));
    while(HAL_GPIO_ReadPin(DHT_PORT, DHT_PIN));

    /* Read data */
    DHT_Read();
    DHT_Read();
    temp = DHT_Read();
    DHT_Read();

    /* ===== FAN CONTROL ===== */
    if (temp > 30 && temp < 60)   // valid + hot
    {
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET); // FAN ON
    }
    else
    {
      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET); // FAN OFF
    }

    HAL_Delay(2000);
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

  RCC_ClkInitTypeDef RCC_ClkInitStruct2 = {0};
  RCC_ClkInitStruct2.ClockType = RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_HCLK |
                                RCC_CLOCKTYPE_PCLK1 |
                                RCC_CLOCKTYPE_PCLK2;

  RCC_ClkInitStruct2.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct2.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct2.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct2.APB2CLKDivider = RCC_HCLK_DIV1;

  HAL_RCC_ClockConfig(&RCC_ClkInitStruct2, FLASH_LATENCY_2);
}

void Error_Handler(void)
{
  while(1);
}
