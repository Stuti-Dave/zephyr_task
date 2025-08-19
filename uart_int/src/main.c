/*UART messages to console with interupt
 * Author: Stuti Dave*/

#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <string.h>

#define UART_DEVICE_NODE DT_CHOSEN(zephyr_console)
#define RX_BUF_SIZE 64

static const struct device *uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);
static uint8_t rx_buf[RX_BUF_SIZE];
static size_t rx_pos = 0;

static void uart_cb(const struct device *dev, void *user_data){
	uint8_t ch;
	while(uart_irq_update(dev) && uart_irq_rx_ready(dev)){
		uart_fifo_read(dev, &ch, 1);
		if(ch == '\n' || ch == '\r') {
			rx_buf[rx_pos] = '\0';
			printk("Received: %s\n", rx_buf);
			rx_pos=0;
		}else if(rx_pos < RX_BUF_SIZE - 1){
			rx_buf[rx_pos++]=ch;
		}
	}
}

int  main(void){
	if(!device_is_ready(uart_dev)){
		printk("UART not ready\n");
		return -1;
	}
	/*Sending welcome using polling */
	const char welcome[] = "UART interrupt reception\r\n";
	for(int str_index = 0; str_index < strlen(welcome); str_index++){
		uart_poll_out(uart_dev, welcome[str_index]);
	}
	
	/*Setup interrupt callback */
	uart_irq_callback_user_data_set(uart_dev, uart_cb, NULL);
	uart_irq_rx_enable(uart_dev);
	while(1){
		k_sleep(K_FOREVER);
	}
	return 0;
}
