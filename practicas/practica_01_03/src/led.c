#include "debounceTec.h"
#include "led.h"

DEBUG_PRINT_ENABLE

//Data de un led,
typedef struct {
	gpioMap_t led;
	ledFsmState_t led_state;
	TickType_t tiempo_restante_led_encendido;
	TickType_t tiempo_tecla_presionada;
}ledData_t;

static gpioMap_t leds[] = {LEDB,LED1,LED2,LED3};
#define LEDS_VALIDOS sizeof(leds)/sizeof(gpioMap_t)

static ledData_t led_array[LEDS_VALIDOS];

void ledInit(void){
	uint32_t i = 0;
	for(i=0;i<LEDS_VALIDOS;i++){
		led_array[i].led = leds[i];
		led_array[i].led_state = LED_ESPERAR_TECLA_LIBERADA;
		led_array[i].tiempo_tecla_presionada = 0;
		led_array[i].tiempo_restante_led_encendido = 0;
	}
}

void ledUpdate(void* taskParmPtr){
	uint32_t i = 0;
	TickType_t tiempo_inicio_ciclo;
	gpioInit( GPIO2, GPIO_OUTPUT );//DEBUG con Analizador Logico
	gpioInit( GPIO3, GPIO_OUTPUT );//DEBUG con Analizador Logico
	tiempo_inicio_ciclo = xTaskGetTickCount();
	while(1){
		gpioWrite(GPIO3,ON);
		for(i=0;i<LEDS_VALIDOS;i++){
			switch(led_array[i].led_state){
			case LED_ESPERAR_TECLA_LIBERADA:
				if(getTeclaLiberada(i)== TRUE){
					led_array[i].led_state = LED_ENCENDIDO;
					led_array[i].tiempo_tecla_presionada = getTeclaTiempoPresionada(i);
					led_array[i].tiempo_restante_led_encendido = led_array[i].tiempo_tecla_presionada - (LEDS_TIME_PERIOD_MS/portTICK_RATE_MS);
					gpioWrite(led_array[i].led,ON);
					gpioWrite(GPIO2,ON);
					printf("Modulo Led: Detecto Tecla%d Liberada con un timepo presionada de %d TICKS",i+1,led_array[i].tiempo_tecla_presionada);
				}
				break;
			case LED_ENCENDIDO:
				if(led_array[i].tiempo_restante_led_encendido == 0 || led_array[i].tiempo_restante_led_encendido > led_array[i].tiempo_tecla_presionada){
					led_array[i].led_state = LED_ESPERAR_TECLA_LIBERADA;
					led_array[i].tiempo_restante_led_encendido = 0;
					led_array[i].tiempo_tecla_presionada = 0;
					gpioWrite(led_array[i].led,OFF);
					gpioWrite(GPIO2,OFF);
				}
				led_array[i].tiempo_restante_led_encendido -= (LEDS_TIME_PERIOD_MS/portTICK_RATE_MS);
				printf("Modulo Led:Timepo Restante LED%d encendido %d TICKS ",i+1,led_array[i].tiempo_restante_led_encendido);
				break;
			default:
				debugPrintlnString("MODULO LED ESTADO INCORRECTO\n");			//que hubo un error en la FSM
				break;
			}
		}
		gpioWrite(GPIO3,OFF);
		vTaskDelayUntil(&tiempo_inicio_ciclo,LEDS_TIME_PERIOD_MS/portTICK_RATE_MS);
	}
}
