#include "../../practica_01_04/inc/test.h"

#include "../../practica_01_04/inc/uartgpio.h"

void testUpdate (void * taskParmPtr){
	uint8_t uart_usb_byte_recibido;
	gpioInit(GPIO2,GPIO_OUTPUT);
	while(1){
		gpioWrite(GPIO2,ON);
		if(uartReadByte(UART_USB,&uart_usb_byte_recibido)){
			if (sw_uart_sent(uart_usb_byte_recibido) == TRUE){
				uartWriteByte(UART_USB,uart_usb_byte_recibido);
			}
			else {
				printf("\nBYTE RECHAZADO:%c\n",uart_usb_byte_recibido);
				//uartWriteByte(UART_USB,uart_usb_byte_recibido);

			}
		}
		gpioWrite(GPIO2,OFF);
	}
}
