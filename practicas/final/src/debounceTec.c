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
static messageToEdgeDetector_t messageToEdgeDectorFromDebounceTec;



typedef struct {
	buttonState_t 	state;
	TickType_t 		lastTickCount;
} debounceState_t;
debounceState_t buttonArray[2];


void debounceTecInit(void){
	//Inicializo todos los leds como que estan sin pulsar, ya que es el estado inicial.
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
		queueReturn = xQueueReceive(queueIRQToDebounceTec,&messageFromQueue,10/portTICK_RATE_MS); //Si no hay evento por 10 ms de flanco recorro toda la FSM pero con otras
		//condiciones
		gpioToggle(GPIO2);
		for(int i = 0; i < 2; i++){//Recorro todas las teclas no importa
			switch(buttonArray[i].state){
			case BUTTON_UP: //Solo tiene sentido recorrer y cambiar a falling si me llego un mensaje a la tecla que le hago el debouncer
				if(queueReturn == pdTRUE){//Solo voy a falling si el evento es de mi tecla y si es descendente otra cosa la ignoro
					if(messageFromQueue.flancoTecla == FALLING_EDGE && messageFromQueue.tecla == i){
						buttonArray[messageFromQueue.tecla].state = BUTTON_FALLING;
						buttonArray[messageFromQueue.tecla].lastTickCount = messageFromQueue.irqTickTime;
					}
				}
				break;
			case BUTTON_FALLING:
				if(queueReturn == pdTRUE){ //Si me llevo un evento mientras uan tecla estaba haciendo el proceso de ir a falling vuelo ya que detecte un rebote
					if(messageFromQueue.tecla == i){
						buttonArray[messageFromQueue.tecla].state = BUTTON_UP;
						buttonArray[messageFromQueue.tecla].lastTickCount = messageFromQueue.irqTickTime;
					}
				}
				else { //Si no tengo pdTrue es que paso el tiempo de los 10 ms sin tener un evento , sino tengo evento cualquier tecla que este mirando tiene que pasar a down
					//esto permite que si dos teclas son pulsadas con poco tiempo de distancia o sea dentro de 10 ticks pasan las dos a down.
					gpioToggle(GPIO3);
					buttonArray[i].state = BUTTON_DOWN;


					//Mando por la cola de mensaje al edge detector que tengo un flaco detectado con el debouncer incluido
					messageToEdgeDectorFromDebounceTec.edgeTickTime = buttonArray[i].lastTickCount;
					messageToEdgeDectorFromDebounceTec.flancoTecla = FALLING_EDGE;
					messageToEdgeDectorFromDebounceTec.tecla = i;
					if(xQueueSend(queueDebounceTecToEdgeDetector,&messageToEdgeDectorFromDebounceTec,0) != pdTRUE){
						printf("COLA DE EDGE DETECTOR LLENA\r\n");
					}


					//Omly for debuggin y testear el logger, envio cuando definitivamente puede decir que un flanco no es falso
					sprintf(messageToLoggerFromDebounceTec.messageToPrint,"TEC%d FALLING EDGE TICK TIME:%d",i+1,buttonArray[i].lastTickCount);
					messageToLoggerFromDebounceTec.messageTick = xTaskGetTickCount();
					if(xQueueSend(queueTasksToLogger,&messageToLoggerFromDebounceTec,0) != pdTRUE){
		                printf("COLA DEL LOGGER LLENA\r\n");
		            }

					//MEnsaje para el destelleo del led cuando efectivamente se detecto un evento en el led.
					messageToLedUpdate.ledSparkTime = 200 / portTICK_RATE_MS;//200 ms en ticks
					messageToLedUpdate.ledToSpark = i; //le paso el indice de la correspondiente tecla por la cola de mensaje
					if(xQueueSend(queueTasksToLedUpdate,&messageToLedUpdate,0) != pdTRUE){
		                printf("COLA LED LLENA\r\n");
					}


					//Generar mensaje de tecla que se le hizo el debouncer
					gpioToggle(GPIO3);
				}
				break;
			case BUTTON_DOWN: //Si el boton esta pulsado directamente solo cambio de estado cuando recibo algun flanco.
				if(queueReturn == pdTRUE){
					if(messageFromQueue.flancoTecla == RISING_EDGE && messageFromQueue.tecla == i){
						buttonArray[messageFromQueue.tecla].state = BUTTON_RAISING;
						buttonArray[messageFromQueue.tecla].lastTickCount = messageFromQueue.irqTickTime;
					}
				}
				break;
			case BUTTON_RAISING://Mismos criteroios que el estado de BUTTON_FALLING
				if(queueReturn == pdTRUE){
					if(messageFromQueue.tecla == i){
						buttonArray[messageFromQueue.tecla].state = BUTTON_DOWN;
						buttonArray[messageFromQueue.tecla].lastTickCount = messageFromQueue.irqTickTime;
					}
				}
				else { //Si no tengo pdTrue es que paso el tiempo de los 10 ms sin tener ningun flanco lo cual implica que es una tecla valida.
					gpioToggle(GPIO3);
					buttonArray[i].state = BUTTON_UP;


					//Mando por la cola de mensaje al edge detector que tengo un flaco detectado con el debouncer incluido
					messageToEdgeDectorFromDebounceTec.edgeTickTime = buttonArray[i].lastTickCount;
					messageToEdgeDectorFromDebounceTec.flancoTecla = RISING_EDGE;
					messageToEdgeDectorFromDebounceTec.tecla = i;
					if(xQueueSend(queueDebounceTecToEdgeDetector,&messageToEdgeDectorFromDebounceTec,0) != pdTRUE){
						printf("COLA DE EDGE DETECTOR LLENA\r\n");
					}


					//ONly for debuggin y testear el logger
					sprintf(messageToLoggerFromDebounceTec.messageToPrint,"TEC%d RISING EDGE TICK TIME:%d",i+1,buttonArray[i].lastTickCount);
					messageToLoggerFromDebounceTec.messageTick = xTaskGetTickCount();
					if(xQueueSend(queueTasksToLogger,&messageToLoggerFromDebounceTec,0) != pdTRUE){
		                printf("COLA DEL LOGGER LLENA\r\n");
		            }

					//MEnsaje para el led de activacion de un flanco de un led
					messageToLedUpdate.ledSparkTime = 200 / portTICK_RATE_MS;//200 ms en ticks
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
