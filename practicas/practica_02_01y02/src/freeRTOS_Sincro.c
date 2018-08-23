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
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "semphr.h"

// sAPI header
#include "sapi.h"

/*==================[definiciones y macros]==================================*/
typedef enum {
   UP,
   DOWN
} ButtonState;
/*==================[definiciones de datos internos]=========================*/
TickType_t TiempoEncendido;
SemaphoreHandle_t Launch_Led, Mutex_But_Led;

/*==================[definiciones de datos externos]=========================*/

DEBUG_PRINT_ENABLE;

/*==================[declaraciones de funciones internas]====================*/

/*==================[declaraciones de funciones externas]====================*/

// Prototipo de funcion de la tarea
void Button( void* taskParmPtr );

void LED1Task( void* taskParmPtr );
void LED2Task( void* taskParmPtr );
void LED3Task( void* taskParmPtr );

/*==================[funcion principal]======================================*/

// FUNCION PRINCIPAL, PUNTO DE ENTRADA AL PROGRAMA LUEGO DE ENCENDIDO O RESET.
int main(void)
{
   // ---------- CONFIGURACIONES ------------------------------
   // Inicializar y configurar la plataforma
   boardConfig();
   Launch_Led = xSemaphoreCreateCounting(3, 0);
   Mutex_But_Led = xSemaphoreCreateMutex();
   // UART for debug messages
   debugPrintConfigUart( UART_USB, 115200 );
   debugPrintlnString( "Blinky con freeRTOS y sAPI." );


   // Crear tarea en freeRTOS
   xTaskCreate(
      Button,                     // Funcion de la tarea a ejecutar
      (const char *)"Button",     // Nombre de la tarea como String amigable para el usuario
      configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
      0,                          // Parametros de tarea
      tskIDLE_PRIORITY+1,         // Prioridad de la tarea
      0                           // Puntero a la tarea creada en el sistema
   );

   xTaskCreate(
	 LED1Task,                     // Funcion de la tarea a ejecutar
	 (const char *)"LED1",     // Nombre de la tarea como String amigable para el usuario
	 configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
	 0,                          // Parametros de tarea
	 tskIDLE_PRIORITY+2,         // Prioridad de la tarea
	 0                           // Puntero a la tarea creada en el sistema
  );

   xTaskCreate(
	LED2Task,                     // Funcion de la tarea a ejecutar
	 (const char *)"LED2",     // Nombre de la tarea como String amigable para el usuario
	 configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
	 0,                          // Parametros de tarea
	 tskIDLE_PRIORITY+2,         // Prioridad de la tarea
	 0                           // Puntero a la tarea creada en el sistema
  );

   xTaskCreate(
	LED3Task,                     // Funcion de la tarea a ejecutar
	 (const char *)"LED3",     // Nombre de la tarea como String amigable para el usuario
	 configMINIMAL_STACK_SIZE*2, // Cantidad de stack de la tarea
	 0,                          // Parametros de tarea
	 tskIDLE_PRIORITY+2,         // Prioridad de la tarea
	 0                           // Puntero a la tarea creada en el sistema
  );

   // Iniciar scheduler
   vTaskStartScheduler();

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

/*==================[definiciones de funciones externas]=====================*/

// Implementacion de funcion de la tarea
void Button( void* taskParmPtr )
{
   // ---------- CONFIGURACIONES ------------------------------
	static ButtonState Estado = UP;
	static TickType_t TiempoPulsado;
   // ---------- REPETIR POR SIEMPRE --------------------------
   while(TRUE) {
	   //Fijo un período de chequeo de 10ms
	   vTaskDelay( 10 / portTICK_RATE_MS );
	   //xSemaphoreTake ( Mutex_But_Led, portMAX_DELAY);
	   // Detecta Botón presionado
	   if(!gpioRead( TEC1 ) && Estado == UP){
		   //Espero 20ms para confirmar que está presionado
		   vTaskDelay( 20 / portTICK_RATE_MS );
		   //Confirmo que estaba presionado
		   if (!gpioRead( TEC1 )){
			   //gpioWrite( LED3, ON );
			   Estado = DOWN;
			   TiempoPulsado = xTaskGetTickCount();
		   }
		   //o no...
	   }

	   //Detecta Botón Liberado
	   if(gpioRead( TEC1 ) && Estado == DOWN){
		   vTaskDelay( 20 / portTICK_RATE_MS );
		   if(gpioRead( TEC1 )){
			   // Led para dar se�al de vida
			   //gpioWrite( LED3, OFF );
			   Estado = UP;
			   //Acá suponemos que el tiempo de uso será menor a 1000hs
			   TiempoPulsado = xTaskGetTickCount() - TiempoPulsado;
			   //Acá damos por entendido que sólo hay dos tareas sincronizadas
			   TiempoEncendido = TiempoPulsado;
			   //gpioWrite( LED3, OFF );
			   xSemaphoreGive( Launch_Led );
			   xSemaphoreGive( Launch_Led );
			   xSemaphoreGive( Launch_Led );
		   }
	   }
	   //xSemaphoreGive( Mutex_But_Led );
   }
}

// Implementacion de funcion de la tarea
void LED1Task( void* taskParmPtr )
{
   // ---------- CONFIGURACIONES -----------------------------
   // ---------- REPETIR POR SIEMPRE --------------------------
   while(TRUE) {
	  xSemaphoreTake ( Launch_Led, portMAX_DELAY);
	  // Enciende el LEDB (Tomo el recurso)
	  //xSemaphoreTake ( Mutex_But_Led, portMAX_DELAY);
	  gpioWrite( LEDR, 1 );
      // Envia la tarea al estado bloqueado durante TiempoEncendido (en ticks)
      vTaskDelay ( TiempoEncendido );
      //xSemaphoreGive( Mutex_But_Led );
      gpioWrite( LEDR, 0 );
      vTaskDelay ( 150 / portTICK_RATE_MS );
   }
}

void LED2Task( void* taskParmPtr )
{
   // ---------- CONFIGURACIONES -----------------------------
   // ---------- REPETIR POR SIEMPRE --------------------------
   while(TRUE) {
	  xSemaphoreTake ( Launch_Led, portMAX_DELAY);
	  // Enciende el LEDB (Tomo el recurso)
	  //xSemaphoreTake ( Mutex_But_Led, portMAX_DELAY);
	  gpioWrite( LEDG, 1 );
      // Envia la tarea al estado bloqueado durante TiempoEncendido (en ticks)
      vTaskDelay ( TiempoEncendido );
      //xSemaphoreGive( Mutex_But_Led );
      gpioWrite( LEDG, 0 );
      vTaskDelay ( 150 / portTICK_RATE_MS );
   }
}

void LED3Task( void* taskParmPtr )
{
   // ---------- CONFIGURACIONES -----------------------------
   // ---------- REPETIR POR SIEMPRE --------------------------
   while(TRUE) {
	  xSemaphoreTake ( Launch_Led, portMAX_DELAY);
	  // Enciende el LEDB (Tomo el recurso)
	  //xSemaphoreTake ( Mutex_But_Led, portMAX_DELAY);
	  gpioWrite( LEDB, 1 );
      // Envia la tarea al estado bloqueado durante TiempoEncendido (en ticks)
      vTaskDelay ( TiempoEncendido );
      //xSemaphoreGive( Mutex_But_Led );
      gpioWrite( LEDB, 0 );
      vTaskDelay ( 150 / portTICK_RATE_MS );
   }
}

/*==================[fin del archivo]========================================*/
