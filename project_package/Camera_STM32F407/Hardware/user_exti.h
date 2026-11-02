#ifndef __USER_EXTI_H
#define __USER_EXTI_H
#include "stm32f4xx_hal.h"

extern uint8_t frame_flag;
extern uint16_t frame_cnt;      //Ö¡¼ÆÊý
extern uint32_t start_tick;
extern float fps;

void OV7670_PB3_EXTI_Init(void);

#endif
