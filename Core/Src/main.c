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
#include "voloop.h"
#include "PWM.h"
#include "buttonAndLED.h"
#include "sample.h"
#include "scheduler.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

#ifndef BUCK_SLIDING_FILTER_DEPTH
#define BUCK_SLIDING_FILTER_DEPTH 16U
#endif

#if (BUCK_SLIDING_FILTER_DEPTH == 0U)
#error "BUCK_SLIDING_FILTER_DEPTH must be greater than 0."
#endif

typedef struct
{
  float samples[BUCK_SLIDING_FILTER_DEPTH];
  float sum;
  uint16_t index;
  uint16_t count;
} Buck_MovingAverageFilterTypeDef;

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define BUCK_CONTROL_DIVIDER 5U
#define BUCK_CONTROL_FREQUENCY_HZ 10000U
#define BUCK_TARGET_OUTPUT_VOLTAGE 10.0f
#define BUCK_TARGET_CURRENT_LIMIT 5.0f

#define BUCK_VIN_CHANNEL Sample_CH1
#define BUCK_IL_CHANNEL Sample_CH3
#define BUCK_VOUT_CHANNEL Sample_CH4

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

static uint8_t s_oledInitOk = 0U;
static SAMPLE_StatusTypeDef s_sampleStatus = SAMPLE_BUSY;
static Buck_HandleTypeDef s_buckHandle = {0};
static volatile uint8_t s_buckControlReady = 0U;
static volatile uint8_t s_buckDividerCounter = 0U;
static volatile VOLOOP_StatusTypeDef s_buckLastStatus = VOLOOP_INVALID_STATE;
static volatile Buck_StateTypeDef s_buckStateShadow = BUCK_DISABLED;
static volatile Buck_FaultCodeTypeDef s_buckFaultShadow = BUCK_NOERROR;
static Buck_MovingAverageFilterTypeDef s_buckOutputVoltageFilter = {0};
static Buck_MovingAverageFilterTypeDef s_buckInductorCurrentFilter = {0};
static volatile float s_buckFilteredOutputVoltage = 0.0f;
static volatile float s_buckFilteredInductorCurrent = 0.0f;
static volatile uint8_t s_buckFilterReady = 0U;

static const PID_InitTypeDef s_buckVoltageLoopPidInit = {
  .mode = PID_OneZero,
  .init.OneZero = {
    .gain = 0.08f,
    .zero = 20.0f,
    .triggerFrequency = BUCK_CONTROL_FREQUENCY_HZ,
  },
};

static const PID_InitTypeDef s_buckCurrentLoopPidInit = {
  .mode = PID_OneZero,
  .init.OneZero = {
    .gain = 0.20f,
    .zero = 200.0f,
    .triggerFrequency = BUCK_CONTROL_FREQUENCY_HZ,
  },
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

static float Buck_GetInputVoltage(void);
static float Buck_GetOutputVoltage(void);
static float Buck_GetInductorCurrent(void);
static float Buck_ReadOutputVoltageSample(void);
static float Buck_ReadInductorCurrentSample(void);
static void Buck_InitHardware(void);
static void Buck_DeInitHardware(void);
static void Buck_StartHardware(void);
static void Buck_StopHardware(void);     
static void Buck_SetDuty(float duty);
static const char* Buck_GetStatusText(void);
static float Buck_ClampNonNegative(float value);
static void Buck_ResetMovingAverage(Buck_MovingAverageFilterTypeDef* filter);
static void Buck_PrimeMovingAverage(Buck_MovingAverageFilterTypeDef* filter, float seed);
static float Buck_UpdateMovingAverage(Buck_MovingAverageFilterTypeDef* filter, float sample);
static void Buck_PrimeMeasurementFilters(void);
static void Buck_UpdateFilteredMeasurements(void);
static void Buck_UpdateStateShadow(void);
static VOLOOP_StatusTypeDef Buck_InitControl(void);
static void OLED_DrawDebugAnimation(uint32_t frameIndex);
static void OLED_DrawSampleFrame(uint32_t frameIndex);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static float Buck_ClampNonNegative(float value)
{
  if (value < 0.0f)
  {
    return 0.0f;
  }

  return value;
}

static float Buck_GetInputVoltage(void)
{
  return 20.0f;
}

static float Buck_GetOutputVoltage(void)
{
  if (s_buckFilterReady != 0U)
  {
    return s_buckFilteredOutputVoltage;
  }

  return Buck_ReadOutputVoltageSample();
}

static float Buck_GetInductorCurrent(void)
{
  if (s_buckFilterReady != 0U)
  {
    return s_buckFilteredInductorCurrent;
  }

  return Buck_ReadInductorCurrentSample();
}

static float Buck_ReadOutputVoltageSample(void)
{
  return Buck_ClampNonNegative(Sample_GetTheoreticalVoltage(BUCK_VOUT_CHANNEL) - 0.4f);
}

static float Buck_ReadInductorCurrentSample(void)
{
  return Buck_ClampNonNegative(Sample_GetTheoreticalCurrent(BUCK_IL_CHANNEL));
}

static void Buck_ResetMovingAverage(Buck_MovingAverageFilterTypeDef* filter)
{
  uint16_t sampleIndex;

  if (filter == NULL)
  {
    return;
  }

  filter->sum = 0.0f;
  filter->index = 0U;
  filter->count = 0U;

  for (sampleIndex = 0U; sampleIndex < BUCK_SLIDING_FILTER_DEPTH; sampleIndex++)
  {
    filter->samples[sampleIndex] = 0.0f;
  }
}

static void Buck_PrimeMovingAverage(Buck_MovingAverageFilterTypeDef* filter, float seed)
{
  uint16_t sampleIndex;

  if (filter == NULL)
  {
    return;
  }

  for (sampleIndex = 0U; sampleIndex < BUCK_SLIDING_FILTER_DEPTH; sampleIndex++)
  {
    filter->samples[sampleIndex] = seed;
  }

  filter->sum = seed * (float)BUCK_SLIDING_FILTER_DEPTH;
  filter->index = 0U;
  filter->count = BUCK_SLIDING_FILTER_DEPTH;
}

static float Buck_UpdateMovingAverage(Buck_MovingAverageFilterTypeDef* filter, float sample)
{
  float filteredValue;

  if (filter == NULL)
  {
    return sample;
  }

  if (filter->count >= BUCK_SLIDING_FILTER_DEPTH)
  {
    filter->sum -= filter->samples[filter->index];
  }
  else
  {
    filter->count++;
  }

  filter->samples[filter->index] = sample;
  filter->sum += sample;
  filter->index++;
  if (filter->index >= BUCK_SLIDING_FILTER_DEPTH)
  {
    filter->index = 0U;
  }

  filteredValue = filter->sum / (float)filter->count;
  return filteredValue;
}

static void Buck_PrimeMeasurementFilters(void)
{
  float outputVoltage;
  float inductorCurrent;

  outputVoltage = Buck_ReadOutputVoltageSample();
  inductorCurrent = Buck_ReadInductorCurrentSample();

  Buck_ResetMovingAverage(&s_buckOutputVoltageFilter);
  Buck_ResetMovingAverage(&s_buckInductorCurrentFilter);
  Buck_PrimeMovingAverage(&s_buckOutputVoltageFilter, outputVoltage);
  Buck_PrimeMovingAverage(&s_buckInductorCurrentFilter, inductorCurrent);

  s_buckFilteredOutputVoltage = outputVoltage;
  s_buckFilteredInductorCurrent = inductorCurrent;
  s_buckFilterReady = 1U;
}

static void Buck_UpdateFilteredMeasurements(void)
{
  float outputVoltage;
  float inductorCurrent;

  if (s_buckFilterReady == 0U)
  {
    Buck_PrimeMeasurementFilters();
    return;
  }

  outputVoltage = Buck_ReadOutputVoltageSample();
  inductorCurrent = Buck_ReadInductorCurrentSample();

  s_buckFilteredOutputVoltage = Buck_UpdateMovingAverage(&s_buckOutputVoltageFilter, outputVoltage);
  s_buckFilteredInductorCurrent = Buck_UpdateMovingAverage(&s_buckInductorCurrentFilter, inductorCurrent);
}

static void Buck_InitHardware(void)
{
  Buck_PrimeMeasurementFilters();
}

static void Buck_DeInitHardware(void)
{
  PWM_Stop(PWM_TIMERA);
  PWM_SetDutyCycle(PWM_TIMERA, BUCK_MIN_DUTY);
}

static void Buck_StartHardware(void)
{
  PWM_Start(PWM_TIMERA);
}

static void Buck_StopHardware(void)
{
  PWM_Stop(PWM_TIMERA);
}

static void Buck_SetDuty(float duty)
{
  PWM_SetDutyCycle(PWM_TIMERA, duty);
}

static void Buck_UpdateStateShadow(void)
{
  if (s_buckControlReady == 0U)
  {
    s_buckStateShadow = BUCK_DISABLED;
    s_buckFaultShadow = BUCK_NOERROR;
    return;
  }

  s_buckStateShadow = VOLOOP_Buck_GetState(&s_buckHandle);
  s_buckFaultShadow = VOLOOP_Buck_GetFaultCode(&s_buckHandle);
}

static const char* Buck_GetStatusText(void)
{
  if (s_sampleStatus != SAMPLE_OK)
  {
    return "SAMP";
  }

  if (s_buckControlReady == 0U)
  {
    return "INIT";
  }

  if (s_buckFaultShadow == BUCK_OCP)
  {
    return "OCP";
  }

  if (s_buckFaultShadow == BUCK_OVP)
  {
    return "OVP";
  }

  switch (s_buckStateShadow)
  {
    case BUCK_CVMODE:
      return "CV";

    case BUCK_CCMODE:
      return "CC";

    case BUCK_DISABLED:
      return "OFF";

    case BUCK_ERROR:
    default:
      return "ERR";
  }
}

static VOLOOP_StatusTypeDef Buck_InitControl(void)
{
  Buck_InitTypeDef buckInit = {
    .InitFunc = Buck_InitHardware,
    .DeInitFunc = Buck_DeInitHardware,
    .Start = Buck_StartHardware,
    .Stop = Buck_StopHardware,
    .SetDuty = Buck_SetDuty,
    .GetInputVoltage = Buck_GetInputVoltage,
    .GetOutputVoltage = Buck_GetOutputVoltage,
    .GetInductorCurrent = Buck_GetInductorCurrent,
    .OutPutVoltagePIDInit = (PID_InitTypeDef*)&s_buckVoltageLoopPidInit,
    .InductorCurrentPIDInit = (PID_InitTypeDef*)&s_buckCurrentLoopPidInit,
  };
  VOLOOP_StatusTypeDef status;

  s_buckControlReady = 0U;

  status = VOLOOP_Buck_Init(&s_buckHandle, &buckInit);
  if (status != VOLOOP_OK)
  {
    return status;
  }

  status = VOLOOP_Buck_SetValue(&s_buckHandle, BUCK_TARGET_OUTPUT_VOLTAGE, BUCK_TARGET_CURRENT_LIMIT);
  if (status != VOLOOP_OK)
  {
    return status;
  }

  status = VOLOOP_Buck_Start(&s_buckHandle);
  if (status != VOLOOP_OK)
  {
    return status;
  }

  s_buckControlReady = 1U;
  Buck_UpdateStateShadow();
  return VOLOOP_OK;
}

void App_HRTIM_RepetitionEventCallback(HRTIM_HandleTypeDef* hhrtim,
                                       uint32_t TimerIdx)
{
  if ((hhrtim != &hhrtim1) || (TimerIdx != HRTIM_TIMERINDEX_MASTER))
  {
    return;
  }

  if (s_sampleStatus != SAMPLE_OK)
  {
    return;
  }

  Buck_UpdateFilteredMeasurements();

  if (s_buckControlReady == 0U)
  {
    return;
  }

  s_buckDividerCounter++;
  if (s_buckDividerCounter < BUCK_CONTROL_DIVIDER)
  {
    return;
  }

  s_buckDividerCounter = 0U;
  s_buckLastStatus = VOLOOP_Buck_Sync(&s_buckHandle);
  Buck_UpdateStateShadow();
}

static void OLED_DrawDebugAnimation(uint32_t frameIndex)
{
  const uint16_t baseX = 112U;
  const uint16_t baseY = 0U;
  const uint8_t size = 15U;
  uint8_t offset;
  uint8_t step;
  uint16_t dotX;
  uint16_t dotY;

  for (offset = 0U; offset <= size; offset++)
  {
    OLEDGFX_DrawPoint((uint16_t)(baseX + offset), baseY, 1U);
    OLEDGFX_DrawPoint((uint16_t)(baseX + offset), (uint16_t)(baseY + size), 1U);
    OLEDGFX_DrawPoint(baseX, (uint16_t)(baseY + offset), 1U);
    OLEDGFX_DrawPoint((uint16_t)(baseX + size), (uint16_t)(baseY + offset), 1U);
  }

  step = (uint8_t)(frameIndex % (uint32_t)((size - 1U) * 4U));
  if (step < (size - 1U))
  {
    dotX = (uint16_t)(baseX + 1U + step);
    dotY = (uint16_t)(baseY + 1U);
  }
  else if (step < (uint8_t)((size - 1U) * 2U))
  {
    dotX = (uint16_t)(baseX + size - 2U);
    dotY = (uint16_t)(baseY + 1U + (step - (size - 1U)));
  }
  else if (step < (uint8_t)((size - 1U) * 3U))
  {
    dotX = (uint16_t)(baseX + size - 2U - (step - (uint8_t)((size - 1U) * 2U)));
    dotY = (uint16_t)(baseY + size - 2U);
  }
  else
  {
    dotX = (uint16_t)(baseX + 1U);
    dotY = (uint16_t)(baseY + size - 2U - (step - (uint8_t)((size - 1U) * 3U)));
  }

  OLEDGFX_DrawPoint(dotX, dotY, 1U);
  OLEDGFX_DrawPoint((uint16_t)(dotX + 1U), dotY, 1U);
  OLEDGFX_DrawPoint(dotX, (uint16_t)(dotY + 1U), 1U);
  OLEDGFX_DrawPoint((uint16_t)(dotX + 1U), (uint16_t)(dotY + 1U), 1U);
}

static void OLED_DrawSampleFrame(uint32_t frameIndex)
{
  float inputVoltage;
  float inductorCurrent;
  float outputVoltage;

  (void)OLEDGFX_Clear();

  if (s_sampleStatus == SAMPLE_OK)
  {
    inputVoltage = Buck_GetInputVoltage();
    inductorCurrent = Buck_GetInductorCurrent();
    outputVoltage = Buck_GetOutputVoltage();

    OLEDGFX_ShowString(OLEDGFX_COL_1, OLEDGFX_LINE_1, "Vin:", OLEDGFX_Clip);
    OLEDGFX_ShowFloat(OLEDGFX_COL_6, OLEDGFX_LINE_1, inputVoltage, 2U, 2U);
    OLEDGFX_ShowString(OLEDGFX_COL_13, OLEDGFX_LINE_1, "V", OLEDGFX_Clip);

    OLEDGFX_ShowString(OLEDGFX_COL_1, OLEDGFX_LINE_2, "IL :", OLEDGFX_Clip);
    OLEDGFX_ShowFloat(OLEDGFX_COL_6, OLEDGFX_LINE_2, inductorCurrent, 1U, 2U);
    OLEDGFX_ShowString(OLEDGFX_COL_13, OLEDGFX_LINE_2, "A", OLEDGFX_Clip);

    OLEDGFX_ShowString(OLEDGFX_COL_1, OLEDGFX_LINE_3, "Vo :", OLEDGFX_Clip);
    OLEDGFX_ShowFloat(OLEDGFX_COL_6, OLEDGFX_LINE_3, outputVoltage, 2U, 2U);
    OLEDGFX_ShowString(OLEDGFX_COL_13, OLEDGFX_LINE_3, "V", OLEDGFX_Clip);

    OLEDGFX_ShowString(OLEDGFX_COL_1, OLEDGFX_LINE_4, Buck_GetStatusText(), OLEDGFX_Clip);
    OLEDGFX_ShowFloat(OLEDGFX_COL_5, OLEDGFX_LINE_4, BUCK_TARGET_OUTPUT_VOLTAGE, 2U, 1U);
    OLEDGFX_ShowString(OLEDGFX_COL_10, OLEDGFX_LINE_4, "V", OLEDGFX_Clip);
    OLEDGFX_ShowFloat(OLEDGFX_COL_12, OLEDGFX_LINE_4, BUCK_TARGET_CURRENT_LIMIT, 1U, 1U);
    OLEDGFX_ShowString(OLEDGFX_COL_16, OLEDGFX_LINE_4, "A", OLEDGFX_Clip);
  }
  else
  {
    OLEDGFX_ShowString(OLEDGFX_COL_1, OLEDGFX_LINE_1, "Sample error", OLEDGFX_Clip);
    OLEDGFX_ShowString(OLEDGFX_COL_1, OLEDGFX_LINE_2, "Code:", OLEDGFX_Clip);
    OLEDGFX_ShowNum(OLEDGFX_COL_7, OLEDGFX_LINE_2, (uint32_t)s_sampleStatus, 1U);
    OLEDGFX_ShowString(OLEDGFX_COL_1, OLEDGFX_LINE_4, "Buck off", OLEDGFX_Clip);
  }

  OLED_DrawDebugAnimation(frameIndex);

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

  if (PWM_Init(PWM_TIMERA) != HAL_OK)
  {
    Error_Handler();
  }
  PWM_Init(PWM_TIMERB);
  PWM_SetDutyCycle(PWM_TIMERB, 0.5f);
  PWM_Start(PWM_TIMERB);
  PWM_SetDutyCycle(PWM_TIMERA, BUCK_MIN_DUTY);
  s_sampleStatus = Sample_Init();
  if (s_sampleStatus == SAMPLE_OK)
  {
    s_buckLastStatus = Buck_InitControl();
    if (s_buckLastStatus != VOLOOP_OK)
    {
      Buck_UpdateStateShadow();
    }
  }
  Scheduler_Init();

  if (HAL_TIM_Base_Start_IT(&htim6) != HAL_OK)
  {
    Error_Handler();
  }

  LED_SetLED1State(0U);
  LED_SetLED2State(0U);
  LED_SetLED3State(0U);
  LED_SetLED4State(0U);

  if (OLEDGFX_Init() == OLEDGFX_OK)
  {
    OLED_DrawSampleFrame(0U);
    s_oledInitOk = 1U;
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
        OLED_DrawSampleFrame(frameIndex++);
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
