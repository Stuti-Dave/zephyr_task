/*UART reception led control messages to control led*/

#include <zephyr/kernel.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/gpio.h>
#include <string.h>

#define UART_DEVICE_NODE DT_CHOSEN(zephyr_console)
static const struct device *uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

#define LED_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

#define RX_BUF_SIZE 64
static uint8_t rx_buf[RX_BUF_SIZE];
static size_t rx_pos = 0;

static void process_command(const char *cmd){
	if(strcmp(cmd, "LED ON")==0){
		gpio_pin_set_dt(&led, 1);
		printk("LED turned ON\n");
	}else if(strcmp(cmd, "LED OFF") == 0){
		gpio_pin_set_dt(&led, 0);
		printk("LED turned OFF\n");
	}else if(strcmp(cmd, "TOGGLE")==0){
		gpio_pin_toggle_dt(&led);
		printk("LED toggled\n");
	}else{
		printk("Error command not found\n");
	}
}

/* Receives from console*/
static void uart_cb(const struct device *dev, void *user_data){
	uint8_t c;
	while(uart_irq_update(dev) && uart_irq_rx_ready(dev)){
		uart_fifo_read(dev, &c, 1);
		
		if(c == '\n' || c == '\r'){
			rx_buf[rx_pos] = '\0';
			process_command(rx_buf);
			rx_pos=0;
		}else if(rx_pos < RX_BUF_SIZE - 1){
			rx_buf[rx_pos++] = c;
		}
	}
}

int main(void)
{
	if(!device_is_ready(uart_dev)){
		printk("UART not ready\n");
		return -1;
	}
	
	gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	
	/*Welcome message */
	const char welcome[] = "Send 'LED ON','LED OFF',or 'TOGGLE'\r\n";
	for(int i = 0; i < strlen(welcome); i++){
		uart_poll_out(uart_dev, welcome[i]);
	}
	
	uart_irq_callback_user_data_set(uart_dev, uart_cb, NULL);
	uart_irq_rx_enable(uart_dev);
	
	while(1){
		k_sleep(K_FOREVER);
	}

	return 0;
}
