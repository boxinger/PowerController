#include "hrtim.h"
#include "i2c.h"
#include "oledgfx.h"
#include "scheduler.h"
#include "tim.h"

void App_HRTIM_RepetitionEventCallback(HRTIM_HandleTypeDef *hhrtim,
                                       uint32_t TimerIdx);

void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  if (hi2c->Instance == I2C1)
  {
    OLEDGFX_TxCpltCallback();
  }
}

void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
  if (hi2c->Instance == I2C1)
  {
    OLEDGFX_TxCpltCallback();
  }
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
  if (hi2c->Instance == I2C1)
  {
    OLEDGFX_ErrorCallback();
  }
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  Scheduler_TIMPeriodElapsedCallback(htim);
}

void HAL_HRTIM_RepetitionEventCallback(HRTIM_HandleTypeDef *hhrtim,
                                       uint32_t TimerIdx)
{
  App_HRTIM_RepetitionEventCallback(hhrtim, TimerIdx);
}
