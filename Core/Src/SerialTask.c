#include "main.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "queue.h"
extern UART_HandleTypeDef huart1;


SemaphoreHandle_t xSemaphore;
char buf[1];
QueueHandle_t xQueue;
TaskHandle_t handleSerialRecieveByte;
char data;


QueueHandle_t xQueue;
void CreateSerialObjects(){
	xQueue = xQueueCreate(16, sizeof(buf));
	HAL_UART_Receive_IT(&huart1, buf, sizeof(buf));


}

void HAL_UART_RxCptHandler(){
	BaseType_t xStatus ;
	static BaseType_t pxHigherPriorityTaskWoken = pdFALSE;
	xStatus = xQueueSendFromISR(xQueue, buf, &pxHigherPriorityTaskWoken);
	HAL_UART_Receive_IT(&huart1, buf, sizeof(buf));
}


void task(){
	while(1){
		BaseType_t xStatus ;
		xStatus = xQueueReceive(xQueue, buf, portMAX_DELAY);
		//char c = USART_ReceiveData(USART1);
		printf("%c", buf[0]);

	}
}

void CreateTask(){
	xTaskCreate(task, "Tarea", 256, NULL, 1, NULL);
}

void SerialSendByte(char data){
	//Esperar semáforo:
	BaseType_t status = xSemaphoreTake(xSemaphore, portMAX_DELAY);
	//Envío del byte
	HAL_UART_Transmit_IT(&huart1, &data, 1);
}

void SerialRecieveByte(){
	int i = 0;
	HAL_UART_Receive_IT(&huart1, &data, 1);
	for(;;){
		xQueueReceive(xQueue,&data,portMAX_DELAY);
		buf[i] = data;
		SerialSendByte(buf[i]);
		if(i >= 5){
			i = 0;
		}

	}
}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef* huart){
	static BaseType_t xHigherPriorityTaskWoken;
	xSemaphoreGiveFromISR(xSemaphore, pdFALSE);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


int __io_putchar(int ch){
	SerialSendByte(ch);
	return ch;
}




