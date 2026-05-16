#ifndef __PWM_H
#define __PWM_H

#include <stdint.h>
#include "main.h"
#include "hrtim.h"

#define PWM_TIMER hhrtim1
#define PWM_MINDUTY 0.05f
#define PWM_MAXDUTY 0.95f

/*
  * @param  Timers Timer counter(s)
  *                   This parameter can be any combination of the following values:
  *                   @arg HRTIM_TIMERID_MASTER
  *                   @arg HRTIM_TIMERID_TIMER_A
  *                   @arg HRTIM_TIMERID_TIMER_B
  *                   @arg HRTIM_TIMERID_TIMER_C
  *                   @arg HRTIM_TIMERID_TIMER_D
  *                   @arg HRTIM_TIMERID_TIMER_E
  *                   @arg HRTIM_TIMERID_TIMER_F
*/
static inline void PWM_Init(uint32_t Timer){
	HAL_HRTIM_WaveformCounterStart(&PWM_TIMER, Timer);
}
static inline void PWM_DeInit(uint32_t Timer){
	HAL_HRTIM_WaveformCounterStop(&PWM_TIMER, Timer);
}

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
