#ifndef __SCCB_H
#define __SCCB_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

// SCCB引脚定义（SCL PB10, SDA PB11）
#define SCCB_SCL_H    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_SET)
#define SCCB_SCL_L    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, GPIO_PIN_RESET)

#define SCCB_SDA_H    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET)
#define SCCB_SDA_L    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET)

#define SCCB_Read_SDA  HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4)

#define OV7670_ID     0x42  //OV7670写地址

void SCCB_GPIO_Init(void);
void SCCB_SDA_OUT(void);
void SCCB_SDA_IN(void);
void SCCB_Start(void);
void SCCB_Stop(void);
void SCCB_No_Ack(void);
uint8_t SCCB_WriteAbyte(uint8_t dat);
uint8_t SCCB_ReadAbyte(void);
uint8_t SCCB_Write_Reg(uint8_t reg,uint8_t data);
uint8_t SCCB_Read_Reg(uint8_t reg);

#endif
