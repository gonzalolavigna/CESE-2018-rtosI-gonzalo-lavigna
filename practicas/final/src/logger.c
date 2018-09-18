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

void initLogger(void){
	//Solo para debug y testear timing
	gpioInit(GPIO4,GPIO_OUTPUT);
	gpioWrite(GPIO4,OFF);
}


void logger(void* taskParmPtr){
	messageToLogger_t messageFromTask;
	while (1){
		if(xQueueReceive(queueTasksToLogger,&messageFromTask,portMAX_DELAY) == pdTRUE){
			gpioToggle(GPIO4);
			switch(messageFromTask.origin){
			case TECLA_TASK:
				printf("MESSAGE->FROM:DEBOUNCER TASK->AT:%d----> ",messageFromTask.messageTick);
				printf("%s\r\n",messageFromTask.messageToPrint);
				break;
			case IRQs:
				printf("MESSAGE->FROM:IRQs->AT:%d----> ",messageFromTask.messageTick);
				printf("%s\r\n",messageFromTask.messageToPrint);
				break;
			case LED_TASK:
				printf("MESSAGE->FROM:LED TASK->AT:%d----> ",messageFromTask.messageTick);
				printf("%s\r\n",messageFromTask.messageToPrint);
				break;
			case EDGE_DETECTOR_TASK:
				printf("MESSAGE->FROM:EDGE DETECTOR TASK->AT:%d---> ",messageFromTask.messageTick);
				printf("%s\r\n",messageFromTask.messageToPrint);
				break;
			default:
				printf("ERROR IN LOGGER TASK\r\n");
				break;

			}
			gpioToggle(GPIO4);
		}
	}
}

void sendMessageToLogger(taskOrigin_t origin, uint8_t * messageToPrint){

}
