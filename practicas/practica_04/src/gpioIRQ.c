/*==================[inlcusiones]============================================*/

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "string.h"

// sAPI header
#include "sapi.h"

//Funciones correspondientes a interrupcion
#include "gpioIRQ.h"
#include "edgeDetector.h"
#include "FreeRTOSConfig.h"
#include "ledUpdate.h"
#include "logger.h"
#include "debounceTec.h"

static messageToLogger_t messageToLoggerFromIRQs;

static void serveGPIO_IRQ (teclaIndex_t tecla, uint8_t edge);

void GPIO0_IRQHandler(void);
void GPIO1_IRQHandler(void);
void GPIO2_IRQHandler(void);
void GPIO3_IRQHandler(void);

void initIRQ(void){
	messageToLoggerFromIRQs.origin = IRQs;

	//Solo para debuggin
	gpioInit(GPIO0,GPIO_OUTPUT);
	gpioInit(GPIO1,GPIO_OUTPUT);
	gpioWrite(GPIO0,ON);
    gpioWrite(GPIO1,ON);
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

	//Configuro prioridad de interrupciones
	NVIC_SetPriority(PIN_INT0_IRQn,0xFFFFFFFF);
	NVIC_SetPriority(PIN_INT1_IRQn,0xFFFFFFFF);
	NVIC_SetPriority(PIN_INT2_IRQn,0xFFFFFFFF);
	NVIC_SetPriority(PIN_INT3_IRQn,0xFFFFFFFF);

	NVIC_ClearPendingIRQ( PIN_INT0_IRQn );
    NVIC_ClearPendingIRQ( PIN_INT1_IRQn );
	NVIC_ClearPendingIRQ( PIN_INT2_IRQn );
    NVIC_ClearPendingIRQ( PIN_INT3_IRQn );

    //Habilito interrupciones
    NVIC_EnableIRQ( PIN_INT0_IRQn );
    NVIC_EnableIRQ( PIN_INT1_IRQn );
    NVIC_EnableIRQ( PIN_INT2_IRQn );
    NVIC_EnableIRQ( PIN_INT3_IRQn );
}

void GPIO0_IRQHandler(void){
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(0));

	//Only for debugging and testing logger
	static BaseType_t xHigherPriorityTaskWoken= pdFALSE;
	sprintf(messageToLoggerFromIRQs.messageToPrint,"FALLING EDGE TEC1");
	messageToLoggerFromIRQs.messageTick = xTaskGetTickCount();
	xQueueSendFromISR(queueTasksToLogger,(void *)&messageToLoggerFromIRQs,&xHigherPriorityTaskWoken);

	gpioWrite(GPIO0,OFF);
	serveGPIO_IRQ(TEC1_INDICE,FALLING_EDGE);
}

void GPIO1_IRQHandler(void){

	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(1));

	//Only for debugging and testing logger
	static BaseType_t xHigherPriorityTaskWoken= pdFALSE;
	sprintf(messageToLoggerFromIRQs.messageToPrint,"RISING EDGE TEC1");
	messageToLoggerFromIRQs.messageTick = xTaskGetTickCount();
	xQueueSendFromISR(queueTasksToLogger,(void *)&messageToLoggerFromIRQs,&xHigherPriorityTaskWoken);

	gpioWrite(GPIO0,ON);
	serveGPIO_IRQ(TEC1_INDICE,RISING_EDGE);
}

void GPIO2_IRQHandler(void){
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(2));

	//Only for debugging and testing logger
	static BaseType_t xHigherPriorityTaskWoken= pdFALSE;
	sprintf(messageToLoggerFromIRQs.messageToPrint,"FALLING EDGE TEC2");
	messageToLoggerFromIRQs.messageTick = xTaskGetTickCount();
	xQueueSendFromISR(queueTasksToLogger,(void *)&messageToLoggerFromIRQs,&xHigherPriorityTaskWoken);

	gpioWrite(GPIO1,OFF);
	serveGPIO_IRQ(TEC2_INDICE,FALLING_EDGE);
}

void GPIO3_IRQHandler(void){


	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH(3));

	//Only for debugging and testing logger
	static BaseType_t xHigherPriorityTaskWoken= pdFALSE;
	sprintf(messageToLoggerFromIRQs.messageToPrint,"RISING EDGE TEC2");
	messageToLoggerFromIRQs.messageTick = xTaskGetTickCount();
	xQueueSendFromISR(queueTasksToLogger,(void *)&messageToLoggerFromIRQs,&xHigherPriorityTaskWoken);

	gpioWrite(GPIO1,ON);
	serveGPIO_IRQ(TEC2_INDICE,RISING_EDGE);
}

static void serveGPIO_IRQ (teclaIndex_t tecla, uint8_t edge){
	static messageQueueIRQToDebounceTec_t messageToQueue;
	static BaseType_t xHigherPriorityTaskWoken= pdFALSE;

	messageToQueue.irqTickTime = xTaskGetTickCount();
	messageToQueue.tecla = tecla;
	messageToQueue.flancoTecla = edge;
	xQueueSendFromISR(queueIRQToDebounceTec,(void *)&messageToQueue,&xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
