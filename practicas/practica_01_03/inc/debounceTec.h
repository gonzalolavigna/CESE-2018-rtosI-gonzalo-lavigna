#define BUTTONS_TIME_PERIOD_MS 50

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
void debounceTecUpdate(void);

//Esta funcion devuelve el estado de la maquina de estado, permite saber siempre si la tecla indicada
//en tecla_indice se encuetnra pulsada.
fsmDebounce_t getTeclaStatus (buttonGpioIndex_t tecla_index);
