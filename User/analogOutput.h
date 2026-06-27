#ifndef ANALOGOUTPUT_H
#define ANALOGOUTPUT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define AnalogOutput_VoltageReference (2.92f)
#define AnalogOutput_DACResolution (4096U)
#define AnalogOutput_DACMaxRawValue (AnalogOutput_DACResolution - 1U)

void AnalogOutput_Init(void);
void AnalogOutput_SetRawValue(uint16_t rawValue);
void AnalogOutput_SetVoltage(float voltage);

#ifdef __cplusplus
}
#endif

#endif /* ANALOGOUTPUT_H */
