#include "../../practica_01_04/inc/test.h"

#include "../../practica_01_04/inc/uartgpio.h"

void testUpdate (void * taskParmPtr){
	uint8_t uart_usb_byte_recibido;
	while(1){
		if(uartReadByte(UART_USB,&uart_usb_byte_recibido)){
			if (sw_uart_sent(uart_usb_byte_recibido) == TRUE){
				uartWriteByte(UART_USB,uart_usb_byte_recibido);
			}
			else {
				printf("\nBYTE RECHAZADO:");
				uartWriteByte(UART_USB,uart_usb_byte_recibido);
				printf("\n");
			}
		}
	}
}
