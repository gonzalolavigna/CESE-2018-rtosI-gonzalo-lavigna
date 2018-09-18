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

static messageToLogger_t messageToLoggerFromEdgeDetector;
static messageToLedUpdate_t messageToLedUpdateFromEdgeDetector;


void edgeDetectorUpdate(void* taskParmPtr){
	gpioInit(GPIO7,GPIO_OUTPUT);
	messageToLoggerFromEdgeDetector.origin = EDGE_DETECTOR_TASK;
	messageToEdgeDetector_t messageFromQueue_1;
	messageToEdgeDetector_t messageFromQueue_2;
	while(1){
		vTaskDelay(100/portTICK_RATE_MS); //La tarea la ejecuto cada 100 ms, es muy dificil tenerla prendida mas de ese tiempo a una tecla
		//Si tengo dos mensajes en la cola significa que me llegaron dos flancos
		if(uxQueueMessagesWaiting(queueDebounceTecToEdgeDetector) >= 2){
			gpioToggle(GPIO7);
			if(xQueueReceive(queueDebounceTecToEdgeDetector,&messageFromQueue_1,0)!= pdTRUE){
				printf("ERROR RECIBIENDO MENSAJE 1\r\n");
			}
			//Informo al logger los valores del primer mensaje
			sprintf(messageToLoggerFromEdgeDetector.messageToPrint,"EDGE DETECTOR MENSAJE 1 TECLA:%d EDGE:%d TICK:%d",messageFromQueue_1.tecla+1,messageFromQueue_1.flancoTecla,messageFromQueue_1.edgeTickTime);
			messageToLoggerFromEdgeDetector.messageTick = xTaskGetTickCount();
			if(xQueueSend(queueTasksToLogger,&messageToLoggerFromEdgeDetector,0)!= pdTRUE){
				printf("COLA LOGGER LLENA\r\n");
			}

			if(xQueueReceive(queueDebounceTecToEdgeDetector,&messageFromQueue_2,0)!= pdTRUE){
				printf("ERROR RECIBIENDO MENSAJE 2\r\n");
			}
			//Informo al logger los valores del segundo mensaje
			sprintf(messageToLoggerFromEdgeDetector.messageToPrint,"EDGE DETECTOR MENSAJE 2 TECLA:%d EDGE:%d TICK:%d",messageFromQueue_2.tecla+1,messageFromQueue_2.flancoTecla,messageFromQueue_2.edgeTickTime);
			messageToLoggerFromEdgeDetector.messageTick = xTaskGetTickCount();
			if(xQueueSend(queueTasksToLogger,&messageToLoggerFromEdgeDetector,0)!= pdTRUE){
				printf("COLA LOGGER LLENA\r\n");
			}

			//Informo al logger que recibi dos mensajes en la cola de mensajes
			sprintf(messageToLoggerFromEdgeDetector.messageToPrint,"EDGE DETECTOR 2 mensajes recibidos");
			messageToLoggerFromEdgeDetector.messageTick = xTaskGetTickCount();
			if(xQueueSend(queueTasksToLogger,&messageToLoggerFromEdgeDetector,0)!= pdTRUE){
				printf("COLA LOGGER LLENA\r\n");
			}
			//Se toman como validas las transferencias que sean falling to falling o rising to rising de distintas teclas
			//Primero descarto los mensajes que sean de la misma tecla ya que no me sirven
			if(messageFromQueue_1.tecla != messageFromQueue_2.tecla){
				if(messageFromQueue_1.flancoTecla == messageFromQueue_2.flancoTecla){
					//Si estoy aca significa que tengo dos eventos flancos que son de teclas distintas y son el mismo flanco
						if(messageFromQueue_1.edgeTickTime > messageFromQueue_2.edgeTickTime){ //Implica que TECLA 1 se pulso despues de TECLA 2
							sprintf(messageToLoggerFromEdgeDetector.messageToPrint,"{%d,%d:%d}\r\n",messageFromQueue_2.tecla, messageFromQueue_1.tecla,(messageFromQueue_1.edgeTickTime-messageFromQueue_2.edgeTickTime)*portTICK_RATE_MS);
							messageToLoggerFromEdgeDetector.messageTick = xTaskGetTickCount();
							if(xQueueSend(queueTasksToLogger,&messageToLoggerFromEdgeDetector,0) != pdTRUE){
								printf("COLA LOGGER LLENA\r\n");
							}
						}
						else {
							sprintf(messageToLoggerFromEdgeDetector.messageToPrint,"{%d,%d:%d}\r\n",messageFromQueue_1.tecla,messageFromQueue_2.tecla,(messageFromQueue_2.edgeTickTime-messageFromQueue_1.edgeTickTime)*portTICK_RATE_MS);
							messageToLoggerFromEdgeDetector.messageTick = xTaskGetTickCount();
							if(xQueueSend(queueTasksToLogger,&messageToLoggerFromEdgeDetector,0) != pdTRUE){
								printf("COLA LOGGER LLENA\r\n");
							}
						}
						//Hago destellar el led
						messageToLedUpdateFromEdgeDetector.ledSparkTime = 200 / portTICK_RATE_MS;
						messageToLedUpdateFromEdgeDetector.ledToSpark = LED3_INDICE;
						if(xQueueSend(queueTasksToLedUpdate,&messageToLedUpdateFromEdgeDetector,0)!= pdTRUE){
							printf("COLA LED LLENA\r\n");
						}
				}
				else{
					//Informo al logger que recibi dos mensajes de distinto flanco solo se mide FALLING-TO FALLING y RISING-TO RISING
					sprintf(messageToLoggerFromEdgeDetector.messageToPrint,"EDGE DETECTOR FLANCOS DISTINTOS DE DISTINTAS TECLA DESCARTANDO MEDICION");
					messageToLoggerFromEdgeDetector.messageTick = xTaskGetTickCount();
					if(xQueueSend(queueTasksToLogger,&messageToLoggerFromEdgeDetector,0)!= pdTRUE){
						printf("COLA LOGGER LLENA\r\n");
					}
				}
			}
			else {
				//Informo al logger que recibi dos mensajes de la misma tecla con lo cual se descartan
				sprintf(messageToLoggerFromEdgeDetector.messageToPrint,"EDGE DETECTOR FLANCOS DE LA MISMAS TECLA DESCARTANDO MEDICION");
				messageToLoggerFromEdgeDetector.messageTick = xTaskGetTickCount();
				if(xQueueSend(queueTasksToLogger,&messageToLoggerFromEdgeDetector,0) != pdTRUE){
					printf("COLA LOGGER LLENA\r\n");
				}
			}
			gpioToggle(GPIO7);
		}
	}
}
