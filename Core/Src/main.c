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
#include "adc.h"
#include "dma.h"
#include "hrtim.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "encoder.h"
#include "oledgfx.h"
#include "buttonAndLED.h"
#include "scheduler.h"
#include "sample.h"
#include "PWM.h"

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

/* USER CODE BEGIN PV */

static uint8_t s_oledInitOk = 0U;
static uint8_t s_sampleInitOk = 0U;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

static void OLED_DrawAdcFrame(uint32_t frameIndex);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static void OLED_DrawAdcLine(uint16_t y, const char* label, Sample_ChannelTypeDef channel)
{
  OLEDGFX_ShowString(OLEDGFX_COL_1, y, label, OLEDGFX_Clip);
  OLEDGFX_ShowFloat(OLEDGFX_COL_7, y, Sample_GetPinVoltage(channel), 1U, 3U);
  OLEDGFX_ShowString(OLEDGFX_COL_12, y, "V", OLEDGFX_Clip);
}

static void OLED_DrawAdcFrame(uint32_t frameIndex)
{
  (void)OLEDGFX_Clear();

  if (s_sampleInitOk == 0U)
  {
    OLEDGFX_ShowString(OLEDGFX_COL_1, OLEDGFX_LINE_1, "ADC init error", OLEDGFX_Clip);
    (void)OLEDGFX_Submit();
    return;
  }

  if ((frameIndex & 1U) == 0U)
  {
    OLED_DrawAdcLine(OLEDGFX_LINE_1, "CH1", Sample_CH1);
    OLED_DrawAdcLine(OLEDGFX_LINE_2, "CH2", Sample_CH2);
    OLED_DrawAdcLine(OLEDGFX_LINE_3, "CH3", Sample_CH3);
    OLED_DrawAdcLine(OLEDGFX_LINE_4, "CH7", Sample_CH7);
  }
  else
  {
    OLED_DrawAdcLine(OLEDGFX_LINE_1, "CH8", Sample_CH8);
    OLED_DrawAdcLine(OLEDGFX_LINE_2, "CH9", Sample_CH9);
    OLED_DrawAdcLine(OLEDGFX_LINE_3, "CH12", Sample_CH12);
    OLED_DrawAdcLine(OLEDGFX_LINE_4, "CH15", Sample_CH15);
  }

  (void)OLEDGFX_Submit();
}

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
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_HRTIM1_Init();
  MX_I2C1_Init();
  MX_TIM3_Init();
  MX_USART1_UART_Init();
  MX_TIM6_Init();
  MX_ADC2_Init();
  MX_ADC3_Init();
  /* USER CODE BEGIN 2 */

  Encoder_Init();
  Encoder_Clear();

  Scheduler_Init();

  LED_SetLED1State(0U);
  LED_SetLED2State(0U);
  LED_SetLED3State(0U);
  LED_SetLED4State(0U);

  if (Sample_Init() == SAMPLE_OK)
  {
    s_sampleInitOk = 1U;
    (void)PWM_MasterStart();
  }

  if (OLEDGFX_Init() == OLEDGFX_OK)
  {
    s_oledInitOk = 1U;
    OLED_DrawAdcFrame(0U);
  }


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    static uint32_t lastBlinkTick = 0U;
    static uint32_t frameIndex = 0U;
    uint32_t now;

    now = HAL_GetTick();

    if ((now - lastBlinkTick) >= 500U)
    {
      lastBlinkTick = now;
      HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_0);
    }

    if (Scheduler_OLEDUpdateState == Scheduler_Pending)
    {
      Scheduler_ClearOLEDUpdatePending();
      if (s_oledInitOk != 0U)
      {
        // OLED_DrawAdcFrame(frameIndex++);
        OLED_DrawAdcFrame(0);
      }
    }

    if (Scheduler_Button1State == Scheduler_Pending)
    {
      Scheduler_ClearButton1Pending();
      HAL_GPIO_TogglePin(LED1_GPIO_Port, LED1_Pin);
    }

    if (Scheduler_Button2State == Scheduler_Pending)
    {
      Scheduler_ClearButton2Pending();
      HAL_GPIO_TogglePin(LED2_GPIO_Port, LED2_Pin);
    }

    if (Scheduler_Button3State == Scheduler_Pending)
    {
      Scheduler_ClearButton3Pending();
      HAL_GPIO_TogglePin(LED3_GPIO_Port, LED3_Pin);
    }

    if (Scheduler_Button4State == Scheduler_Pending)
    {
      Scheduler_ClearButton4Pending();
      HAL_GPIO_TogglePin(LED4_GPIO_Port, LED4_Pin);
    }

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

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV4;
  RCC_OscInitStruct.PLL.PLLN = 85;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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
