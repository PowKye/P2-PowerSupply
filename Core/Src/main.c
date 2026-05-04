/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
volatile uint32_t adc_value = 0;
volatile uint8_t flag_log_adc = 0;
volatile uint32_t adc_accumulator = 0;
volatile uint8_t adc_sample_count = 0;
volatile uint32_t adc_avg = 0;
volatile uint16_t adc_target = (1.21 * 4095) / (3.3); // 3V output
volatile uint8_t dac_output = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

uint8_t CycleRGBLED(int numCycles, int delayMs);
void LogGPIOState(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin);
void LogStartupMessage(void);
void WritePortByte(GPIO_TypeDef *GPIOx, uint8_t isHighByte, uint8_t value);
void Task_ReadADC(void);
void Task_LogADC(void);
void Task_DigitalStabilizer(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_I2C1_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

  // Start 10ms time base
  HAL_TIM_Base_Start_IT(&htim2);

  // Start encoder hardware reading
  HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);

  LogStartupMessage();

  // Cycle through RGB colors when the system boots
  while (!CycleRGBLED(1, 300))
  {
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    // Loggs ADC readings once 100ms
    if (flag_log_adc == 1)
    {
      Task_LogADC();
      flag_log_adc = 0; // Reset the flag after sending the message
    }

    Task_DigitalStabilizer();

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
   */
  hadc1.Instance = ADC1;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
   */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_71CYCLES_5;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */
}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */
}

/**
 * @brief TIM2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7199;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 100;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_OC_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_TIMING;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */
  HAL_TIM_MspPostInit(&htim2);
}

/**
 * @brief TIM3 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_Encoder_InitTypeDef sConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
  sConfig.IC1Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC1Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC1Filter = 0;
  sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
  sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
  sConfig.IC2Prescaler = TIM_ICPSC_DIV1;
  sConfig.IC2Filter = 0;
  if (HAL_TIM_Encoder_Init(&htim3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */
}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, R_LED_Pin | G_LED_Pin | Vb2_Pin | Vb3_Pin | Vb4_Pin | Vb5_Pin | Vb6_Pin | Vb7_Pin | Vb0_Pin | Vb1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(B_LED_GPIO_Port, B_LED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : ENC_SW_Pin */
  GPIO_InitStruct.Pin = ENC_SW_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(ENC_SW_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : R_LED_Pin G_LED_Pin */
  GPIO_InitStruct.Pin = R_LED_Pin | G_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : Vb2_Pin Vb3_Pin Vb4_Pin Vb5_Pin
                           Vb6_Pin Vb7_Pin Vb0_Pin Vb1_Pin */
  GPIO_InitStruct.Pin = Vb2_Pin | Vb3_Pin | Vb4_Pin | Vb5_Pin | Vb6_Pin | Vb7_Pin | Vb0_Pin | Vb1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : B_LED_Pin */
  GPIO_InitStruct.Pin = B_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(B_LED_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/// @brief Loggs startup message through UART1
/// @param
void LogStartupMessage(void)
{
  char *msg = "P2 v.1 running\r\n";

  HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}

/// @brief  Cycles through R-G-B sequence on the built in RGB LED on the board
/// @param numCycles
/// @param delayMs
uint8_t CycleRGBLED(int numCycles, int delayMs)
{
  static int currentCycle = 0;
  static int state = 0;
  static uint32_t lastTick = 0;
  static uint8_t isRunning = 0;

  if (currentCycle >= numCycles)
  {
    // Reset state for future calls
    currentCycle = 0;
    state = 0;
    isRunning = 0;
    return 1; // Completed
  }

  // Force immediate execution on first call
  if (!isRunning)
  {
    lastTick = HAL_GetTick() - delayMs;
    isRunning = 1;
  }

  if ((HAL_GetTick() - lastTick) >= delayMs)
  {
    lastTick = HAL_GetTick();

    switch (state)
    {
    case 0:
      HAL_GPIO_WritePin(GPIOA, B_LED_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOB, R_LED_Pin, GPIO_PIN_SET);
      state = 1;
      break;
    case 1:
      HAL_GPIO_WritePin(GPIOB, R_LED_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOB, G_LED_Pin, GPIO_PIN_SET);
      state = 2;
      break;
    case 2:
      HAL_GPIO_WritePin(GPIOB, G_LED_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOA, B_LED_Pin, GPIO_PIN_SET);
      state = 3;
      break;
    case 3:
      HAL_GPIO_WritePin(GPIOA, B_LED_Pin, GPIO_PIN_RESET);
      currentCycle++;
      if (currentCycle < numCycles)
      {
        HAL_GPIO_WritePin(GPIOB, R_LED_Pin, GPIO_PIN_SET);
        state = 1;
      }
      break;
    }
  }

  return 0; // Still in progress
}

/// @brief Loggs State of GPIOx_Pin through UART1
/// @param GPIOx
/// @param GPIO_Pin
void LogGPIOState(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
  GPIO_PinState state = HAL_GPIO_ReadPin(GPIOx, GPIO_Pin);
  char *port_name;
  if (GPIOx == GPIOA)
    port_name = "GPIOA";
  else if (GPIOx == GPIOB)
    port_name = "GPIOB";
  else if (GPIOx == GPIOC)
    port_name = "GPIOC";
  else
    port_name = "UNKNOWN";
  int pin_num = 0;
  uint16_t temp = GPIO_Pin;
  while (temp > 1)
  {
    temp >>= 1;
    pin_num++;
  }
  char msg[50];
  if (state == GPIO_PIN_SET)
  {
    sprintf(msg, "%s Pin %d is ACTIVE (High)\r\n", port_name, pin_num);
  }
  else
  {
    sprintf(msg, "%s Pin %d is INACTIVE (Low)\r\n", port_name, pin_num);
  }
  HAL_UART_Transmit(&huart1, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
}

/// @brief Atomically writes an 8-bit value to either the high or low byte of a given GPIO port
/// @param GPIOx Port to write to (e.g., GPIOB)
/// @param isHighByte 1 to target pins 8-15, 0 to target pins 0-7
/// @param value 8-bit value to write
void WritePortByte(GPIO_TypeDef *GPIOx, uint8_t isHighByte, uint8_t value)
{
  if (isHighByte)
  {
    // Target pins 8-15: Set mask shifted by 8, Reset mask shifted by 24
    GPIOx->BSRR = ((uint32_t)value << 8) | ((uint32_t)(~value & 0xFF) << 24);
  }
  else
  {
    // Target pins 0-7: Set mask shifted by 0, Reset mask shifted by 16
    GPIOx->BSRR = (uint32_t)value | ((uint32_t)(~value & 0xFF) << 16);
  }
}

/// @brief Reads ADC value and filters it -> every raw read counts as 12.5% of the filtered read
/// @param
void Task_ReadADC(void)
{
  HAL_ADC_Start(&hadc1);
  if (HAL_ADC_PollForConversion(&hadc1, 1) == HAL_OK)
  {
    uint32_t adc_raw_value = HAL_ADC_GetValue(&hadc1);
    adc_value = (adc_value * 7 + adc_raw_value + 4) / 8;

    // Accumulate data for the digital stabilizer
    adc_accumulator += adc_value;
    adc_sample_count++;
  }
}

void Task_DigitalStabilizer(void)
{
  // Process only if 10 readings have been accumulated (100ms at 10ms rate)
  if (adc_sample_count >= 10)
  {
    // Copy and reset data atomically to prevent race conditions with the interrupt
    __disable_irq();
    uint32_t acc_copy = adc_accumulator;
    uint8_t count_copy = adc_sample_count;
    adc_accumulator = 0;
    adc_sample_count = 0;
    __enable_irq();

    // Compute the average of the accumulated ADC samples
    adc_avg = acc_copy / count_copy;

    // Slow comparator
    if (adc_avg < adc_target)
    {
      if (dac_output < 255)
      {
        dac_output++;
      }
    }
    else if (adc_avg > adc_target)
    {
      if (dac_output > 0)
      {
        dac_output--;
      }
    }

    WritePortByte(GPIOB, 1, dac_output);
  }
}

/// @brief
/// @param
void Task_LogADC(void)
{
  char msg[80];
  uint32_t voltage_x100 = (adc_value * 330 + 2047) / 4095; //*990
  uint32_t v_int = voltage_x100 / 100;
  uint32_t v_frac = voltage_x100 % 100;
  // DEBUG: Added dac_output to the log message temporarily
  int len = sprintf(msg, "ADC_Raw: %lu | V_Out: %lu.%02luV | DAC: %u\r\n", adc_value, v_int, v_frac, dac_output);

  HAL_UART_Transmit(&huart1, (uint8_t *)msg, len, 100);
}

/// @brief
/// @param htim
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2)
  {
    Task_ReadADC();

    // Tick divider from 10ms to 1000ms
    static uint8_t ticks = 0;
    ticks++;
    if (ticks >= 100) // 100 ticks * 10 ms = 1 second
    {
      flag_log_adc = 1; // Send the flag to the main function
      ticks = 0;
    }
  }
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
