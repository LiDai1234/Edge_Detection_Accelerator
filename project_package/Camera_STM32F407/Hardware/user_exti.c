#include "stm32f4xx_hal.h"
#include "user_exti.h"
#include "ov7670.h"
#include "HAL_Delay.h"

uint8_t frame_flag = 0;
uint16_t frame_cnt = 0;      //帧计数
uint32_t start_tick = 0;
float fps = 0.0f;

/**************************************************************************************
描  述 : 初始化OV7670模块VSYNC引脚PB3为外部中断输入，下降沿触发（原代码注释写错：Mode是FALLING）
入  参 : 无
返回值 : 无
**************************************************************************************/
void OV7670_PB3_EXTI_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* 开启GPIOB时钟 */
    __HAL_RCC_GPIOB_CLK_ENABLE();

    // PB3 外部中断，上拉输入
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;  // 下降沿外部中断（你原来代码注释写的上升沿，和代码不一致，按需改IT_RISING）
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // NVIC配置 EXTI3 中断
    HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 1);
    HAL_NVIC_EnableIRQ(EXTI3_IRQn);
}

/**************************************************************************************
描  述 : EXTI3中断服务函数（PB3对应EXTI3）
入  参 : 无
返回值 : 无
**************************************************************************************/
void EXTI3_IRQHandler(void)
{
    // HAL库统一入口
    HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_3);
}

/**
@brief  GPIO外部中断回调函数，PB3中断触发后进入这里
@param  GPIO_Pin:触发中断的引脚
*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == GPIO_PIN_3)
    {
		frame_flag++;		//帧中断标记累加1
//		if(frame_flag == 1)
//		{
			// FIFO写指针复位，WR就是你现在PE9
			OV7670_WRST_L;	   //FIFO写指针复位
			HAL_Delay_us(150);
			OV7670_WRST_H;		
			OV7670_WR_H;		   //FIFO写使能，允许写入FIFO

//		}
//		else if(frame_flag >= 2)
//		{
//			OV7670_WR_L;		   //关闭FIFO写使能，不允许写入FIFO，防止下一帧覆盖
//			EXTI->IMR &= ~(1U << 3);
//			
//		
//		}
    }

}

