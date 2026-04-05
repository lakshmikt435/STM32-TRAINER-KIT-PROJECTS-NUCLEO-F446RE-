#include "main.h"

/* Function declarations */
void SystemClock_Config(void);
void Error_Handler(void);
void MX_GPIO_Init(void);

/* PINS */
#define TRIG_PORT GPIOB
#define TRIG_PIN  GPIO_PIN_5

#define ECHO_PORT GPIOA
#define ECHO_PIN  GPIO_PIN_8

#define BUZZER_PORT GPIOA
#define BUZZER_PIN  GPIO_PIN_7

/* MICRO DELAY */
void delay_us(uint16_t us)
{
  uint32_t start = DWT->CYCCNT;
  uint32_t ticks = us * (HAL_RCC_GetHCLKFreq()/1000000);
  while ((DWT->CYCCNT - start) < ticks);
}

/* SINGLE DISTANCE */
float read_distance()
{
  uint32_t timeout = 30000;

  HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);
  delay_us(2);

  HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_SET);
  delay_us(10);
  HAL_GPIO_WritePin(TRIG_PORT, TRIG_PIN, GPIO_PIN_RESET);

  while(!HAL_GPIO_ReadPin(ECHO_PORT, ECHO_PIN))
    if(timeout-- == 0) return 100;

  uint32_t start = DWT->CYCCNT;

  timeout = 30000;
  while(HAL_GPIO_ReadPin(ECHO_PORT, ECHO_PIN))
    if(timeout-- == 0) break;

  uint32_t end = DWT->CYCCNT;

  float time = (end - start) / (float)(HAL_RCC_GetHCLKFreq()/1000000);
  float distance = (time * 0.0343) / 2.0;

  if(distance < 2 || distance > 400)
    return 100;

  return distance;
}

/* AVERAGED DISTANCE */
float get_distance()
{
  float sum = 0;

  for(int i=0; i<5; i++)
  {
    sum += read_distance();
    HAL_Delay(10);
  }

  return sum / 5.0;
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

  float distance;

  while (1)
  {
    distance = get_distance();

    /* ===== SMOOTH CONTROL ===== */

    if (distance > 20)
    {
      /* OFF */
      HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
      HAL_Delay(100);
    }
    else if (distance > 5)
    {
      /* MAP DISTANCE TO DELAY */
      uint32_t delay_time = (distance * 20);  // dynamic

      HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
      HAL_Delay(delay_time);

      HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
      HAL_Delay(delay_time);
    }
    else
    {
      /* VERY CLOSE → CONTINUOUS */
      HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
    }
  }
}

/* CLOCK CONFIG */
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
  while(1);
}
