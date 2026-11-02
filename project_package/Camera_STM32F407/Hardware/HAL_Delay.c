#include "stm32f4xx_hal.h"
#include "HAL_Delay.h"

/**
 * @brief  微秒级阻塞延时，自动处理SysTick溢出重装
 * @param  nus  微秒，范围：1 ~ 1000000
 */
void HAL_Delay_us(uint32_t us)
{
	uint32_t fac_us;

	// SysTick时钟 = HCLK，无8分频
	fac_us = SystemCoreClock / 1000000UL;

	uint32_t ticks;
	uint32_t told, tnow, tcnt = 0;
	uint32_t reload = SysTick->LOAD;
	ticks = us * fac_us;
	told = SysTick->VAL;
	while (1)
	{
		tnow = SysTick->VAL;
		if (tnow != told)
		{
			if (tnow < told)
			{
				// 正常递减，没有溢出
				tcnt += told - tnow;
			}
			else
			{
				// 计数器减到0，发生重装溢出
				tcnt += reload - tnow + told;
			}
			told = tnow;
			if (tcnt >= ticks)
			{
				break;
			}
		}
	}
}


/**
 * @brief  秒级延时
 * @param  s: 延时秒数
 * @retval 无
 */
void HAL_Delay_s(uint32_t s)
{
    while(s--)
    {
        HAL_Delay(1000);  //HAL库自带毫秒延时
    }
}
