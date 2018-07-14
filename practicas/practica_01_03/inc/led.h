#define LEDS_TIME_PERIOD_MS 40
#include "sapi.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

//Esto sirve para indicar el indice de la tecla, esto permite abstraernos de donde esta ubicada efectivamente
//la tecla, esto lo podemso cambiar desde el .C
typedef enum {
	LED1_INDICE = 0,
	LED2_INDICE,
	LED3_INDICE,
	LED4_INDICE
} ledGpioIndex_t;

//Estados de la maquina de estados que controla la maquina de estado.
typedef enum {
	LED_ESPERAR_TECLA_LIBERADA = 0,
	LED_ENCENDIDO
} ledFsmState_t;

void ledInit(void);
void ledUpdate(void* taskParmPtr);
