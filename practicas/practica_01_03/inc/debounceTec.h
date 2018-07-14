#define BUTTONS_TIME_PERIOD_MS 40
#include "sapi.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

//Esto sirve para indicar el indice de la tecla, esto permite abstraernos de donde esta ubicada efectivamente
//la tecla, esto lo podemso cambiar desde el .C
typedef enum {
	TEC1_INDICE = 0,
	TEC2_INDICE,
	TEC3_INDICE,
	TEC4_INDICE,
} buttonGpioIndex_t;

//Estados de la maquina de estados que controla la maquina de estado.
typedef enum {
	BUTTON_UP = 0,
	BUTTON_FALLING,
	BUTTON_DOWN,
	BUTTON_RAISING
} fsmDebounce_t;

void debounceTecInit(void);
void debounceTecUpdate(void* taskParmPtr);

//Esta funcion devuelve el estado de la maquina de estado, permite saber siempre si la tecla indicada
//en tecla_indice se encuetnra pulsada.
bool_t getTeclaLiberada (buttonGpioIndex_t tecla_index);
TickType_t getTeclaTiempoPresionada (buttonGpioIndex_t tecla_index);
