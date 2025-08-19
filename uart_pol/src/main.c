/* UART polling messages welcome on console
 * Author: Stuti*/

#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <string.h>

#define UART_DEVICE_NODE DT_CHOSEN(zephyr_console)
static const struct device *uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

void main(void){
	if(!device_is_ready(uart_dev)){
		printk("UART device not ready\n");
		return;}
	
	/*To transmit welcome message */
	const char welcome[] = "Welcome to UART polling demo!\r\n";
	for(int i = 0; i < strlen(welcome); i++){
		uart_poll_out(uart_dev, welcome[i]);
	}
	
	uint8_t c;
	while(1){
		if(uart_poll_in(uart_dev, &c) == 0){
			uart_poll_out(uart_dev, c);
		}
		k_msleep(10);
	}
}
