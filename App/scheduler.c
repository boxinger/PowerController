#include "scheduler.h"
#include <stdint.h>

#define Scheduler_ButtonScaler 50U
static uint16_t s_buttonCounter = 0U;
volatile Scheduler_PendingStatusTypeDef Scheduler_Button1State = Scheduler_Unpending;
volatile Scheduler_PendingStatusTypeDef Scheduler_Button2State = Scheduler_Unpending;
volatile Scheduler_PendingStatusTypeDef Scheduler_Button3State = Scheduler_Unpending;
volatile Scheduler_PendingStatusTypeDef Scheduler_Button4State = Scheduler_Unpending;
volatile Scheduler_PendingStatusTypeDef Scheduler_EncoderButtonState = Scheduler_Unpending;

void Scheduler_Init(void){
}

__weak void Scheduler_Button1Callback(void){}
__weak void Scheduler_Button2Callback(void){}
__weak void Scheduler_Button3Callback(void){}
__weak void Scheduler_Button4Callback(void){}
__weak void Scheduler_EncoderButtonCallback(void){}
    

static void s_buttonUpdate(void){
    static uint8_t lastButton1State = 0U;
    static uint8_t lastButton2State = 0U;
    static uint8_t lastButton3State = 0U;
    static uint8_t lastButton4State = 0U;
    static uint8_t lastEncoderButtonState = 0U;
    uint8_t currentButton1State = Button_GetButton1State();
    uint8_t currentButton2State = Button_GetButton2State();
    uint8_t currentButton3State = Button_GetButton3State();
    uint8_t currentButton4State = Button_GetButton4State();
    uint8_t currentEncoderButtonState = Button_GetEncoderButtonState();

    if(currentButton1State == 1U && lastButton1State == 0U){
        Scheduler_Button1State = Scheduler_Pending;
        Scheduler_Button1Callback();
    }
    if(currentButton2State == 1U && lastButton2State == 0U){
        Scheduler_Button2State = Scheduler_Pending;
        Scheduler_Button2Callback();
    }
    if(currentButton3State == 1U && lastButton3State == 0U){
        Scheduler_Button3State = Scheduler_Pending;
        Scheduler_Button3Callback();
    }
    if(currentButton4State == 1U && lastButton4State == 0U){
        Scheduler_Button4State = Scheduler_Pending;
        Scheduler_Button4Callback();
    }
    if(currentEncoderButtonState == 1U && lastEncoderButtonState == 0U){
        Scheduler_EncoderButtonState = Scheduler_Pending;
        Scheduler_EncoderButtonCallback();
    }

    lastButton1State = currentButton1State;
    lastButton2State = currentButton2State;
    lastButton3State = currentButton3State;
    lastButton4State = currentButton4State;
    lastEncoderButtonState = currentEncoderButtonState;
}

void Scheduler_Update(void){
    s_buttonCounter++;
    if (s_buttonCounter >= Scheduler_ButtonScaler)
    {
        s_buttonCounter = 0U;

        s_buttonUpdate();
    }

}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
    if(htim->Instance == TIM6){
        Scheduler_Update();
    }
}

