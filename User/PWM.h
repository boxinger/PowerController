#ifndef __PWM_H
#define __PWM_H

#include <stdint.h>
#include "main.h"
#include "hrtim.h"

#define PWM_TIMER hhrtim1
#define PWM_MINDUTY 0.05f
#define PWM_MAXDUTY 0.95f
#define PWM_TIMER_PRESCALER HRTIM_PRESCALERRATIO_MUL32
#define PWM_TIMER_PERIOD 27200U
#define PWM_DEFAULT_CMPARE 13600U
#define PWM_DEADTIME_PRESCALER HRTIM_TIMDEADTIME_PRESCALERRATIO_MUL8
#define PWM_DEADTIME_RISING_VALUE 10U
#define PWM_DEADTIME_FALLING_VALUE 10U

/*
	* @param  Timer PWM timer enum
	*                   This parameter can be one of the following values:
	*                   @arg PWM_TIMERA
	*                   @arg PWM_TIMERB
	*                   @arg PWM_TIMERC
	*                   @arg PWM_TIMERD
	*                   @arg PWM_TIMERE
	*                   @arg PWM_TIMERF
	* @note   PWM_Init only configures and starts the requested timer PWM template.
	*         HRTIM common initialization and GPIO alternate function configuration
	*         must still be provided by external board-level code.
*/

typedef enum {
	PWM_TIMERA = 0U,
	PWM_TIMERB = 1U,
	PWM_TIMERC = 2U,
	PWM_TIMERD = 3U,
	PWM_TIMERE = 4U,
	PWM_TIMERF = 5U,
	PWM_TIMER_COUNT
} PWM_TimerIdTypeDef;

HAL_StatusTypeDef PWM_Init(PWM_TimerIdTypeDef Timer);
HAL_StatusTypeDef PWM_DeInit(uint32_t Timer);

static inline void PWM_Start_CHA(){
	HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA1);
	HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TA2);
}
static inline void PWM_Stop_CHA(){
	HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TA1);
	HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TA2);
}
static inline void PWM_Start_CHB(){
	HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TB1);
	HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TB2);
}
static inline void PWM_Stop_CHB(){
	HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TB1);
	HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TB2);
}
static inline void PWM_Start_CHC(){
	HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TC1);
	HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TC2);
}
static inline void PWM_Stop_CHC(){
	HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TC1);
	HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TC2);
}
static inline void PWM_Start_CHD(){
	HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TD1);
	HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TD2);
}
static inline void PWM_Stop_CHD(){
	HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TD1);
	HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TD2);
}
static inline void PWM_Start_CHE(){
	HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TE1);
	HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TE2);
}
static inline void PWM_Stop_CHE(){
	HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TE1);
	HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TE2);
}
static inline void PWM_Start_CHF(){
	HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TF1);
	HAL_HRTIM_WaveformOutputStart(&hhrtim1, HRTIM_OUTPUT_TF2);
}
static inline void PWM_Stop_CHF(){
	HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TF1);
	HAL_HRTIM_WaveformOutputStop(&hhrtim1, HRTIM_OUTPUT_TF2);
}

/*
  * @param  TimerIdx Timer index
  *                   This parameter can be one of the following values:
  *                   @arg HRTIM_TIMERINDEX_TIMER_A for timer A
  *                   @arg HRTIM_TIMERINDEX_TIMER_B for timer B
  *                   @arg HRTIM_TIMERINDEX_TIMER_C for timer C
  *                   @arg HRTIM_TIMERINDEX_TIMER_D for timer D
  *                   @arg HRTIM_TIMERINDEX_TIMER_E for timer E
  *                   @arg HRTIM_TIMERINDEX_TIMER_F for timer F
*/
static inline void PWM_SetCompareRaw(uint32_t TimerIdx, uint16_t compare){
	HRTIM1 -> sTimerxRegs[TimerIdx].CMP1xR = compare;
}
static inline void PWM_SetDutyCycle(uint32_t TimerIdx, float dutyCycle){
	if (dutyCycle < PWM_MINDUTY) {
		dutyCycle = PWM_MINDUTY;
	} else if (dutyCycle > PWM_MAXDUTY) {
		dutyCycle = PWM_MAXDUTY;
	}

	PWM_SetCompareRaw(TimerIdx, (uint16_t)(dutyCycle * (__HAL_HRTIM_GETPERIOD(&hhrtim1, TimerIdx) + 1)));
}

#endif
