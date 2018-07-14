#include "../../practica_01_04/inc/uartgpio.h"
#include "sapi.h"

static bool_t get_new_byte_available (void);


typedef struct {
	gpioMap_t gpio_uart;
	uartGpioFsmState_t uart_state;
	TickType_t uart_delay;
	uint8_t byte_a_transmitir;
	bool_t new_byte_available;
} uartData_t;

uartData_t uart_gpio;

bool_t sw_uart_config (uint16_t baudrate, gpioMap_t gpio_uart){
	if(baudrate > MAX_BAUD_RATE){
		printf("UART GPIO ERROR: Baud Rate configurado > 500 bps\r\n\n");
		return FALSE;
	}
	else if(baudrate == 0){
		uart_gpio.uart_delay = (TickType_t) ((1000/BAUD_RATE_DEFAULT))/portTICK_PERIOD_MS;
	} else {
		uart_gpio.uart_delay = (TickType_t) ((1000/baudrate))/portTICK_PERIOD_MS;
	}

	uart_gpio.byte_a_transmitir = 0;
	uart_gpio.gpio_uart = gpio_uart;
	gpioInit( uart_gpio.gpio_uart, GPIO_OUTPUT );

	uart_gpio.uart_state = UART_GPIO_IDLE;
	uart_gpio.new_byte_available = FALSE;
	printf("UART GPIO: UART Configurada a un Tiempo de Bit en Ticks:%d por Puerto EDU-CIAA:%d\r\n\n",uart_gpio.uart_delay,uart_gpio.gpio_uart);
	return TRUE;
}

void sw_uart_sent (uint8_t byte_a_transmitir){
	uart_gpio.byte_a_transmitir = byte_a_transmitir;
	uart_gpio.new_byte_available = TRUE;
}

void sw_uart_update(void * taskParmPtr){
	TickType_t tiempo_inicio_ciclo;
	tiempo_inicio_ciclo = xTaskGetTickCount();
	gpioInit( GPIO1, GPIO_OUTPUT );
	while (TRUE){
		gpioWrite(GPIO1,ON);
		switch (uart_gpio.uart_state){
		case UART_GPIO_IDLE:
			if(get_new_byte_available()== TRUE){
				uart_gpio.uart_state = UART_GPIO_BIT_START;
			}
			gpioWrite(uart_gpio.gpio_uart,ON);
			break;
		case UART_GPIO_BIT_START:
			gpioWrite(uart_gpio.gpio_uart,OFF);
			uart_gpio.uart_state = UART_GPIO_BIT0;
			break;
		case UART_GPIO_BIT0:
			gpioWrite(uart_gpio.gpio_uart,uart_gpio.byte_a_transmitir & UART_GPIO_LSB_MASK);
			uart_gpio.byte_a_transmitir= uart_gpio.byte_a_transmitir >> 1;
			uart_gpio.uart_state = UART_GPIO_BIT1;
			break;
		case UART_GPIO_BIT1:
			gpioWrite(uart_gpio.gpio_uart,uart_gpio.byte_a_transmitir & UART_GPIO_LSB_MASK);
			uart_gpio.byte_a_transmitir= uart_gpio.byte_a_transmitir >> 1;
			uart_gpio.uart_state = UART_GPIO_BIT2;
			break;
		case UART_GPIO_BIT2:
			gpioWrite(uart_gpio.gpio_uart,uart_gpio.byte_a_transmitir & UART_GPIO_LSB_MASK);
			uart_gpio.byte_a_transmitir= uart_gpio.byte_a_transmitir >> 1;
			uart_gpio.uart_state = UART_GPIO_BIT3;
			break;
		case UART_GPIO_BIT3:
			gpioWrite(uart_gpio.gpio_uart,uart_gpio.byte_a_transmitir & UART_GPIO_LSB_MASK);
			uart_gpio.byte_a_transmitir= uart_gpio.byte_a_transmitir >> 1;
			uart_gpio.uart_state = UART_GPIO_BIT4;
			break;
		case UART_GPIO_BIT4:
			gpioWrite(uart_gpio.gpio_uart,uart_gpio.byte_a_transmitir & UART_GPIO_LSB_MASK);
			uart_gpio.byte_a_transmitir= uart_gpio.byte_a_transmitir >> 1;
			uart_gpio.uart_state = UART_GPIO_BIT5;
			break;
		case UART_GPIO_BIT5:
			gpioWrite(uart_gpio.gpio_uart,uart_gpio.byte_a_transmitir & UART_GPIO_LSB_MASK);
			uart_gpio.byte_a_transmitir= uart_gpio.byte_a_transmitir >> 1;
			uart_gpio.uart_state = UART_GPIO_BIT6;
			break;
		case UART_GPIO_BIT6:
			gpioWrite(uart_gpio.gpio_uart,uart_gpio.byte_a_transmitir & UART_GPIO_LSB_MASK);
			uart_gpio.byte_a_transmitir= uart_gpio.byte_a_transmitir >> 1;
			uart_gpio.uart_state = UART_GPIO_BIT7;
			break;
		case UART_GPIO_BIT7:
			gpioWrite(uart_gpio.gpio_uart,uart_gpio.byte_a_transmitir & UART_GPIO_LSB_MASK);
			uart_gpio.byte_a_transmitir= uart_gpio.byte_a_transmitir >> 1;
			uart_gpio.uart_state = UART_GPIO_BIT_STOP;
			break;
		case UART_GPIO_BIT_STOP:
			gpioWrite(uart_gpio.gpio_uart,ON);
			uart_gpio.uart_state = UART_GPIO_IDLE;
			break;
		default:
			printf("UART GPIO:ESTADO INCORRECTO\r\n\n");
			break;
		}

		gpioWrite(GPIO1,OFF);
		vTaskDelayUntil(&tiempo_inicio_ciclo,uart_gpio.uart_delay);

	}
}

static bool_t get_new_byte_available (void){
	if(uart_gpio.new_byte_available == TRUE){
		uart_gpio.new_byte_available = FALSE;
		return TRUE;
	}else {
		return FALSE;
	}
}
