#ifndef __OV7670_H
#define __OV7670_H

#include "stm32f4xx_hal.h"
#include <stdint.h>

// -------------- 引脚宏定义（HAL库版本）----------------
#define OV7670_RRST_H    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_SET)
#define OV7670_RRST_L    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET)

#define OV7670_RCK_H     HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET)
#define OV7670_RCK_L     HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET)

#define OV7670_OE_H      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET)
#define OV7670_OE_L      HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET)

#define OV7670_WRST_H    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_SET)
#define OV7670_WRST_L    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET)

#define OV7670_WR_L    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET)
#define OV7670_WR_H    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET)

// 读取PC0~PC7 8位数据总线
//#define OV7670_DATA      (GPIOC->IDR & 0x00FF)

#define CHANGE_REG_NUM  171                    //需要配置的寄存器总数		  



#define IMG_WIDTH  160
#define IMG_HEIGHT 146
//单行缓存，只存一行像素 240*2 = 480字节
extern uint8_t line_buf[IMG_WIDTH*2];

// 读取HREF引脚电平
#define READ_HREF    HAL_GPIO_ReadPin(GPIOD, GPIO_PIN_7)
// 读取PCLK引脚电平
#define READ_PCLK    HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_8)

#define OV_VSYNC_PIN        GPIO_PIN_3
#define OV_VSYNC_PORT       GPIOB

#define OV_OE_PIN           GPIO_PIN_8
#define OV_OE_PORT          GPIOA

#define OV_WRST_PIN         GPIO_PIN_10
#define OV_WRST_PORT        GPIOA

#define OV_RCLK_PIN         GPIO_PIN_12
#define OV_RCLK_PORT        GPIOA

#define OV_RRST_PIN         GPIO_PIN_9
#define OV_RRST_PORT        GPIOA

#define OV_WR_PIN           GPIO_PIN_11   
#define OV_WR_PORT          GPIOA




// 函数声明
extern uint8_t OV7670_Init(void);	

extern void OV7670_StartReadLine(void);
extern void OV7670_ReadFrameToLCD(uint16_t lcd_x, uint16_t lcd_y);

extern void OV7670_Light_Mode(uint8_t mode);
extern void OV7670_Color_Saturation(uint8_t tone);
extern void OV7670_Brightness(uint8_t bright);
extern void OV7670_Contrast(uint8_t contrast);
extern void OV7670_Special_Effects(uint8_t effect);
extern void OV7670_Window_Set(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height);

#endif

