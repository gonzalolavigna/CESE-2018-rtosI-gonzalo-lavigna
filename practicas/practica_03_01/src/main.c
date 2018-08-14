/* Copyright 2017-2018, Eric Pernia
 * All rights reserved.
 *
 * This file is part of sAPI Library.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*==================[inlcusiones]============================================*/

// Includes de FreeRTOS
#include "../../practica_03_01/inc/FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "string.h"

// sAPI header
#include "sapi.h"


/*==================[definiciones y macros]==================================*/
#define RISING_EDGE 0
#define FALLING_EDGE 1

typedef struct {
	TickType_t 	eventTickTime;
	gpioMap_t	tecla;
	uint8_t 	flancoTecla;
} messageToDebounceTec_t;


typedef enum {BUTTON_DOWN = 0,  BUTTON_UP} buttonState_t;

typedef struct {
	buttonState_t 	state;
	TickType_t 		lastTickCount;
} debounceState_t;

debounceState_t buttonArray[4];


typedef enum {LED_ON = 0, LED_OFF} ledState_t;

typedef struct {
	gpioMap_t 		led;
	ledState_t 	 	state;
	TickType_t 		count_time;
	TickType_t 		actual_toggle_time;
	TickType_t 		next_toggle_time;
	bool_t 			new_configuration;
}ledFSMState_t;

ledFSMState_t ledArray[4];


/*==================[definiciones de datos internos]=========================*/

QueueHandle_t queueISRToDebounceTec;

/*==================[definiciones de datos externos]=========================*/

DEBUG_PRINT_ENABLE;

/*==================[declaraciones de funciones internas]====================*/
void initIRQ(void);
void debounceTec(void* taskParmPtr);
void debounceTecInit(void);

void ledUpdate(void *taskParmPtr);
void ledInit(void);

uint8_t getTeclaIndice(messageToDebounceTec_t meesage);
void getEdgePrintMessage(messageToDebounceTec_t message,uint8_t * buffer);

void serveGPIO_IRQ (gpioMap_t tecla, uint8_t edge);

/*==================[declaraciones de funciones externas]====================*/

// Prototipo de funcion de la tarea

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main(void)
{
   // ---------- CONFIGURACIONES ------------------------------
   // Inicializar y configurar la plataforma
   boardConfig();
   // UART for debug messages
   debugPrintConfigUart( UART_USB, 115200 );
   debugPrintlnString( "IRQs in RTOS\r\n" );
   gpioInit(GPIO0,GPIO_OUTPUT);
   gpioInit(GPIO1,GPIO_OUTPUT);

   queueISRToDebounceTec = xQueueCreate(10,sizeof(messageToDebounceTec_t));
   if(queueISRToDebounceTec == NULL)
	   uartWriteString(UART_USB,"COLA NO CREADA\r\n");

   debounceTecInit();
   ledInit();
   initIRQ();

   // Crear tarea en freeRTOS
   xTaskCreate(
		   debounceTec,                     // Funcion de la tarea a ejecutar
		   (const char *)"debounceTec",     // Nombre de la tarea como String amigable para el usuario
		   configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
		   0,                          // Parametros de tarea
		   tskIDLE_PRIORITY+1,         // Prioridad de la tarea
		   0                           // Puntero a la tarea creada en el sistema
   );

   // Crear tarea en freeRTOS
   xTaskCreate(
		   ledUpdate,                     // Funcion de la tarea a ejecutar
		   (const char *)"debounceTec",     // Nombre de la tarea como String amigable para el usuario
		   configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
		   0,                          // Parametros de tarea
		   tskIDLE_PRIORITY+2,         // Prioridad de la tarea
		   0                           // Puntero a la tarea creada en el sistema
   );

   vTaskStartScheduler();

   uartWriteString(UART_USB,"PANIC\r\n");

   // ---------- REPETIR POR SIEMPRE --------------------------
   while( TRUE ) {
      // Si cae en este while 1 significa que no pudo iniciar el scheduler
   }


   // NO DEBE LLEGAR NUNCA AQUI, debido a que a este programa se ejecuta
   // directamenteno sobre un microcontroladore y no es llamado por ningun
   // Sistema Operativo, como en el caso de un programa para PC.
   return 0;
}

/*==================[definiciones de funciones internas]=====================*/

void initIRQ(void){

	//TEC 1 IRQs
	Chip_SCU_GPIOIntPinSel(0,0,4);
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(0));
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT,PININTCH(0));
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT,PININTCH(0));

	Chip_SCU_GPIOIntPinSel(1,0,4);
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(1));
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT,PININTCH(1));
	Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT,PININTCH(1));

	//TEC 2 IRQs
	Chip_SCU_GPIOIntPinSel(2,0,8);
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(2));
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT,PININTCH(2));
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT,PININTCH(2));

	Chip_SCU_GPIOIntPinSel(3,0,8);
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(3));
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT,PININTCH(3));
	Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT,PININTCH(3));

	//TEC 3 IRQs
	Chip_SCU_GPIOIntPinSel(4,0,9);
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(4));
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT,PININTCH(4));
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT,PININTCH(4));

	Chip_SCU_GPIOIntPinSel(5,0,9);
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(5));
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT,PININTCH(5));
	Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT,PININTCH(5));

	//TEC 4 IRQs
	Chip_SCU_GPIOIntPinSel(6,1,9);
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(6));
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT,PININTCH(6));
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT,PININTCH(6));

	Chip_SCU_GPIOIntPinSel(7,1,9);
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(7));
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT,PININTCH(7));
	Chip_PININT_EnableIntHigh(LPC_GPIO_PIN_INT,PININTCH(7));


	NVIC_SetPriority(PIN_INT0_IRQn,0xFFFFFFFF);
	NVIC_SetPriority(PIN_INT1_IRQn,0xFFFFFFFF);
	NVIC_SetPriority(PIN_INT2_IRQn,0xFFFFFFFF);
	NVIC_SetPriority(PIN_INT3_IRQn,0xFFFFFFFF);
	NVIC_SetPriority(PIN_INT4_IRQn,0xFFFFFFFF);
	NVIC_SetPriority(PIN_INT5_IRQn,0xFFFFFFFF);
	NVIC_SetPriority(PIN_INT6_IRQn,0xFFFFFFFF);
	NVIC_SetPriority(PIN_INT7_IRQn,0xFFFFFFFF);

	NVIC_ClearPendingIRQ( PIN_INT0_IRQn );
    NVIC_ClearPendingIRQ( PIN_INT1_IRQn );
	NVIC_ClearPendingIRQ( PIN_INT2_IRQn );
    NVIC_ClearPendingIRQ( PIN_INT3_IRQn );
	NVIC_ClearPendingIRQ( PIN_INT4_IRQn );
    NVIC_ClearPendingIRQ( PIN_INT5_IRQn );
	NVIC_ClearPendingIRQ( PIN_INT6_IRQn );
    NVIC_ClearPendingIRQ( PIN_INT7_IRQn );

    NVIC_EnableIRQ( PIN_INT0_IRQn );
    NVIC_EnableIRQ( PIN_INT1_IRQn );
    NVIC_EnableIRQ( PIN_INT2_IRQn );
    NVIC_EnableIRQ( PIN_INT3_IRQn );
    NVIC_EnableIRQ( PIN_INT4_IRQn );
    NVIC_EnableIRQ( PIN_INT5_IRQn );
    NVIC_EnableIRQ( PIN_INT6_IRQn );
    NVIC_EnableIRQ( PIN_INT7_IRQn );
}

void GPIO0_IRQHandler(void){
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(0));
	serveGPIO_IRQ(TEC1,FALLING_EDGE);
}

void GPIO1_IRQHandler(void){
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(1));
	serveGPIO_IRQ(TEC1,RISING_EDGE);
}

void GPIO2_IRQHandler(void){
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(2));
	serveGPIO_IRQ(TEC2,FALLING_EDGE);
}

void GPIO3_IRQHandler(void){
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(3));
	serveGPIO_IRQ(TEC2,RISING_EDGE);
}

void GPIO4_IRQHandler(void){
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(4));
	serveGPIO_IRQ(TEC3,FALLING_EDGE);
}

void GPIO5_IRQHandler(void){
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(5));
	serveGPIO_IRQ(TEC3,RISING_EDGE);
}

void GPIO6_IRQHandler(void){
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(6));
	serveGPIO_IRQ(TEC4,FALLING_EDGE);
}

void GPIO7_IRQHandler(void){
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(7));
	serveGPIO_IRQ(TEC4,RISING_EDGE);
}


void serveGPIO_IRQ (gpioMap_t tecla, uint8_t edge){
	static messageToDebounceTec_t messageToQueue;
	static BaseType_t xHigherPriorityTaskWoken= pdFALSE;

	gpioToggle(GPIO0);
	messageToQueue.eventTickTime = xTaskGetTickCount();
	messageToQueue.tecla = tecla;
	messageToQueue.flancoTecla = edge;
	xQueueSendFromISR(queueISRToDebounceTec,(void *)&messageToQueue,&xHigherPriorityTaskWoken);
	gpioToggle(GPIO0);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void debounceTecInit(void){
	buttonArray[0].state = BUTTON_UP;
	buttonArray[0].lastTickCount = 0;
	buttonArray[1].state = BUTTON_UP;
	buttonArray[1].lastTickCount = 0;
	buttonArray[2].state = BUTTON_UP;
	buttonArray[2].lastTickCount = 0;
	buttonArray[3].state = BUTTON_UP;
	buttonArray[3].lastTickCount = 0;
}


void debounceTec(void* taskParmPtr){
	messageToDebounceTec_t messageFromQueue;
	uint8_t print_buffer[20];
	uint8_t tecla_indice;
	while(1){
		if(xQueueReceive(queueISRToDebounceTec,&messageFromQueue,portMAX_DELAY) == pdTRUE){
			tecla_indice = getTeclaIndice(messageFromQueue);
			switch( buttonArray[tecla_indice].state){
			case BUTTON_UP:
				if(messageFromQueue.flancoTecla == FALLING_EDGE){
					if((messageFromQueue.eventTickTime - buttonArray[tecla_indice].lastTickCount) > (40/portTICK_RATE_MS) )
						printf("EVENTO TEC%d LIBERADA por un tiempo de %d TICKS\r\n",tecla_indice+1,messageFromQueue.eventTickTime - buttonArray[tecla_indice].lastTickCount);

						buttonArray[tecla_indice].state = BUTTON_DOWN;
						buttonArray[tecla_indice].lastTickCount = messageFromQueue.eventTickTime;
				}
				break;
			case BUTTON_DOWN:
				if(messageFromQueue.flancoTecla == RISING_EDGE){
					if((messageFromQueue.eventTickTime - buttonArray[tecla_indice].lastTickCount) > (40/portTICK_RATE_MS) )
						printf("EVENTO TEC%d PULSADA por un tiempo de %d TICKS\r\n",tecla_indice,messageFromQueue.eventTickTime - buttonArray[tecla_indice].lastTickCount);

						buttonArray[tecla_indice].state = BUTTON_UP;
						buttonArray[tecla_indice].lastTickCount = messageFromQueue.eventTickTime;
				}
				break;
			default:
				uartWriteString(UART_USB,"DANGER WILL ROBINSON");
			}
		}
		else{
			uartWriteString(UART_USB,"DANGER WILL ROBINSON");
		}
	}
}

uint8_t getTeclaIndice(messageToDebounceTec_t message){
	uint8_t indice;
	switch(message.tecla){
		case TEC1:
			indice = 0;
			break;
		case TEC2:
			indice =  1;
			break;
		case TEC3:
			indice =  2;
			break;
		case TEC4:
			indice =  3;
			break;
		default:
			indice =  4;
	}
	return indice;
}

void getEdgePrintMessage(messageToDebounceTec_t message,uint8_t * buffer){
	if (message.flancoTecla == RISING_EDGE)
		strncpy(buffer,"RISING",sizeof("RISING"));
	else if(message.flancoTecla == FALLING_EDGE)
		strncpy(buffer,"FALLING",sizeof("FALLING"));
	else strncpy(buffer,"DANGER WILL ROBINSON",sizeof("DANGER WILL ROBINSON"));
}


void ledInit(void){
	ledArray[0].actual_toggle_time = 0;
	ledArray[0].count_time = 0;
	ledArray[0].led = LEDB;
	ledArray[0].new_configuration = FALSE;
	ledArray[0].next_toggle_time = 0;
	ledArray[0].state = LED_OFF;

	ledArray[0].actual_toggle_time = 0;
	ledArray[0].count_time = 0;
	ledArray[0].led = LED1;
	ledArray[0].new_configuration = FALSE;
	ledArray[0].next_toggle_time = 0;
	ledArray[0].state = LED_OFF;

	ledArray[0].actual_toggle_time = 0;
	ledArray[0].count_time = 0;
	ledArray[0].led = LED2;
	ledArray[0].new_configuration = FALSE;
	ledArray[0].next_toggle_time = 0;
	ledArray[0].state = LED_OFF;

	ledArray[0].actual_toggle_time = 0;
	ledArray[0].count_time = 0;
	ledArray[0].led = LED3;
	ledArray[0].new_configuration = FALSE;
	ledArray[0].next_toggle_time = 0;
	ledArray[0].state = LED_OFF;
}

/*==================[definiciones de funciones externas]=====================*/

/*==================[fin del archivo]========================================*/
