#ifndef Scheduler_H
#define Scheduler_H

#include "ButtonAndLED.h"

typedef enum {
    Scheduler_Unpending = 0U,
    Scheduler_Pending = 1U
} Scheduler_PendingStatusTypeDef;

volatile extern Scheduler_PendingStatusTypeDef Scheduler_Button1State;
volatile extern Scheduler_PendingStatusTypeDef Scheduler_Button2State;
volatile extern Scheduler_PendingStatusTypeDef Scheduler_Button3State;
volatile extern Scheduler_PendingStatusTypeDef Scheduler_Button4State;
volatile extern Scheduler_PendingStatusTypeDef Scheduler_EncoderButtonState;
static inline void Scheduler_ClearButton1Pending(void){
    Scheduler_Button1State = Scheduler_Unpending;
}
static inline void Scheduler_ClearButton2Pending(void){
    Scheduler_Button2State = Scheduler_Unpending;
}
static inline void Scheduler_ClearButton3Pending(void){
    Scheduler_Button3State = Scheduler_Unpending;
}
static inline void Scheduler_ClearButton4Pending(void){
    Scheduler_Button4State = Scheduler_Unpending;
}
static inline void Scheduler_ClearEncoderButtonPending(void){
    Scheduler_EncoderButtonState = Scheduler_Unpending;
}
__weak void Scheduler_Button1Callback(void);
__weak void Scheduler_Button2Callback(void);
__weak void Scheduler_Button3Callback(void);
__weak void Scheduler_Button4Callback(void);
__weak void Scheduler_EncoderButtonCallback(void);



void Scheduler_Init(void);

void Scheduler_Update(void);

#endif