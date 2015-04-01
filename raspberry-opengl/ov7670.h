#ifndef OV7670_H
#define OV7670_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bcm2835.h"

void OV_ReadFrames(uint8_t *frameLeft, uint8_t *frameRight, uint32_t *args);
void OV_InitCommunication(void);
void OV_EndCommunication(void);

#ifdef __cplusplus
}
#endif

#endif
