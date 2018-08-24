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

/*==================[definiciones de datos internos]=========================*/

QueueHandle_t queueIRQToDebounceTec;
QueueHandle_t queueDebounceTecToEdgeDetector;
QueueHandle_t queueTasksToLogger;
QueueHandle_t queueTasksToLedUpdate;



// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main(void)
{
   // ---------- CONFIGURACIONES ------------------------------
   // Inicializar y configurar la plataforma
   boardConfig();
   // UART for debug messages
   printf("EDGE DETECTOR\r\n");


   queueIRQToDebounceTec = xQueueCreate(10,sizeof(messageQueueIRQToDebounceTec_t));
   if(queueIRQToDebounceTec == NULL)
	   printf("COLA ISR a Teclas NO CREADA\r\n");

   queueTasksToLogger = xQueueCreate(10,sizeof(messageToLogger_t));
   if(queueIRQToDebounceTec == NULL)
	   printf("COLA LOGGER NO CREADA\r\n");

   queueTasksToLedUpdate = xQueueCreate(10,sizeof(messageToLedUpdate_t));
   if(queueTasksToLedUpdate == NULL)
	   printf("COLA LED UPDATE NO CREADA\r\n");

   //Inicializo las interrupciones
   initIRQ();
   initLogger();
   initLed();
   debounceTecInit();

   // Crear tarea en freeRTOS
   xTaskCreate(
		   debounceTec,                // Funcion de la tarea a ejecutar
		   (const char *)"debounceTec",// Nombre de la tarea como String amigable para el usuario
		   configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
		   0,                          // Parametros de tarea
		   tskIDLE_PRIORITY+2,         // Prioridad de la tarea
		   0                           // Puntero a la tarea creada en el sistema
   );

   // Crear tarea en freeRTOS
   xTaskCreate(
		   logger,                     // Funcion de la tarea a ejecutar
		   (const char *)"logger",	   // Nombre de la tarea como String amigable para el usuario
		   configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
		   0,                          // Parametros de tarea
		   tskIDLE_PRIORITY+1,         // Prioridad de la tarea
		   0                           // Puntero a la tarea creada en el sistema
   );

   // Crear tarea en freeRTOS
   xTaskCreate(
		   ledUpdate,                  // Funcion de la tarea a ejecutar
		   (const char *)"led", 	   // Nombre de la tarea como String amigable para el usuario
		   configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
		   0,                          // Parametros de tarea
		   tskIDLE_PRIORITY+3,         // Prioridad de la tarea
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


