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

static messageToLogger_t messageToLoggerFromDebounceTec;
static messageToLedUpdate_t messageToLedUpdate;



typedef struct {
	buttonState_t 	state;
	TickType_t 		lastTickCount;
} debounceState_t;
debounceState_t buttonArray[2];


void debounceTecInit(void){
	buttonArray[0].state = BUTTON_UP;
	buttonArray[0].lastTickCount = 0;
	buttonArray[1].state = BUTTON_UP;
	buttonArray[1].lastTickCount = 0;

	//Only for debugging
	gpioInit(GPIO2,GPIO_OUTPUT);
	gpioInit(GPIO3,GPIO_OUTPUT);
	gpioWrite(GPIO2,OFF);
	gpioWrite(GPIO3,OFF);

	messageToLoggerFromDebounceTec.origin = TECLA_TASK;

}

void debounceTec(void* taskParmPtr){
	messageQueueIRQToDebounceTec_t messageFromQueue;
	BaseType_t queueReturn = pdFALSE;
	TickType_t delayDebouncer = portMAX_DELAY;
	while(1){
		queueReturn = xQueueReceive(queueIRQToDebounceTec,&messageFromQueue,10/portTICK_RATE_MS);
		gpioToggle(GPIO2);
		for(int i = 0; i < 2; i++){//Recorro todas las teclas no importa
			switch(buttonArray[i].state){
			case BUTTON_UP:
				if(queueReturn == pdTRUE){//Solo voy a falling si el evento es de mi tecla y si es descendente otra cosa la ignoro
					if(messageFromQueue.flancoTecla == FALLING_EDGE && messageFromQueue.tecla == i){
						buttonArray[messageFromQueue.tecla].state = BUTTON_FALLING;
						buttonArray[messageFromQueue.tecla].lastTickCount = messageFromQueue.irqTickTime;
					}
				}
				break;
			case BUTTON_FALLING:
				if(queueReturn == pdTRUE){
					if(messageFromQueue.tecla == i){
						buttonArray[messageFromQueue.tecla].state = BUTTON_UP;
						buttonArray[messageFromQueue.tecla].lastTickCount = messageFromQueue.irqTickTime;
					}
				}
				else { //Si no tengo pdTrue es que paso el tiempo de los 10 ms sin tener nada
					gpioToggle(GPIO3);
					buttonArray[i].state = BUTTON_DOWN;

					//ONly for debuggin y testear el logger
					///TODO: Unificar en una sola funcion
					sprintf(messageToLoggerFromDebounceTec.messageToPrint,"TEC%d FALLING EDGE TICK TIME:%d",i+1,buttonArray[i].lastTickCount);
					messageToLoggerFromDebounceTec.messageTick = xTaskGetTickCount();
					if(xQueueSend(queueTasksToLogger,&messageToLoggerFromDebounceTec,0) != pdTRUE){
		                printf("COLA DEL LOGGER LLENA\r\n");
		            }

					//TODO:Unificar en una sola funcion
					//MEnsaje para el led de activacion de un flanco de un led
					messageToLedUpdate.ledSparkTime = 200 / portTICK_RATE_MS;//200 ms en ticks
					//TODO: Hacer algo mas seguro que esto y tambien mas escalable
					messageToLedUpdate.ledToSpark = i; //le paso el indice de la correspondiente tecla por la cola de mensaje
					if(xQueueSend(queueTasksToLedUpdate,&messageToLedUpdate,0) != pdTRUE){
		                printf("COLA LED LLENA\r\n");
					}


					//Generar mensaje de tecla que se le hizo el debouncer
					gpioToggle(GPIO3);
				}
				break;
			case BUTTON_DOWN:
				if(queueReturn == pdTRUE){
					if(messageFromQueue.flancoTecla == RISING_EDGE && messageFromQueue.tecla == i){
						buttonArray[messageFromQueue.tecla].state = BUTTON_RAISING;
						buttonArray[messageFromQueue.tecla].lastTickCount = messageFromQueue.irqTickTime;
					}
				}
				break;
			case BUTTON_RAISING:
				if(queueReturn == pdTRUE){
					if(messageFromQueue.tecla == i){
						buttonArray[messageFromQueue.tecla].state = BUTTON_DOWN;
						buttonArray[messageFromQueue.tecla].lastTickCount = messageFromQueue.irqTickTime;
					}
				}
				else { //Si no tengo pdTrue es que paso el tiempo de los 10 ms sin tener nada
					gpioToggle(GPIO3);
					buttonArray[i].state = BUTTON_UP;

					//ONly for debuggin y testear el logger
					///TODO: Unificar en una sola funcion
					sprintf(messageToLoggerFromDebounceTec.messageToPrint,"TEC%d RISING EDGE TICK TIME:%d",i+1,buttonArray[i].lastTickCount);
					messageToLoggerFromDebounceTec.messageTick = xTaskGetTickCount();
					if(xQueueSend(queueTasksToLogger,&messageToLoggerFromDebounceTec,0) != pdTRUE){
		                printf("COLA DEL LOGGER LLENA\r\n");
		            }

					//TODO:Unificar en una sola funcion
					//MEnsaje para el led de activacion de un flanco de un led
					messageToLedUpdate.ledSparkTime = 200 / portTICK_RATE_MS;//200 ms en ticks
					//TODO: Hacer algo mas seguro que esto y tambien mas escalable
					messageToLedUpdate.ledToSpark = i; //le paso el indice de la correspondiente tecla por la cola de mensaje
					if(xQueueSend(queueTasksToLedUpdate,&messageToLedUpdate,0) != pdTRUE){
		                printf("COLA LED LLENA\r\n");
					}

					//Generar mensaje de tecla que se le hizo el debouncer
					gpioToggle(GPIO3);
				}
				break;
			default:
				printf("ERROR:DEBOUNCE TEC FSM\r\n");
				break;
			}
		}
		gpioToggle(GPIO2);
	}
}
