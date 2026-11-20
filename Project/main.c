/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "queue.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "SysInit.h"

/* Private macro -------------------------------------------------------------*/
#define  CLEAR_BUFFER_CH 0 //'0'
#define TAM 16

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

UART_HandleTypeDef huart1;

/* Definitions for SendTask_Temp */
osThreadId_t SendTask_TempHandle;
const osThreadAttr_t SendTask_Temp_attributes = {
  .name = "SendTask_Temp",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for SendTask_Caudal */
osThreadId_t SendTask_CaudalHandle;
const osThreadAttr_t SendTask_Caudal_attributes = {
  .name = "SendTask_Caudal",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh4,
};
/* Definitions for SendTask_Ph */
osThreadId_t SendTask_PhHandle;
const osThreadAttr_t SendTask_Ph_attributes = {
  .name = "SendTask_Ph",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityRealtime,
};
/* Definitions for ReceivTask_HMI */
osThreadId_t ReceivTask_HMIHandle;
const osThreadAttr_t ReceivTask_HMI_attributes = {
  .name = "ReceivTask_HMI",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for SendTask_Conduc */
osThreadId_t SendTask_ConducHandle;
const osThreadAttr_t SendTask_Conduc_attributes = {
  .name = "SendTask_Conduc",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh7,
};
/* Definitions for SendTask_Redox */
osThreadId_t SendTask_RedoxHandle;
const osThreadAttr_t SendTask_Redox_attributes = {
  .name = "SendTask_Redox",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh5,
};
/* Definitions for SendTask_Humed */
osThreadId_t SendTask_HumedHandle;
const osThreadAttr_t SendTask_Humed_attributes = {
  .name = "SendTask_Humed",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};

/* Definitions for Cyclic_ErrorHan */
osThreadId_t Cyclic_ErrorHanHandle;
const osThreadAttr_t Cyclic_ErrorHan_attributes = {
  .name = "Cyclic_ErrorHan",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityRealtime3,
};

/* Definitions for myQueue01 */
osMessageQueueId_t myQueue01Handle;
const osMessageQueueAttr_t myQueue01_attributes = {
  .name = "myQueue01"
};
/* USER CODE BEGIN PV */
typedef enum
{
	eSender1_Ph,
	eSender2_Temp,
	eSender3_Caudal,
	eSender4_Conduct,
	eSender5_Redox,
	eSender6_Humed
} DataSource_t;

typedef struct
{
	//uint32_t u32Value;
	uint8_t u8QUE_TxBufer[TAM];
	DataSource_t eDataSource;
} Data_t;

static  Data_t xStructsToSend[ 6 ] =
{
	{ "0.0", eSender1_Ph }, /* Used by Sender1 Ph. */
	{ "0.0", eSender2_Temp }, /* Used by Sender2 Temperature. */
	{ "0.0", eSender3_Caudal }, /* Used by Sender3 Caudal */
	{ "0.0", eSender4_Conduct }, /* Used by Sender4 Conduct */
	{ "0.0", eSender5_Redox }, /* Used by Sender5 Redox */
	{ "0.0", eSender6_Humed }, /* Used by Sender5 Humed */
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

void vSendTask_Ph(void *argument);
void vSendTask_Temp(void *argument);
void vSendTask_Conduct(void *argument);
void vSendTask_Caudal(void *argument);
void vSendTask_Redox(void *argument);
void vSendTask_Humed(void *argument);

void vCyclic_ErrorHan(void *argument);
void vReceivTask_HMI(void *argument);

static const uint8_t TMP100_ADDR = 0x64 << 1; // Use 7-bit address
static const uint8_t TMP102_ADDR = 0x66 << 1; // Use 7-bit address

int main(void)
{
  /* Initialize all configured peripherals */
	SI_vInitSystem(&huart1, &hi2c1, &hi2c2);

  /* creation of myQueue01 */
  myQueue01Handle = osMessageQueueNew (128, sizeof(Data_t), &myQueue01_attributes);

  /* creation of SendTask_Temp */
  SendTask_TempHandle = osThreadNew(vSendTask_Temp, &(xStructsToSend[1]), &SendTask_Temp_attributes);

  /* creation of SendTask_Ph */
  SendTask_PhHandle = osThreadNew(vSendTask_Ph, &(xStructsToSend[0]), &SendTask_Ph_attributes);

  /* creation of SendTask_Caudal */
  SendTask_CaudalHandle = osThreadNew(vSendTask_Caudal, &(xStructsToSend[2]), &SendTask_Caudal_attributes);

  /* creation of SendTask_Conduct */
  SendTask_ConducHandle = osThreadNew(vSendTask_Conduct, &(xStructsToSend[3]), &SendTask_Conduc_attributes);

  /* creation of SendTask_Redox */
  SendTask_RedoxHandle = osThreadNew(vSendTask_Redox, &(xStructsToSend[4]), &SendTask_Redox_attributes);

  /* creation of SendTask_Humed */
  SendTask_HumedHandle = osThreadNew(vSendTask_Humed, &(xStructsToSend[5]), &SendTask_Humed_attributes);

  /* creation of Cyclic_ErrorHan */
  Cyclic_ErrorHanHandle = osThreadNew(vCyclic_ErrorHan, NULL, &Cyclic_ErrorHan_attributes);

  /* creation of ReceivTask_HMI */
  ReceivTask_HMIHandle = osThreadNew(vReceivTask_HMI, NULL, &ReceivTask_HMI_attributes);

  /* Start scheduler */
  osKernelStart();

  while (1){
  }
}


/* USER CODE END Header_StartTask03 */
void vSendTask_Temp(void *argument)
{
	TickType_t xLastWakeTime;
	const TickType_t xDelay1000ms = pdMS_TO_TICKS( 1000 );
	BaseType_t xStatus;
	uint8_t u8QUE_TxBuf[TAM] = {0};

	xLastWakeTime = xTaskGetTickCount();
	memset(u8QUE_TxBuf,CLEAR_BUFFER_CH, TAM);

	memcpy(u8QUE_TxBuf," Temperature", TAM);

	u8QUE_TxBuf[TAM - 2] = 13;
	u8QUE_TxBuf[TAM - 1] = '\n';

	for(;;)
	{
	  //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, 1); // 0 turn ON

	  memcpy(xStructsToSend[1].u8QUE_TxBufer, u8QUE_TxBuf, TAM);

	  //xStatus = xQueueOverwrite( myQueue01Handle, argument);
	  xStatus = xQueueSendToBack( myQueue01Handle, argument, xDelay1000ms );

	  vTaskDelayUntil( &xLastWakeTime, xDelay1000ms );

	  if( xStatus != pdPASS )
	  {
	  /* The send operation could not complete because the queue was full -
	  this must be an error as the queue should never contain more than
	  one item! */
		  HAL_UART_Transmit(&huart1, (uint8_t*)"Could not send to the queue.\r\n",strlen("Could not send to the queue.\r\n"),100);
		//HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, 1); // 0 turn ON
	  }
	  else{
		  //HAL_UART_Transmit(&huart1, (uint8_t*)"Temperature was sending correctly.\r\n",strlen("Was sending correctly.\r\n"),100);
	  }
	  //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, 0); // 0 turn Off
	}
}

/* USER CODE END Header_StartTask02 */
void vSendTask_Ph(void *argument)
{
	TickType_t xLastWakeTime;
	const TickType_t xDelay386ms = pdMS_TO_TICKS( 386 ); //with 260 receives 254 (still processing), 384 works ok
	BaseType_t xStatus;
	uint8_t u8QUE_TxBuf[TAM] = {0};

	uint8_t u8I2C_RX_Buf_100[TAM] = {0}; // DATA to receive by I2C address = 100
	uint8_t u8I2C_TX_Buffer[2] = {82,13};  //Request\r

	HAL_StatusTypeDef StatusI2C = HAL_BUSY;

	//uint8_t u8I2C_RxBuf[TAM] = "1_23.289";

	xLastWakeTime = xTaskGetTickCount();
	memset(u8QUE_TxBuf,CLEAR_BUFFER_CH, TAM);

	memset(u8I2C_RX_Buf_100,CLEAR_BUFFER_CH, TAM);

	for(;;)
	{
	  //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1); // 0 turn ON
		if(HAL_I2C_Master_Transmit(&hi2c1,TMP100_ADDR,u8I2C_TX_Buffer,1,1000)!= HAL_OK)
		{
			Error_Handler();
		}

		memset(u8I2C_RX_Buf_100,CLEAR_BUFFER_CH, TAM);

		vTaskDelayUntil( &xLastWakeTime, xDelay386ms ); //here goes to block state

		StatusI2C = HAL_I2C_Master_Receive(&hi2c1 ,TMP100_ADDR, (uint8_t *)u8I2C_RX_Buf_100, 8,1000);

		if(StatusI2C == HAL_BUSY)
		{
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, 1); // turn ON
			Error_Handler();
		}
		else{
			if(StatusI2C == HAL_TIMEOUT)
			{
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, 1); // turn ON
				Error_Handler();
			}
		}
		memcpy(xStructsToSend[0].u8QUE_TxBufer, u8I2C_RX_Buf_100, TAM);

		xStatus = xQueueSendToBack( myQueue01Handle, argument, xDelay386ms );

		if( xStatus != pdPASS )
		{
		/* The send operation could not complete because the queue was full - this must be an error as
		* the queue should never contain more than one item! */
		  HAL_UART_Transmit(&huart1, (uint8_t*)"Could not send to the queue.\r\n",strlen("Could not send to the queue.\r\n"),100);
		//HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1); // 0 turn ON
		}
	  //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 0); // 0 turn Off
	}
}

void vSendTask_Conduct(void *argument)
{
	TickType_t xLastWakeTime;
	const TickType_t xDelay386ms = pdMS_TO_TICKS( 386 );
	BaseType_t xStatus;
	uint8_t u8QUE_TxBuf[TAM] = {0};

	xLastWakeTime = xTaskGetTickCount();
	memset(u8QUE_TxBuf,CLEAR_BUFFER_CH, TAM);

	memcpy(u8QUE_TxBuf," Conduct", sizeof("Conduct"));

	u8QUE_TxBuf[TAM - 2] = 13;
	u8QUE_TxBuf[TAM - 1] = '\n';

	for(;;)
	{
	  //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, 1); // 0 turn ON

	  memcpy(xStructsToSend[3].u8QUE_TxBufer, u8QUE_TxBuf, TAM);

	  xStatus = xQueueSendToBack( myQueue01Handle, argument, xDelay386ms );

	  vTaskDelayUntil( &xLastWakeTime, xDelay386ms );

	  if( xStatus != pdPASS )
	  {
	  /* The send operation could not complete because the queue was full -
	  this must be an error as the queue should never contain more than
	  one item! */
		  HAL_UART_Transmit(&huart1, (uint8_t*)"Could not send to the queue.\r\n",strlen("Could not send to the queue.\r\n"),100);
		//HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, 1); // 0 turn ON
	  }
	  else{
		  //HAL_UART_Transmit(&huart1, (uint8_t*)"Temperature was sending correctly.\r\n",strlen("Was sending correctly.\r\n"),100);
	  }
	  //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, 0); // 0 turn Off
	}
}

/* USER CODE END Header_StartTask03. Caudal, ID = 0x3 */
void vSendTask_Caudal(void *argument)
{
	TickType_t xLastWakeTime;
	const TickType_t xDelay400ms = pdMS_TO_TICKS( 400 ); //10_02_2025 with 390 goes to Error_Handler.
	BaseType_t xStatus;
	uint8_t u8QUE_TxBuf[TAM] = {0};

	uint8_t u8I2C_RX_Buf_102[TAM] = {0}; // DATA to receive by I2C address = 102
	uint8_t u8I2C_TX_Buffer[2] = {82,13};  //Request\r

	HAL_StatusTypeDef StatusI2C = HAL_BUSY;

	xLastWakeTime = xTaskGetTickCount();
	memset(u8QUE_TxBuf,CLEAR_BUFFER_CH, TAM);

	memset(u8I2C_RX_Buf_102,CLEAR_BUFFER_CH, TAM);

	//memcpy(u8QUE_TxBuf,"Caudal", sizeof("Caudal"));

	u8QUE_TxBuf[TAM - 2] = 13;
	u8QUE_TxBuf[TAM - 1] = '\n';

	for(;;)
	{
	  //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, 1); // 0 turn ON

		if(HAL_I2C_Master_Transmit(&hi2c1,TMP102_ADDR,u8I2C_TX_Buffer,1,1000)!= HAL_OK)
		{
			Error_Handler();
		}

		memset(u8I2C_RX_Buf_102,CLEAR_BUFFER_CH, TAM);

		vTaskDelayUntil( &xLastWakeTime, xDelay400ms );

		StatusI2C = HAL_I2C_Master_Receive(&hi2c1 ,TMP102_ADDR, (uint8_t *)u8I2C_RX_Buf_102, 8,1000);

		if(StatusI2C == HAL_BUSY)
		{
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, 1); // turn ON Blue
			Error_Handler();
		}
		else{
			if(StatusI2C == HAL_TIMEOUT)
			{
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, 1); // turn ON yellow
				Error_Handler();
			}
		}

		memcpy(xStructsToSend[2].u8QUE_TxBufer, u8I2C_RX_Buf_102, TAM);

		xStatus = xQueueSendToBack( myQueue01Handle, argument, xDelay400ms );

		if( xStatus != pdPASS )
		{
		/* The send operation could not complete because the queue was full - this must be an error as
		 * the queue should never contain more than one item! */
		  HAL_UART_Transmit(&huart1, (uint8_t*)"Could not send to the queue.\r\n",strlen("Could not send to the queue.\r\n"),100);
		//HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, 1); // 0 turn ON
		}
		else{
		  //HAL_UART_Transmit(&huart1, (uint8_t*)"Caudal was sending correctly.\r\n",strlen("Was sending correctly.\r\n"),100);
		}
	  //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, 0); // 0 turn Off
	}
}


void vSendTask_Redox(void *argument)
{
	TickType_t xLastWakeTime;
	const TickType_t xDelay386ms = pdMS_TO_TICKS( 392 );
	BaseType_t xStatus;
	uint8_t u8QUE_TxBuf[TAM] = {0};

	xLastWakeTime = xTaskGetTickCount();
	memset(u8QUE_TxBuf,CLEAR_BUFFER_CH, TAM);

	memcpy(u8QUE_TxBuf," Redox", sizeof("Redox"));

	u8QUE_TxBuf[TAM - 2] = 13;
	u8QUE_TxBuf[TAM - 1] = '\n';

	for(;;)
	{
	  //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, 1); // 0 turn ON

	  memcpy(xStructsToSend[4].u8QUE_TxBufer, u8QUE_TxBuf, TAM);

	  xStatus = xQueueSendToBack( myQueue01Handle, argument, xDelay386ms );

	  vTaskDelayUntil( &xLastWakeTime, xDelay386ms );

	  if( xStatus != pdPASS )
	  {
	  /* The send operation could not complete because the queue was full -
	  this must be an error as the queue should never contain more than
	  one item! */
		  HAL_UART_Transmit(&huart1, (uint8_t*)"Could not send to the queue.\r\n",strlen("Could not send to the queue.\r\n"),100);
		//HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, 1); // 0 turn ON
	  }
	  else{
		  //HAL_UART_Transmit(&huart1, (uint8_t*)"Temperature was sending correctly.\r\n",strlen("Was sending correctly.\r\n"),100);
	  }
	  //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, 0); // 0 turn Off
	}
}

void vSendTask_Humed(void *argument)
{
	TickType_t xLastWakeTime;
	const TickType_t xDelay400ms = pdMS_TO_TICKS( 400 );
	BaseType_t xStatus;
	uint8_t u8QUE_TxBuf[TAM] = {0};

	xLastWakeTime = xTaskGetTickCount();
	memset(u8QUE_TxBuf,CLEAR_BUFFER_CH, TAM);

	memcpy(u8QUE_TxBuf," Humidity", sizeof("Humidity"));

	u8QUE_TxBuf[TAM - 2] = 13;
	u8QUE_TxBuf[TAM - 1] = '\n';

	for(;;)
	{
	  //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, 1); // 0 turn ON

	  memcpy(xStructsToSend[5].u8QUE_TxBufer, u8QUE_TxBuf, TAM);

	  xStatus = xQueueSendToBack( myQueue01Handle, argument, xDelay400ms );

	  vTaskDelayUntil( &xLastWakeTime, xDelay400ms );

	  if( xStatus != pdPASS )
	  {
	  /* The send operation could not complete because the queue was full -
	  this must be an error as the queue should never contain more than
	  one item! */
		  HAL_UART_Transmit(&huart1, (uint8_t*)"Could not send to the queue.\r\n",strlen("Could not send to the queue.\r\n"),100);
		//HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, 1); // 0 turn ON
	  }
	  else{
		  //HAL_UART_Transmit(&huart1, (uint8_t*)"Temperature was sending correctly.\r\n",strlen("Was sending correctly.\r\n"),100);
	  }
	  //HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, 0); // 0 turn Off
	}
}


void vReceivTask_HMI(void *argument)
{
	Data_t xReceivedStructure;
	const TickType_t xDelay320ms = pdMS_TO_TICKS( 320 ); //180 was ok but get empty once
	BaseType_t xStatus;
	BaseType_t NumDataQueue = 0;
	uint8_t u8QUE_RxBuf[TAM] = {0};

	memset(u8QUE_RxBuf,CLEAR_BUFFER_CH, TAM);

	for(;;)
	{
	  NumDataQueue = uxQueueMessagesWaiting( myQueue01Handle );
	  if( NumDataQueue > 128 )
	  {
		  HAL_UART_Transmit(&huart1, (uint8_t*)"Queue should have been empty!\r\n",strlen("Queue should have been empty!\r\n"),100);
	  }
	  //vTaskDelayUntil( &xLastWakeTime, xDelay120ms ); //Goes to blocked state, but GPIO keeps ON
													  //GPIO goes to off until pass 120ms
	  //xStatus = xQueueReceive( myQueue01Handle, &xReceivedStructure, 0 );
	  xStatus = xQueueReceive( myQueue01Handle, &xReceivedStructure, xDelay320ms );

	  if( xStatus == pdPASS )
	  {
		  /* Data was successfully received from the queue, print out the received value. */
		  u8QUE_RxBuf[TAM - 2] = 13;
		  u8QUE_RxBuf[TAM - 1] = '\n';

		  switch(xReceivedStructure.eDataSource)
		  {
		  	  case eSender1_Ph:
				  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 1); // 0 turn ON

				  HAL_UART_Transmit(&huart1, (uint8_t*)"\n\r From Sender 1 Ph = ",strlen("\n\r From Sender 1 Ph = "),200);

				  if(HAL_UART_Transmit(&huart1, (uint8_t *)xReceivedStructure.u8QUE_TxBufer, sizeof(xReceivedStructure.u8QUE_TxBufer), 200)!= HAL_OK)
				  {
					Error_Handler();
				  }
				  break;

		  	  case eSender2_Temp:
				  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, 1); // 0 turn ON
				  HAL_UART_Transmit(&huart1, (uint8_t*)"\n\r From Sender 2 Temperature = ",strlen("\n\r From Sender 2 Temperature = "),200);

				  if(HAL_UART_Transmit(&huart1, (uint8_t *)xReceivedStructure.u8QUE_TxBufer, sizeof(xReceivedStructure.u8QUE_TxBufer), 200)!= HAL_OK)
				  {
					Error_Handler();
				  }
				  break;

		  	  case eSender3_Caudal:
				  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, 1); // 0 turn ON
				  HAL_UART_Transmit(&huart1, (uint8_t*)"\n\r From Sender 3 Caudal = ",strlen("\n\r From Sender 3 Caudal = "),200);

				  if(HAL_UART_Transmit(&huart1, (uint8_t *)xReceivedStructure.u8QUE_TxBufer, sizeof(xReceivedStructure.u8QUE_TxBufer), 200)!= HAL_OK)
				  {
					Error_Handler();
				  }
				  break;

		  	  case eSender4_Conduct:
				  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 1); // 0 turn ON
				  HAL_UART_Transmit(&huart1, (uint8_t*)"\n\r From Sender 4 Conduct = ",strlen("\n\r From Sender 4 Conduct = "),200);

				  if(HAL_UART_Transmit(&huart1, (uint8_t *)xReceivedStructure.u8QUE_TxBufer, sizeof(xReceivedStructure.u8QUE_TxBufer), 200)!= HAL_OK)
				  {
					Error_Handler();
				  }
				  break;

		  	  case eSender5_Redox:
				  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 1); // 0 turn ON
				  HAL_UART_Transmit(&huart1, (uint8_t*)"\n\r From Sender 5 Redox = ",strlen("\n\r From Sender 5 Redox = "),200);

				  if(HAL_UART_Transmit(&huart1, (uint8_t *)xReceivedStructure.u8QUE_TxBufer, sizeof(xReceivedStructure.u8QUE_TxBufer), 200)!= HAL_OK)
				  {
					Error_Handler();
				  }
				  break;

		  	  case eSender6_Humed:
		  		  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 1); // 0 turn ON
				  HAL_UART_Transmit(&huart1, (uint8_t*)"\n\r From Sender 6 Humidity = ",strlen("\n\r From Sender 6 Humidity = "),200);

				  if(HAL_UART_Transmit(&huart1, (uint8_t *)xReceivedStructure.u8QUE_TxBufer, sizeof(xReceivedStructure.u8QUE_TxBufer), 200)!= HAL_OK)
				  {
					Error_Handler();
				  }
				  break;

		  	  default:
		  		HAL_UART_Transmit(&huart1, (uint8_t*)"Could not receive from the queue.\r\n",strlen("Could not receive from the queue.\r\n"),100);
		  		break;
		  }

	  }
	  else{
	  /* Data was not received from the queue even after waiting for 100ms.
	  This must be an error as the sending tasks are free running and will be
	  continuously writing to the queue. */
		  HAL_UART_Transmit(&huart1, (uint8_t*)"Could not receive from the queue.\r\n",strlen("Could not receive from the queue.\r\n"),100);
	  }

	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, 0); // 0 turn Off

	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 0); // 0 turn OFF
	  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, 0); // 0 turn OFF

	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0); // 0 turn OFF
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0); // 0 turn OFF
	  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 0); // 0 turn OFF

	  memset(u8QUE_RxBuf,CLEAR_BUFFER_CH, TAM);
	}
}

void vCyclic_ErrorHan(void *argument)
{
	TickType_t xLastWakeTime;
	const TickType_t xDelay4s = pdMS_TO_TICKS( 4000 );

	xLastWakeTime = xTaskGetTickCount();

	for(;;)
	{
		HAL_UART_Transmit(&huart1, (uint8_t*)"\n\r Error Handler task (4s)\r\n",strlen("\n\r Error Handler task (4s)\r\n"),100);
		vTaskDelayUntil( &xLastWakeTime, xDelay4s );
	}
}


void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
	 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_15, 0); // 0 turn Off

	 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 0); // 0 turn OFF
	 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_14, 0); // 0 turn OFF

	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, 0); // 0 turn OFF
	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, 0); // 0 turn OFF
	 HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, 0); // 0 turn OFF
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
