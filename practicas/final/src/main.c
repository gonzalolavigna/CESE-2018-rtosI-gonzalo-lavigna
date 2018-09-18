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

   //colo para enviar de las interrupciones al debouncer la deteccion de un evento.
   queueIRQToDebounceTec = xQueueCreate(10,sizeof(messageQueueIRQToDebounceTec_t));
   if(queueIRQToDebounceTec == NULL)
	   printf("COLA ISR a Teclas NO CREADA\r\n");

   //cola para enviar al logger mensajes en uint8_t para poder imprimir.
   queueTasksToLogger = xQueueCreate(10,sizeof(messageToLogger_t));
   if(queueTasksToLogger == NULL)
	   printf("COLA LOGGER NO CREADA\r\n");

   //cola de mensajes para enviar un mensaje a los leds para poder hacer el destello
   queueTasksToLedUpdate = xQueueCreate(20,sizeof(messageToLedUpdate_t));
   if(queueTasksToLedUpdate == NULL)
	   printf("COLA LED UPDATE NO CREADA\r\n");

   //cola desde la que el debouncer  le envia al detector dee flancos un flanco una vez que se hizo el correspondiente debouncer.
   queueDebounceTecToEdgeDetector = xQueueCreate(10,sizeof(messageToEdgeDetector_t));
   if(queueDebounceTecToEdgeDetector == NULL)
	   printf("COLA EDGE DETECTOR NO CREADA\r\n");

   //Inicializo las interrupciones
   initIRQ();
   initLogger();
   initLed();
   debounceTecInit();

   // Crear tarea en freeRTOS
   //Tarea para hacer el debouncer de las teclas
   xTaskCreate(
		   debounceTec,                // Funcion de la tarea a ejecutar
		   (const char *)"debounceTec",// Nombre de la tarea como String amigable para el usuario
		   configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
		   0,                          // Parametros de tarea
		   tskIDLE_PRIORITY+2,         // Prioridad de la tarea
		   0                           // Puntero a la tarea creada en el sistema
   );

   // Crear tarea en freeRTOS
   //Tarea para hacer el logger, tiene la menor prioridad ya que corre en background para enviar lo que se necesita informar
   xTaskCreate(
		   logger,                     // Funcion de la tarea a ejecutar
		   (const char *)"logger",	   // Nombre de la tarea como String amigable para el usuario
		   configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
		   0,                          // Parametros de tarea
		   tskIDLE_PRIORITY+1,         // Prioridad de la tarea
		   0                           // Puntero a la tarea creada en el sistema
   );

   // Crear tarea en freeRTOS
   //Tarea para hacer el destello de los leds, prioridad mas alta para que no muera por inanicion y controlar que el tiempo se acerque lo mas posible a los 200 ms
   xTaskCreate(
		   ledUpdate,                  // Funcion de la tarea a ejecutar
		   (const char *)"led", 	   // Nombre de la tarea como String amigable para el usuario
		   configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
		   0,                          // Parametros de tarea
		   tskIDLE_PRIORITY+3,         // Prioridad de la tarea
		   0                           // Puntero a la tarea creada en el sistema
   );

   // Crear tarea en freeRTOS
   //Tarea que ejecuta la medicion de tiemp ode las pulsaciones de las teclas.
   xTaskCreate(
		   edgeDetectorUpdate,         // Funcion de la tarea a ejecutar
		   (const char *)"edge detector", 	   // Nombre de la tarea como String amigable para el usuario
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


