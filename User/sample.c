#include "sample.h"

#include "adc.h"

#define SAMPLE_INJECTED_TIMEOUT_MS 10U
#define SAMPLE_ADC1_CHANNEL_COUNT 3U
#define SAMPLE_ADC2_CHANNEL_COUNT 3U
#define SAMPLE_ADC3_CHANNEL_COUNT 2U

#define SAMPLE_BufferSize (SAMPLE_ADC1_CHANNEL_COUNT + SAMPLE_ADC2_CHANNEL_COUNT + SAMPLE_ADC3_CHANNEL_COUNT)
uint16_t SAMPLE_Buffer[SAMPLE_BufferSize] = {0U};
float VoltageScaleFactor = SAMPLE_VoltageReference / (float)SAMPLE_ADCResolution;

static uint16_t s_vrefintRaw = 0U;

SAMPLE_StatusTypeDef Sample_Init(void){
    if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK) {
        return SAMPLE_ERROR;
    }
    if (HAL_ADCEx_Calibration_Start(&hadc2, ADC_SINGLE_ENDED) != HAL_OK) {
        return SAMPLE_ERROR;
    }
    if (HAL_ADCEx_Calibration_Start(&hadc3, ADC_SINGLE_ENDED) != HAL_OK) {
        return SAMPLE_ERROR;
    }

	if (HAL_ADCEx_InjectedStart(&hadc1) != HAL_OK) {
		return SAMPLE_ERROR;
	}

	if (HAL_ADCEx_InjectedPollForConversion(&hadc1, SAMPLE_INJECTED_TIMEOUT_MS) != HAL_OK) {
		return SAMPLE_TIMEOUT;
	}

	s_vrefintRaw = (uint16_t)HAL_ADCEx_InjectedGetValue(&hadc1, ADC_INJECTED_RANK_1);
    if (HAL_ADC_Start_DMA(&hadc1, (uint32_t*)&SAMPLE_Buffer[0], SAMPLE_ADC1_CHANNEL_COUNT) != HAL_OK) {
        return SAMPLE_ERROR;
    }
    if (HAL_ADC_Start_DMA(&hadc2, (uint32_t*)&SAMPLE_Buffer[SAMPLE_ADC1_CHANNEL_COUNT], SAMPLE_ADC2_CHANNEL_COUNT) != HAL_OK) {
        return SAMPLE_ERROR;
    }
    if (HAL_ADC_Start_DMA(&hadc3, (uint32_t*)&SAMPLE_Buffer[SAMPLE_ADC1_CHANNEL_COUNT + SAMPLE_ADC2_CHANNEL_COUNT], SAMPLE_ADC3_CHANNEL_COUNT) != HAL_OK) {
        return SAMPLE_ERROR;
    }
	return SAMPLE_OK;
}

uint16_t Sample_GetRaw(Sample_ChannelTypeDef channel) {
    if (channel >= Sample_ChannelCount) {
        return 0U;
    }

    return SAMPLE_Buffer[channel];
}

float Sample_GetPinVoltage(Sample_ChannelTypeDef channel) {
    if (channel >= Sample_ChannelCount) {
        return 0.0f;
    }

    return ((float)SAMPLE_Buffer[channel] * VoltageScaleFactor);
}

float Sample_GetTheoreticalVoltage(Sample_ChannelTypeDef channel){
    if (channel >= Sample_ChannelCount) {
        return 0.0f;
    }
    return 62.6f * (Sample_GetPinVoltage(channel) - 1.65f);
}

float Sample_GetTheoreticalCurrent(Sample_ChannelTypeDef channel){
    if (channel >= Sample_ChannelCount) {
        return 0.0f;
    }
    return 3.049f * (Sample_GetPinVoltage(channel) - 1.65f);
}

