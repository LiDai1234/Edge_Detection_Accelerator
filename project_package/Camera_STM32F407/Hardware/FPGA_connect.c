#include "stm32f4xx_hal.h"
#include "FPGA_connect.h"


void FPGA_DATA_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	
	__HAL_RCC_GPIOD_CLK_ENABLE();
	//==== PD0~PD15 数据口，上拉输入 ====
	GPIO_InitStruct.Pin = FPGA_DATA_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(FPGA_DATA_PORT, &GPIO_InitStruct);
}

void FPGA_PC8_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	__HAL_RCC_GPIOC_CLK_ENABLE();
	GPIO_InitStruct.Pin = GPIO_PIN_8;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}


