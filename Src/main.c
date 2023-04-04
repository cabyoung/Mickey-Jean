/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
//#define OV2640_320x240
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
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef hlpuart1;
UART_HandleTypeDef huart2;

SPI_HandleTypeDef hspi1;

/* USER CODE BEGIN PV */
const uint8_t SRAM_READ = 0b00000011; //read data from memory array beginning @ selected address
const uint8_t SRAM_WRITE = 0b00000010; //write data to memory array beg @ selected address
const uint8_t SRAM_ESDI = 0b00111011; //sdi mode
const uint8_t SRAM_ESQI = 0b00111000; //sqi mode
const uint8_t SRAM_RSTDQI = 0b11111111; //reset sdi/sqi mode
const uint8_t SRAM_RDMR = 0b00000101; //read mode register
const uint8_t SRAM_WRMR = 0b00000001; //write mode register
static const uint8_t PROXSENSOR_ADDR = 0x60 << 1;
const uint8_t PS_DATA = 0x08;
static const uint8_t GPS_ADDR = 0x42 << 1;
static const uint8_t ARDUCAM_ADDR = 0x30 << 1;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_LPUART1_UART_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void SRAM(void)
{
	//variables
	char spi_buf[20];
	uint8_t addr;
	uint8_t srcbuf[3];
	int uart_buf_len;
	char uart_buf[50];
	addr = 0x01;
	spi_buf[0] = 0x00;
	//write correct mode
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4 , GPIO_PIN_RESET);//CS low to start write
	HAL_SPI_Transmit(&hspi1, (uint8_t *)&SRAM_WRMR, 1, 100);//0x1
	HAL_SPI_Transmit(&hspi1, (uint8_t*)spi_buf, 1, 100);//0x0
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4 , GPIO_PIN_SET);//CS high to end write
	//read mode
	spi_buf[0] = 0x00;
	addr = 0x01;
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4 , GPIO_PIN_RESET);//CS low to start read
	HAL_SPI_Transmit(&hspi1, (uint8_t*)&SRAM_RDMR, 1, 100); //0x5
	HAL_SPI_Receive(&hspi1, (uint8_t*)spi_buf, 1, 100);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4 , GPIO_PIN_SET);//CS high to end read
	//write to mem
	spi_buf[0] = 0xFF;
	addr = 0x30;
	srcbuf[0] = (addr << 16);
	srcbuf[1] = (addr << 8);
	srcbuf[2] = (addr << 0);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4 , GPIO_PIN_RESET);
	HAL_SPI_Transmit(&hspi1, (uint8_t*)&SRAM_WRITE, 1, 100);//0x2
	HAL_SPI_Transmit(&hspi1, (uint8_t*)&srcbuf, 3, 100);
	HAL_SPI_Transmit(&hspi1, (uint8_t*)spi_buf, 1, 100);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4 , GPIO_PIN_SET);
	uart_buf_len= sprintf(uart_buf, "SRAM Writes:%d\n\r", (unsigned int)spi_buf[0]);
	HAL_UART_Transmit(&hlpuart1, (uint8_t*)uart_buf, uart_buf_len, 100);
	//reread mode
	spi_buf[0] = 0x0;
	addr = 0x01;
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4 , GPIO_PIN_RESET);//CS low to start read
	HAL_SPI_Transmit(&hspi1, (uint8_t*)&SRAM_RDMR, 1, 100); //0x5
	HAL_SPI_Receive(&hspi1, (uint8_t*)spi_buf, 1, 100);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4 , GPIO_PIN_SET);//CS high to end read
	//read from mem
	spi_buf[0] = 0x0;
	addr = 0x00030;
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4 , GPIO_PIN_RESET);//CS low to start read
	HAL_SPI_Transmit(&hspi1, (uint8_t*)&SRAM_READ, 1, 100); //0x3
	HAL_SPI_Transmit(&hspi1, (uint8_t*)&srcbuf, 3, 100);
	HAL_SPI_Transmit(&hspi1, (uint8_t*)&srcbuf, 3, 100);
	HAL_SPI_Receive(&hspi1, (uint8_t*)spi_buf, 1, 100);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4 , GPIO_PIN_SET);//CS high to end read
	uart_buf_len= sprintf(uart_buf, "SRAM Reads:%d\n\r", (unsigned int)spi_buf[0]);
	HAL_UART_Transmit(&hlpuart1, (uint8_t*)uart_buf, uart_buf_len, 100);
}
void PROXSENSOR(void)
{
	//Proximity sensor test--works
	uint8_t buf[256];
	int uart_buf_len;
	char uart_buf[50];
	HAL_Delay(500);
	buf[0] = 0x00;
	buf[1] = 0x00;
	buf[2] = 0x00;
	buf[3] = 0x00;
	//initialization
	HAL_I2C_Mem_Write(&hi2c1, PROXSENSOR_ADDR, 0x04, 1, (uint8_t*)buf, 2, HAL_MAX_DELAY);//IRED current
	HAL_I2C_Mem_Write(&hi2c1, PROXSENSOR_ADDR, 0x03, 1, (uint8_t*)buf, 2, HAL_MAX_DELAY);//IRED current
	HAL_I2C_Mem_Write(&hi2c1, PROXSENSOR_ADDR, 0x06, 1, (uint8_t*)buf, 2, HAL_MAX_DELAY);//IRED current
	HAL_I2C_Mem_Read(&hi2c1, PROXSENSOR_ADDR, 0x08, I2C_MEMADD_SIZE_8BIT, (uint8_t*)buf, 2, HAL_MAX_DELAY);//only read
	uart_buf_len= sprintf(uart_buf, "Proximity Sensor Reads:%d%d\n\r", (unsigned int)buf[0], (unsigned int)buf[1]);
	HAL_UART_Transmit(&hlpuart1, (uint8_t*)uart_buf, uart_buf_len, 100);
	HAL_Delay(500);
}
void GPS(void)
{
	uint8_t buf[256];
	int uart_buf_len;
	char uart_buf[50];
	buf[0] = 0xFF;
	HAL_I2C_Master_Transmit(&hi2c1, GPS_ADDR,(uint8_t*)buf, 1, HAL_MAX_DELAY);
	buf[0] = 0x00;
	HAL_Delay(1000);
	HAL_I2C_Master_Receive(&hi2c1, GPS_ADDR, (uint8_t*)buf, 256, HAL_MAX_DELAY);
	uint8_t x = 0;
	HAL_UART_Transmit(&hlpuart1, (uint8_t*)buf, 256, 100);
	x++;
	while(x < 25)
	{
		uart_buf_len= sprintf(uart_buf, ":%d", (unsigned int)buf[x]);
		HAL_UART_Transmit(&hlpuart1, (uint8_t*)uart_buf, uart_buf_len, 100);
		HAL_Delay(500);
		x++;
	}
}

void PIC(void)
{
	uint8_t reg[1];
    	uint8_t data[1];

    	reg[0] = 0xFF;
    	data[0] = 0x01;
    	HAL_I2C_Mem_Write(&hi2c1, ARDUCAM_ADDR, reg[0], 1, (uint8_t*)data, 1, HAL_MAX_DELAY);

    	reg[0] = 0x12;
    	data[0] = 0x80;
    	HAL_I2C_Mem_Write(&hi2c1, ARDUCAM_ADDR, reg[0], 1, (uint8_t*)data, 1, HAL_MAX_DELAY);

    	HAL_Delay(200); //
    	uint8_t init[193][2] = {
    			{0xff, 0x0},
    			{0x2c, 0xff},
    			{0x2e, 0xdf},
    			{0xff, 0x1},
    			{0x3c, 0x32},
    			{0x11, 0x0},
    			{0x09, 0x2},
    			{0x04, 0xa8},
    			{0x13, 0xe5},
    			{0x14, 0x48},
    			{0x2c, 0xc},
    			{0x33, 0x78},
    			{0x3a, 0x33},
    			{0x3b, 0xfb},
    			{0x3e, 0x0},
    			{0x43, 0x11},
    			{0x16, 0x10},
    			{0x39, 0x2},
    			{0x35, 0x88},



    			{0x22, 0xa},
    			{0x37, 0x40},
    			{0x23, 0x0},
    			{0x34, 0xa0},
    			{0x06, 0x2},
    			{0x06, 0x88},
    			{0x07, 0xc0},
    			{0x0d, 0xb7},
    			{0x0e, 0x1},
    			{0x4c, 0x0},
    			{0x4a, 0x81},
    			{0x21, 0x99},
    			{0x24, 0x40},
    			{0x25, 0x38},
    			{0x26, 0x82},
    			{0x5c, 0x0},
    			{0x63, 0x0},
    			{0x46, 0x22},
    			{0x0c, 0x3a},
    			{0x5d, 0x55},
    			{0x5e, 0x7d},
    			{0x5f, 0x7d},
    			{0x60, 0x55},
    			{0x61, 0x70},
    			{0x62, 0x80},
    			{0x7c, 0x5},
    			{0x20, 0x80},
    			{0x28, 0x30},
    			{0x6c, 0x0},
    			{0x6d, 0x80},
    			{0x6e, 0x0},
    			{0x70, 0x2},
    			{0x71, 0x94},
    			{0x73, 0xc1},
    			{0x3d, 0x34},
    			{0x12, 0x4},
    			{0x5a, 0x57},
    			{0x4f, 0xbb},
    			{0x50, 0x9c},
    			{0xff, 0x0},
    			{0xe5, 0x7f},
    			{0xf9, 0xc0},
    			{0x41, 0x24},
    			{0xe0, 0x14},
    			{0x76, 0xff},
    			{0x33, 0xa0},
    			{0x42, 0x20},
    			{0x43, 0x18},
    			{0x4c, 0x0},
    			{0x87, 0xd0},
    			{0x88, 0x3f},
    			{0xd7, 0x3},
    			{0xd9, 0x10},
    			{0xd3, 0x82},
    			{0xc8, 0x8},
    			{0xc9, 0x80},
    			{0x7c, 0x0},
    			{0x7d, 0x0},
    			{0x7c, 0x3},
    			{0x7d, 0x48},
    			{0x7d, 0x48},
    			{0x7c, 0x8},
    			{0x7d, 0x20},
    			{0x7d, 0x10},
    			{0x7d, 0xe},
    			{0x90, 0x0},
    			{0x91, 0xe},
    			{0x91, 0x1a},
    			{0x91, 0x31},
    			{0x91, 0x5a},
    			{0x91, 0x69},
    			{0x91, 0x75},
    			{0x91, 0x7e},
    			{0x91, 0x88},
    			{0x91, 0x8f},
    			{0x91, 0x96},
    			{0x91, 0xa3},
    			{0x91, 0xaf},
    			{0x91, 0xc4},
    			{0x91, 0xd7},
    			{0x91, 0xe8},
    			{0x91, 0x20},
    			{0x92, 0x0},



    			{0x93, 0x6},
    			{0x93, 0xe3},
    			{0x93, 0x3},
    			{0x93, 0x3},
    			{0x93, 0x0},
    			{0x93, 0x2},
    			{0x93, 0x0},
    			{0x93, 0x0},
    			{0x93, 0x0},
    			{0x93, 0x0},
    			{0x93, 0x0},
    			{0x93, 0x0},
    			{0x93, 0x0},
    			{0x96, 0x0},
    			{0x97, 0x8},
    			{0x97, 0x19},
    			{0x97, 0x2},
    			{0x97, 0xc},
    			{0x97, 0x24},
    			{0x97, 0x30},
    			{0x97, 0x28},
    			{0x97, 0x26},
    			{0x97, 0x2},
    			{0x97, 0x98},
    			{0x97, 0x80},
    			{0x97, 0x0},
    			{0x97, 0x0},
    			{0xa4, 0x0},
    			{0xa8, 0x0},
    			{0xc5, 0x11},
    			{0xc6, 0x51},
    			{0xbf, 0x80},
    			{0xc7, 0x10},
    			{0xb6, 0x66},
    			{0xb8, 0xa5},
    			{0xb7, 0x64},
    			{0xb9, 0x7c},
    			{0xb3, 0xaf},
    			{0xb4, 0x97},
    			{0xb5, 0xff},
    			{0xb0, 0xc5},
    			{0xb1, 0x94},
    			{0xb2, 0xf},
    			{0xc4, 0x5c},
    			{0xa6, 0x0},
    			{0xa7, 0x20},
    			{0xa7, 0xd8},
    			{0xa7, 0x1b},
    			{0xa7, 0x31},
    			{0xa7, 0x0},
    			{0xa7, 0x18},
    			{0xa7, 0x20},
    			{0xa7, 0xd8},
    			{0xa7, 0x19},
    			{0xa7, 0x31},
    			{0xa7, 0x0},
    			{0xa7, 0x18},
    			{0xa7, 0x20},
    			{0xa7, 0xd8},
    			{0xa7, 0x19},
    			{0xa7, 0x31},
    			{0xa7, 0x0},
    			{0xa7, 0x18},
    			{0x7f, 0x0},
    			{0xe5, 0x1f},
    			{0xe1, 0x77},
    			{0xdd, 0x7f},
    			{0xc2, 0xe},



    			{0xff, 0x0},
    			{0xe0, 0x4},
    			{0xc0, 0xc8},
    			{0xc1, 0x96},
    			{0x86, 0x3d},
    			{0x51, 0x90},
    			{0x52, 0x2c},
    			{0x53, 0x0},
    			{0x54, 0x0},
    			{0x55, 0x88},
    			{0x57, 0x0},



    			{0x50, 0x92},
    			{0x5a, 0x50},
    			{0x5b, 0x3c},
    			{0x5c, 0x0},
    			{0xd3, 0x4},
    			{0xe0, 0x0},



    			{0xff, 0x0},
    			{0x05, 0x0},



    			{0xda, 0x8},
    			{0xd7, 0x3},
    			{0xe0, 0x0},



    			{0x05, 0x0},




    			//{0xff,0xff}
    	};

    	int i;
    	for(i=0;i<193;i++) {
    			reg [0]= init[i][0];
    			data [0]= init[i][1];
    			HAL_I2C_Mem_Write(&hi2c1, ARDUCAM_ADDR, reg[0], 1, (uint8_t*)data, 1, HAL_MAX_DELAY);
    	}

    	HAL_Delay(100);//how much - 100
    	//sensor

    	uint8_t add[2];
    	add[0] = 0x83;
    	add[1] = 0x18;
    	uint8_t dat[2];
    	dat[0] = 0x00;
    	uint8_t cov = 0x00;

    	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 , GPIO_PIN_RESET);//CS low to use camera
    	HAL_SPI_Transmit(&hspi1, (uint8_t *)&add, 2, HAL_MAX_DELAY);
    	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 , GPIO_PIN_SET);//CS low to use camera
    	//fifo
    	add[0] = 0x84;
    	add[1] = 0x20;  //0x33;//MCU write on bus //0x20
    	dat[0] = 0x00;
    	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 , GPIO_PIN_RESET);//CS low to use camera
    	HAL_SPI_Transmit(&hspi1, (uint8_t *)&add, 2, HAL_MAX_DELAY);
    	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 , GPIO_PIN_SET);//CS low to use camera

    	add[1] = 0x10; // and 0x02; //0x33;//MCU write on bus
    	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 , GPIO_PIN_RESET);//CS low to use camera
    	HAL_SPI_Transmit(&hspi1, (uint8_t *)&add, 2, HAL_MAX_DELAY);
    	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 , GPIO_PIN_SET);//CS low to use camera

    	add[1] = 0x02; //0x10 and 0x02; //0x33;//MCU write on bus
    	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 , GPIO_PIN_RESET);//CS low to use camera
    	HAL_SPI_Transmit(&hspi1, (uint8_t *)&add, 2, HAL_MAX_DELAY);
    	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 , GPIO_PIN_SET);//CS low to use camera

    	//read flag

    	dat[0] = 0;
    	add[0] = 0x41;
    	add[1] = 0x00;
    	HAL_Delay(10);//maybe needed
    	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 , GPIO_PIN_RESET);//CS low to use camera
    	HAL_SPI_Transmit(&hspi1, (uint8_t *)&add, 1, HAL_MAX_DELAY);
    	HAL_SPI_Receive(&hspi1, (uint8_t *)&dat, 1, HAL_MAX_DELAY);
    	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 , GPIO_PIN_SET);//CS low to use camera

    	//burst reads
    	uint8_t dat1[1];
    	dat[0] = 0;
    	add[0] = 0x3C;
    	add[1] = 0x00;//MCU write on bus
    	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 , GPIO_PIN_RESET);//CS low to use camera
    	HAL_SPI_Transmit(&hspi1, (uint8_t *)&add, 1, HAL_MAX_DELAY);
    	HAL_SPI_Receive(&hspi1, (uint8_t *)&dat, 1, HAL_MAX_DELAY); //dummy receive
    	HAL_UART_Transmit(&huart2, (uint8_t *)"IMG", 3, HAL_MAX_DELAY);
    	HAL_UART_Transmit(&huart2, (uint8_t *) &cov, 1, HAL_MAX_DELAY);
    	HAL_UART_Transmit(&huart2, (uint8_t *) &cov, 1, HAL_MAX_DELAY);
    	dat[0] = 0;
    	//transmit two bytes of data at a time
    	uint8_t c = 0;
    	uint16_t y = 0;
    	uint32_t count = 0;
    	while(c < 240)
    	{
    		c++;
    		for(y = 0; y < 320; y++)
    		{
    			HAL_SPI_Receive(&hspi1, (uint8_t *)&dat, 1, HAL_MAX_DELAY);
    			HAL_SPI_Receive(&hspi1, (uint8_t *)&dat1, 1, HAL_MAX_DELAY);
    			//HAL_UART_Transmit(&huart2, (uint8_t *)&dat, 1, HAL_MAX_DELAY);//transmit two bytes of data at a time
    			if(((c >= 40) && (c < 200)) && ((y >= 100) && (y < 220))){
    				HAL_UART_Transmit(&huart2, (uint8_t *)&dat1, 1, HAL_MAX_DELAY);//transmit two bytes of data at a time
    				HAL_UART_Transmit(&huart2, (uint8_t *)&dat, 1, HAL_MAX_DELAY);//transmit two bytes of data at a time
    				count++;
    			}
    		}

    	}
    	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0 , GPIO_PIN_SET);//CS low to use camera
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

	//char uart_buf[50];

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
  MX_I2C1_Init();
  MX_LPUART1_UART_Init();
  MX_SPI1_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	uint8_t uart_buf[5];
	uint8_t welcome[] = "Welcome to Mickey Jean :)\n\r";
	HAL_UART_Transmit(&hlpuart1, welcome, sizeof(welcome), 10000);
	uint8_t menu[] = "JIN-SRAM\n\rTAY-Proximity Sensor\n\rJOON-GPS\n\rSUGA-Camera\n\r";
	HAL_UART_Transmit(&hlpuart1, menu, sizeof(menu), 10000);
	uart_buf[0] = 0;
	uart_buf[1] = 0;
	uart_buf[2] = 0;
	uart_buf[3] = 0;
	HAL_UART_Receive(&hlpuart1, (uint8_t*)uart_buf, 4, 10000);
	if (uart_buf[0] == 'J' && uart_buf[1] == 'I' && uart_buf[2] == 'N'){
	  uint8_t sramtest[] = "Memory Check!\n\r";
	  HAL_UART_Transmit(&hlpuart1, sramtest, sizeof(sramtest), 10000);
	  SRAM();
	}
	if (uart_buf[0] == 'T' && uart_buf[1] == 'A' && uart_buf[2] == 'Y'){
	  uint8_t prox[] = "Proximity Sensor\n\r";
	  HAL_UART_Transmit(&hlpuart1, prox, sizeof(prox), 10000);
	  PROXSENSOR();
	}
	if (uart_buf[0] == 'J' && uart_buf[1] == 'O' && uart_buf[2] == 'O' && uart_buf[3] == 'N'){
	  uint8_t gps[] = "GPS: Where are you?\n\r";
	  HAL_UART_Transmit(&hlpuart1, gps, sizeof(gps), 10000);
	  GPS();
	}
	if (uart_buf[0] == 'S' && uart_buf[1] == 'U' && uart_buf[2] == 'G' && uart_buf[3] == 'A'){
	  uint8_t pic[] = "Snap a Pic!\n\r";
	  HAL_UART_Transmit(&hlpuart1, pic, sizeof(pic), 10000);
	  PIC();
	}
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_5;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_LPUART1
                              |RCC_PERIPHCLK_I2C1;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.Lpuart1ClockSelection = RCC_LPUART1CLKSOURCE_PCLK1;
  PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x00000708;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief LPUART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_LPUART1_UART_Init(void)
{

  /* USER CODE BEGIN LPUART1_Init 0 */

  /* USER CODE END LPUART1_Init 0 */

  /* USER CODE BEGIN LPUART1_Init 1 */

  /* USER CODE END LPUART1_Init 1 */
  hlpuart1.Instance = LPUART1;
  hlpuart1.Init.BaudRate = 9600;
  hlpuart1.Init.WordLength = UART_WORDLENGTH_8B;
  hlpuart1.Init.StopBits = UART_STOPBITS_1;
  hlpuart1.Init.Parity = UART_PARITY_NONE;
  hlpuart1.Init.Mode = UART_MODE_TX_RX;
  hlpuart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  hlpuart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  hlpuart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&hlpuart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LPUART1_Init 2 */

  /* USER CODE END LPUART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin : PA4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB5 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
