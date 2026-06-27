#include "analogOutput.h"

#include "dac.h"

static uint8_t s_analogOutputInitialized = 0U;

void AnalogOutput_Init(void)
{
    if (s_analogOutputInitialized != 0U) {
        return;
    }

    if (HAL_DAC_Start(&hdac1, DAC_CHANNEL_1) != HAL_OK) {
        return;
    }

    if (HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 0U) != HAL_OK) {
        return;
    }

    s_analogOutputInitialized = 1U;
}

void AnalogOutput_SetRawValue(uint16_t rawValue)
{
    uint32_t dacValue;

    if (s_analogOutputInitialized == 0U) {
        return;
    }

    dacValue = rawValue;
    if (dacValue > AnalogOutput_DACMaxRawValue) {
        dacValue = AnalogOutput_DACMaxRawValue;
    }

    (void)HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, dacValue);
}

void AnalogOutput_SetVoltage(float voltage)
{
    uint16_t rawValue;

    if (!(voltage > 0.0f)) {
        rawValue = 0U;
    } else if (voltage >= AnalogOutput_VoltageReference) {
        rawValue = (uint16_t)AnalogOutput_DACMaxRawValue;
    } else {
        rawValue = (uint16_t)((voltage * (float)AnalogOutput_DACMaxRawValue /
                               AnalogOutput_VoltageReference) + 0.5f);
    }

    AnalogOutput_SetRawValue(rawValue);
}
