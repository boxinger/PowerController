#ifndef SCHEDULER_PORT_H
#define SCHEDULER_PORT_H

#include "tim.h"

static void Scheduler_Port_Init(void){
    HAL_TIM_Base_Start_IT(&htim6);
}


#endif