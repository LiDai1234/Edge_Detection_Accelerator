#include "stm32f4xx_hal.h"
#include "ov7670.h"
#include "sccb.h"
#include "lcd.h"
#include "ov7670config.h"	

uint8_t line_buf[IMG_WIDTH*2]; //单行缓存，仅480字节，F103完全放得下

// 采集一行OV7670数据存入line_buf，截取前240像素
void OV7670_ReadOneLine(uint8_t *buf)
{
	uint16_t px_cnt = 0;
	uint8_t byte1,byte2;
	while(READ_HREF == GPIO_PIN_SET) //HREF高电平=本行有效像素
	{
		if(px_cnt >= IMG_WIDTH*2) break; //采集够240像素（480字节）就停止
		
		while(READ_PCLK == GPIO_PIN_RESET); //等待PCLK变高
		byte1 = GPIOB->IDR & 0x00FF; //读取D0~D7
		while(READ_PCLK == GPIO_PIN_SET); //等待PCLK变低

		while(READ_PCLK == GPIO_PIN_RESET);
		byte2 = GPIOB->IDR & 0x00FF;
		while(READ_PCLK == GPIO_PIN_SET);

		buf[px_cnt++] = byte1;
		buf[px_cnt++] = byte2;
	}
}

// 读取一帧，逐行采集，逐行刷新LCD
void OV7670_ReadFrameToLCD(uint16_t lcd_x, uint16_t lcd_y)
{
	uint16_t y;
	for(y=0; y<IMG_HEIGHT; y++)
	{
		OV7670_ReadOneLine(line_buf);
		//单行显示函数，替换你的LCD_ShowPicture，只画一行
		LCD_ShowLine(lcd_x, lcd_y+y, IMG_WIDTH, line_buf);
	}
}


/**************************************************************************************
 * 描  述 : OV7670芯片初始化（IO口由CubeMX完成，此处仅设置初始电平 + SCCB + 寄存器配置）
 * 入  参 : 无
 * 返回值 : 0,成功;1,复位;2,读ID错误.
 **************************************************************************************/
uint8_t OV7670_Init(void)
{
	uint8_t temp;
	uint16_t i = 0;
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	/* 开启对应GPIO时钟 */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();

	//==== VSYNC PB3 上拉输入 ====
	GPIO_InitStruct.Pin = OV_VSYNC_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(OV_VSYNC_PORT, &GPIO_InitStruct);

	//==== OE(PA8), WR PA11) 推挽输出 ====
	GPIO_InitStruct.Pin = OV_OE_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(OV_OE_PORT, &GPIO_InitStruct);

	GPIO_InitStruct.Pin = OV_WR_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(OV_WR_PORT, &GPIO_InitStruct);



	//==== WRST PA10 推挽输出 ====
	GPIO_InitStruct.Pin = OV_WRST_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(OV_WRST_PORT, &GPIO_InitStruct);

	//==== RCLK(即RCK) PA12 推挽输出 ====
	GPIO_InitStruct.Pin = OV_RCLK_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(OV_RCLK_PORT, &GPIO_InitStruct);

	//==== RRST PA9 推挽输出 ====
	GPIO_InitStruct.Pin = OV_RRST_PIN;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(OV_RRST_PORT, &GPIO_InitStruct);

	// 初始化输出引脚全部置高（原版上电默认高电平）
	HAL_GPIO_WritePin(OV_OE_PORT,  OV_OE_PIN,  GPIO_PIN_SET);
	HAL_GPIO_WritePin(OV_WR_PORT, OV_WR_PIN, GPIO_PIN_SET);
	HAL_GPIO_WritePin(OV_WRST_PORT,OV_WRST_PIN,GPIO_PIN_SET);
	HAL_GPIO_WritePin(OV_RCLK_PORT,OV_RCLK_PIN,GPIO_PIN_SET);
	HAL_GPIO_WritePin(OV_RRST_PORT,OV_RRST_PIN,GPIO_PIN_SET);

	SCCB_GPIO_Init();

	if(SCCB_Write_Reg(0x12,0x80))	//COM7软件复位
	{
		return 1;
	}
	HAL_Delay(50);

	temp = SCCB_Read_Reg(0x0b);	//VER ID
	if(temp != 0x73)
	{
		return 2;
	}
	temp = SCCB_Read_Reg(0x0a);	//PID
	if(temp != 0x76)
	{
		return 2;
	}

	//加载OV7670配置寄存器表
	for(i = 0; i < sizeof(ov7670_init_reg_table)/sizeof(ov7670_init_reg_table[0]); i++)
	{
		SCCB_Write_Reg(ov7670_init_reg_table[i][0], ov7670_init_reg_table[i][1]);
	}

	return 0x00;
}
/**************************************************************************************
 * 描  述 : 配置OV7670芯片相关寄存器以平衡色彩
 * 入  参 : uint8_t mode 场所模式
 * 返回值 : 无
 **************************************************************************************/
void OV7670_Light_Mode(uint8_t mode)
{
	uint8_t reg01val=0;
	uint8_t reg02val=0;
	uint8_t reg13val=0xE7;//默认就是设置为自动白平衡
	
	switch(mode)
	{
		case 1:                  //晴天
			reg01val=0x5A;
			reg02val=0x5C;
		  reg13val=0xE5;
			break;	
		case 2:                  //阴天
			reg01val=0x58;
			reg02val=0x60;
			reg13val=0xE5;
			break;	
		case 3:                  //办公室
			reg01val=0x84;
			reg02val=0x4c;
			reg13val=0xE5;
			break;	
		case 4:                  //居家
			reg01val=0x96;
			reg02val=0x40;
		  reg13val=0xE5;
			break;	
	}
	SCCB_Write_Reg(0x13,reg13val);     //SCCB总线写操作OV7670芯片COM7寄存器 
	SCCB_Write_Reg(0x01,reg01val);     //SCCB总线写操作OV7670芯片BLUE寄存器
	SCCB_Write_Reg(0x02,reg02val);     //SCCB总线写操作OV7670芯片RED寄存器
}				  
/**************************************************************************************
 * 描  述 : 配置OV7670芯片相关寄存器以进行色度设置
 * 入  参 : uint8_t tone 色度
 * 返回值 : 无
 **************************************************************************************/
void OV7670_Color_Saturation(uint8_t tone)
{
	uint8_t reg4fval=0x80;              //0
	uint8_t reg50val=0x80;
 	uint8_t reg52val=0x22;
	uint8_t reg53val=0x5E;
	uint8_t reg54val=0x80;
 	switch(tone)
	{
		case 0:                      //-2
			reg4fval=0x40; 
      reg50val=0x40;		
			reg52val=0x11;
			reg53val=0x2F;
		  reg54val=0x40;
			break;	
		case 1:                      //-1
			reg4fval=0x66;
      reg50val=0x66;		
			reg52val=0x1B;
			reg53val=0x4B;	
      reg54val=0x66;		
			break;	
		case 3:                      //1
			reg4fval=0x99;
      reg50val=0x99;			
			reg52val=0x28;
			reg53val=0x71;	
      reg54val=0x99;			
			break;	
		case 4:                      //2
			reg4fval=0xC0;
      reg50val=0xC0;		
			reg52val=0x33;
			reg53val=0x8D;
      reg54val=0xC0;		
			break;	
	}
	SCCB_Write_Reg(0x4F,reg4fval);	   //SCCB总线写操作OV7670芯片MTX1寄存器 
	SCCB_Write_Reg(0x50,reg50val);     //SCCB总线写操作OV7670芯片MTX2寄存器  
	SCCB_Write_Reg(0x51,0x00);	       //SCCB总线写操作OV7670芯片MTX3寄存器  
	SCCB_Write_Reg(0x52,reg52val);     //SCCB总线写操作OV7670芯片MTX4寄存器 
	SCCB_Write_Reg(0x53,reg53val);     //SCCB总线写操作OV7670芯片MTX5寄存器 
	SCCB_Write_Reg(0x54,reg54val);     //SCCB总线写操作OV7670芯片MTX6寄存器  
	SCCB_Write_Reg(0x58,0x9E);         //SCCB总线写操作OV7670芯片MTXS寄存器 
}
/**************************************************************************************
 * 描  述 : 配置OV7670芯片相关寄存器以进行亮度设置
 * 入  参 : uint8_t bright 亮度
 * 返回值 : 无
 **************************************************************************************/
void OV7670_Brightness(uint8_t bright)
{
	uint8_t reg55val=0x00;                      //0
  switch(bright)
	{
		case 0:                              //-2
			reg55val=0xB0;	 	 
			break;	
		case 1:                              //-1
			reg55val=0x98;	 	 
			break;	
		case 3:                              //1
			reg55val=0x18;	 	 
			break;	
		case 4:                              //2
			reg55val=0x30;	 	 
			break;	
	}
	SCCB_Write_Reg(0x55,reg55val);         //SCCB总线写操作OV7670芯片BRIGHT寄存器 
}
/**************************************************************************************
 * 描  述 : 配置OV7670芯片相关寄存器以进行对比度设置
 * 入  参 : uint8_t contrast 对比度
 * 返回值 : 无
 **************************************************************************************/
void OV7670_Contrast(uint8_t contrast)
{
	uint8_t reg56val=0x40;                      //0
  	switch(contrast)
	{
		case 0:                              //-2
			reg56val=0x30;	 	 
			break;	
		case 1:                              //-1
			reg56val=0x38;	 	 
			break;	
		case 3:                              //1
			reg56val=0x50;	 	 
			break;	
		case 4:                              //2
			reg56val=0x60;	 	 
			break;	
	}
	SCCB_Write_Reg(0x56,reg56val);         //SCCB总线写操作OV7670芯片CONTRAS寄存器 
}
	
/**************************************************************************************
 * 描  述 : 配置OV7670芯片相关寄存器以进行特效设置
 * 入  参 : uint8_t effect 特效
 * 返回值 : 无
 **************************************************************************************/
void OV7670_Special_Effects(uint8_t effect)
{
	uint8_t reg3aval=0x04;                      //普通模式
	uint8_t reg67val=0xC0;
	uint8_t reg68val=0x80;
	switch(effect)
	{
		case 1:                              //负片
			reg3aval=0x24;
			reg67val=0x80;
			reg68val=0x80;
			break;	
		case 2:                              //黑白
			reg3aval=0x14;
			reg67val=0x80;
			reg68val=0x80;
			break;	
		case 3:                              //偏红色
			reg3aval=0x14;
			reg67val=0xC0;
			reg68val=0x80;
			break;	
		case 4:                              //偏绿色
			reg3aval=0x14;
			reg67val=0x40;
			reg68val=0x40;
			break;	
		case 5:                              //偏蓝色
			reg3aval=0x14;
			reg67val=0x80;
			reg68val=0xC0;
			break;	
		case 6:                              //复古
			reg3aval=0x14;
			reg67val=0xA0;
			reg68val=0x40;
			break;	 
	}
	SCCB_Write_Reg(0x3A,reg3aval);         //SCCB总线写操作OV7670芯片TSLB寄存器
	SCCB_Write_Reg(0x67,reg67val);         //SCCB总线写操作OV7670芯片MANU寄存器
	SCCB_Write_Reg(0x68,reg68val);         //SCCB总线写操作OV7670芯片MANV寄存器
}	
/**************************************************************************************
 * 描  述 : 配置OV7670芯片相关寄存器以设置图像输出窗口
 * 入  参 : uint16_t sx,uint16_t sy,uint16_t width,uint16_t height
 * 返回值 : 无
 **************************************************************************************/
//void OV7670_Window_Set(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height)
//{
//	uint16_t endx;
//	uint16_t endy;
//	uint8_t temp; 
//	endx=sx+width*2;	               
// 	endy=sy+height*2;
//	if(endy>784)endy-=784;
//	temp=SCCB_Read_Reg(0x03);			         //SCCB总线读操作OV7670芯片VREF寄存器	
//	temp&=0xF0;
//	temp|=((endx&0x03)<<2)|(sx&0x03);
//	SCCB_Write_Reg(0x03,temp);			       //SCCB总线写操作OV7670芯片VREF寄存器	
//	SCCB_Write_Reg(0x19,sx>>2);		         //SCCB总线写操作OV7670芯片VSTRT寄存器
//	SCCB_Write_Reg(0x1A,endx>>2);	         //SCCB总线写操作OV7670芯片VSTOP寄存器

//	temp=SCCB_Read_Reg(0x32);			         //SCCB总线读操作OV7670芯片HREF寄存器	
//	temp&=0xC0;
//	temp|=((endy&0x07)<<3)|(sy&0x07);
//	SCCB_Write_Reg(0x17,sy>>3);		         //SCCB总线写操作OV7670芯片HSTART寄存器
//	SCCB_Write_Reg(0x18,endy>>3);	         //SCCB总线写操作OV7670芯片HSTOP寄存器
//}

void OV7670_Window_Set(uint16_t sx, uint16_t sy, uint16_t width, uint16_t height)
{
	uint16_t endx;
	uint16_t endy;
	uint8_t temp; 
	
	endx = sx + width *2;  // 【删掉 *2】             
 	endy = sy + height*2; // 【删掉 *2】
	if(endy > 784) endy -= 784;
    
	// 垂直方向 (Y)
	temp = SCCB_Read_Reg(0x03);			         
	temp &= 0xF0;
	temp |= ((endy & 0x03) << 2) | (sy & 0x03);
	SCCB_Write_Reg(0x03, temp);			       
	SCCB_Write_Reg(0x19, sy >> 2);		         
	SCCB_Write_Reg(0x1A, endy >> 2);	         
    
	// 水平方向 (X)
	temp = SCCB_Read_Reg(0x32);			         
	temp &= 0xC0;
	temp |= ((endx & 0x07) << 3) | (sx & 0x07);
	SCCB_Write_Reg(0x32, temp);			         
	SCCB_Write_Reg(0x17, sx >> 3);		         
	SCCB_Write_Reg(0x18, endx >> 3);	         
}
