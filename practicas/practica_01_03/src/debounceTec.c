#include "debounceTec.h"

DEBUG_PRINT_ENABLE

//Data de una tecla,
typedef struct {
	gpioMap_t tecla;					//Se guarda la tecla a la cual se realiza el debouncing.
	fsmDebounce_t state;				//Estado de la fsm, que se guarda para volver a ser llamada en el proximo ciclo
	bool_t tecla_liberada;
	TickType_t tiempo_tecla_presionada;
}debounceData_t;

//Se genera un arregla con todas las teclas validas disponible, este codigo puede
//ser ampliado a muchas teclas. Solo se debe actualizar el enum TEC INDEX; para ser llamado correctamente
static gpioMap_t teclas[] = {TEC1,TEC2,TEC3,TEC4};
#define TECLAS_VALIDAS sizeof(teclas)/sizeof(gpioMap_t)

static debounceData_t teclaArray[TECLAS_VALIDAS];

void debounceTecInit(void){
	uint32_t i= 0;
	for(i=0; i < TECLAS_VALIDAS; i++){
		teclaArray[i].state = BUTTON_UP;//Cade vez que se prende la placa, el botton esta en estado released.
		teclaArray[i].tecla = teclas[i];
		teclaArray[i].tecla_liberada = FALSE;
		teclaArray[i].tiempo_tecla_presionada = 0;
	}
	printf("Debouncer Inicializado\n");
}

void debounceTecUpdate(void* taskParmPtr){
	uint32_t i = 0;
	TickType_t tiempo_inicio_ciclo;
	TickType_t tiempo_tecla_presionada;
	gpioInit( GPIO0, GPIO_OUTPUT );//DEBUG con Analizador Logico
	gpioInit( GPIO1, GPIO_OUTPUT );//DEBUG con Analizador Logico
	tiempo_inicio_ciclo = xTaskGetTickCount();
	while(TRUE){
		gpioWrite(GPIO1,ON);
		for(i=0; i < TECLAS_VALIDAS ; i++){//Chequeo todas las posibles teclas, configuradas en el arreglo.
			switch(teclaArray[i].state){
			case BUTTON_UP:
				if(!gpioRead(teclaArray[i].tecla)){
					teclaArray[i].state=BUTTON_FALLING;
					teclaArray[i].tiempo_tecla_presionada = 0;
					teclaArray[i].tecla_liberada = FALSE;
				}
				break;
			case BUTTON_FALLING:
				if(!gpioRead(teclaArray[i].tecla)){
					teclaArray[i].state=BUTTON_DOWN;
					tiempo_tecla_presionada = xTaskGetTickCount();
					gpioWrite(GPIO0,ON); //DEBUG con Analizador Logico
				}
				else
					teclaArray[i].state = BUTTON_UP;
				break;
			case BUTTON_DOWN:
				if(gpioRead(teclaArray[i].tecla))
					teclaArray[i].state = BUTTON_RAISING;
				break;
			case BUTTON_RAISING:
				if(gpioRead(teclaArray[i].tecla)){
					teclaArray[i].state =BUTTON_UP;
					teclaArray[i].tiempo_tecla_presionada = xTaskGetTickCount() - tiempo_tecla_presionada;
					teclaArray[i].tecla_liberada = TRUE;
					printf("MODULO TECLA:TEC%d Liberada, Tiempo Pulsada %d en Ticks\n",i+1,teclaArray[i].tiempo_tecla_presionada);
					gpioWrite(GPIO0,OFF); //DEBUG con Analizador Logico
				}
				break;
			default:
				debugPrintlnString("MODULO TECLA:ESTADO INCORRECTO\n");			//que hubo un error en la FSM
				break;
			}
		}
		gpioWrite(GPIO1,OFF);
		vTaskDelayUntil(&tiempo_inicio_ciclo,BUTTONS_TIME_PERIOD_MS/portTICK_RATE_MS);
	}
}

bool_t getTeclaLiberada (buttonGpioIndex_t tecla_index){
	return teclaArray[tecla_index].tecla_liberada;
}
TickType_t getTeclaTiempoPresionada (buttonGpioIndex_t tecla_index){
	return teclaArray[tecla_index].tiempo_tecla_presionada;
}
