/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h> // 为了使用 abs() 函数
#include "lcd.h"
#include "lcd_init.h"
#include "pic.h"
#include "HAL_Delay.h"
#include "sccb.h"
#include "ov7670.h"
#include "user_exti.h"
#include "FPGA_connect.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
uint8_t frame_buf[IMG_WIDTH * IMG_HEIGHT * 2]; 

//uint32_t value = 0;
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


void OV7670_tftlcd(void)
{
	uint16_t i, j;
	uint32_t idx = 0; 

	if(frame_flag)
	{
		for(i = 0; i < IMG_HEIGHT; i++)
		{
			for(j = 0; j < IMG_WIDTH; j++)
			{
				// 等待 FPGA 拉高 PC8
                while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_8) == GPIO_PIN_RESET);
				
                uint16_t pixel = (uint16_t)(GPIOE->IDR & 0xFFFF);

                // 将像素拆分为两个字节存入 frame_buf（保持 RGB565 格式）
                frame_buf[idx]     = (uint8_t)(pixel >> 8);   // 高字节
                frame_buf[idx + 1] = (uint8_t)(pixel & 0xFF); // 低字节
                idx += 2;
			}
		}
		//value = idx; //看一帧有多少字节
		LCD_ShowPicture(40, 0, IMG_WIDTH, IMG_HEIGHT, frame_buf);
		frame_cnt++;
		frame_flag = 0;
//		EXTI->IMR |= (1U << 3);
	}
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */
  LCD_Init();
  OV7670_Init();
  FPGA_DATA_Init();
  //OV7670_Window_Set(12, 3, 183, 144);
  FPGA_PC8_Init();
  OV7670_PB3_EXTI_Init();
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);  //OE PA8 低有效输出

  LCD_Fill(0,0,LCD_W,LCD_H,BLUE);//指定区域填充颜色

  uint8_t val1 = SCCB_Read_Reg(0x12);  //读取寄存器
	uint8_t val2 = SCCB_Read_Reg(0x40);  //读取寄存器
	uint8_t val3 = SCCB_Read_Reg(0x70);  //读取寄存器
	uint8_t val4 = SCCB_Read_Reg(0x71);  //读取寄存器
	uint8_t val5 = SCCB_Read_Reg(0x3a);  //读取寄存器
	uint8_t val6 = GPIOD->IDR;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  
	  
	  OV7670_tftlcd();
	  if(HAL_GetTick() - start_tick >= 1000)
      {
        fps = frame_cnt;       //把这一秒的帧数赋值给fps
        frame_cnt = 0;         //清零计数器，重新统计下一秒
        start_tick = HAL_GetTick(); //重置计时起点
      }
	
	
	LCD_ShowIntNum(48,150,fps,3,RED,WHITE,16);
	LCD_ShowHexNum(80, 150, val1, 3, RED, WHITE, 16);
	LCD_ShowHexNum(120, 150, val2, 3, RED, WHITE, 16);
	LCD_ShowHexNum(160, 150, val3, 3, RED, WHITE, 16);
	LCD_ShowHexNum(200, 150, val4, 3, RED, WHITE, 16);
	LCD_ShowHexNum(48, 190, val5, 3, RED, WHITE, 16);
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
