#include "sapi.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#define MAX_BAUD_RATE 500
#define BAUD_RATE_DEFAULT 300
#define UART_GPIO_LSB_MASK 0x01

typedef enum {
	UART_GPIO_IDLE = 0,
	UART_GPIO_BIT_START,
	UART_GPIO_BIT0,
	UART_GPIO_BIT1,
	UART_GPIO_BIT2,
	UART_GPIO_BIT3,
	UART_GPIO_BIT4,
	UART_GPIO_BIT5,
	UART_GPIO_BIT6,
	UART_GPIO_BIT7,
	UART_GPIO_BIT_STOP,
}uartGpioFsmState_t;

void sw_uart_update(void * taskParmPtr);
bool_t sw_uart_sent (uint8_t byte_a_transmitir);
bool_t sw_uart_config (uint16_t baudrate, gpioMap_t gpio_uart);

