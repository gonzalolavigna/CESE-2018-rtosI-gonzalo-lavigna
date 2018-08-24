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

static messageToLogger_t messageToLoggerFromLedUpdate;


typedef struct {
	ledState_t 		state;
	TickType_t 		remainningTicks;
	TickType_t		lastTickCount;
	gpioMap_t 		led;
} ledInfo_t;

ledInfo_t ledArray[NUMBER_OF_LEDS];

void initLed(void){

	messageToLoggerFromLedUpdate.origin = LED_TASK;

	ledArray[0].remainningTicks = 0;
	ledArray[0].state = LED_NOT_SPARKING;
	ledArray[0].led = LEDB;
	ledArray[0].lastTickCount = 0;

	ledArray[1].remainningTicks = 0;
	ledArray[1].state = LED_NOT_SPARKING;
	ledArray[1].led = LED1;
	ledArray[1].lastTickCount = 0;

	ledArray[2].remainningTicks = 0;
	ledArray[2].state = LED_NOT_SPARKING;
	ledArray[2].led = LED2;
	ledArray[2].lastTickCount = 0;

	//Only for Debug
	gpioInit(GPIO5,GPIO_OUTPUT);
	gpioInit(GPIO6,GPIO_OUTPUT);

	gpioWrite(GPIO5,OFF);
	gpioWrite(GPIO6,OFF);

}

void ledUpdate(void* taskParmPtr){
	messageToLedUpdate_t messageFromQueue;
	BaseType_t queueReturn = pdFALSE;

	while(1){
		//Tener en cuenta que no me llevo el elemento de la cola
		queueReturn = xQueueReceive(queueTasksToLedUpdate,&messageFromQueue,DELAY_TASK_LED_UPDATE);
		gpioToggle(GPIO5);
		for(int i=0;i<NUMBER_OF_LEDS;i++){
			switch(ledArray[i].state){
			case LED_SPARKING:
				if(queueReturn == pdTRUE){
					if(messageFromQueue.ledToSpark != i){ //Como el mensaje no va dirigido al led en cuestion indexado directamente decremento para ver que no hayan incovenientes
						if(ledArray[i].lastTickCount + ledArray[i].remainningTicks < xTaskGetTickCount()){ // Si es menor al tiempo actual me pase de cuenta
							ledArray[i].state = LED_NOT_SPARKING;
							gpioWrite(ledArray[i].led,OFF);
							//Only for debug
							gpioWrite(GPIO6,OFF);
						} //Sino llegue al valor directamente no hago nada y espero para volver a preguntar
					}
					else{
						//Esto seria un repeated start
						ledArray[i].lastTickCount = xTaskGetTickCount();
						ledArray[i].remainningTicks = messageFromQueue.ledSparkTime;
						ledArray[i].state = LED_NOT_SPARKING;
						gpioWrite(ledArray[i].led,OFF);
						//Only for debug
						gpioWrite(GPIO6,OFF);
						vTaskDelay(50/portTICK_RATE_MS);
						xQueueSend(queueTasksToLedUpdate,&messageFromQueue,0);

					}
				}
				else{
					if(ledArray[i].lastTickCount + ledArray[i].remainningTicks < xTaskGetTickCount()){ // Si es menor al tiempo actual me pase de cuenta
						ledArray[i].state = LED_NOT_SPARKING;
						gpioWrite(ledArray[i].led,OFF);
						//Only for debug
						gpioWrite(GPIO6,OFF);
					} //Sino llegue al valor directamente no hago nada y espero para volver a preguntar
				}
				break;
			case LED_NOT_SPARKING:
				if(queueReturn == pdTRUE){
					if(messageFromQueue.ledToSpark == i){
						ledArray[i].lastTickCount = xTaskGetTickCount();
						ledArray[i].remainningTicks = messageFromQueue.ledSparkTime-DELAY_TASK_LED_UPDATE;
						ledArray[i].state = LED_SPARKING;
						gpioWrite(ledArray[i].led,ON);
						//Only for debug
						gpioWrite(GPIO6,ON);
					}
				}
				break;
			default:
				printf("ERROR IN LED UPDATE\r\n");
			}
		}
		gpioToggle(GPIO5);
	}
}

