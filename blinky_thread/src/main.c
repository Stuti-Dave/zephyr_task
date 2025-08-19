/**/

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include<zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#define SLEEP_MS1 400
#define SLEEP_MS2 800
#define SLEEP_MS3 1200
#define SLEEP_MS4 1600

#define LED1_NODE DT_ALIAS(led0)
#define LED2_NODE DT_ALIAS(led1)
#define LED3_NODE DT_ALIAS(led2)
#define LED4_NODE DT_ALIAS(led3)

#define STACK_SIZE 512
#define PRIORITY 5

//#include "led_threads.h"

LOG_MODULE_REGISTER(main);

static const struct gpio_dt_spec led1=GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2=GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3=GPIO_DT_SPEC_GET(LED3_NODE, gpios);
static const struct gpio_dt_spec led4=GPIO_DT_SPEC_GET(LED4_NODE, gpios);

K_SEM_DEFINE(sem1, 1, 1);
K_SEM_DEFINE(sem2, 0, 1);
K_SEM_DEFINE(sem3, 0, 1);
K_SEM_DEFINE(sem4, 0, 1);

void led1_thread(void *a, void *b, void *c){
        while(1){
		k_sem_take(&sem1, K_FOREVER);
                gpio_pin_toggle_dt(&led1);
                k_msleep(SLEEP_MS1);
		k_sem_give(&sem2);
        }
}

void led2_thread(void *a, void *b, void *c){
        while(1){
		k_sem_take(&sem2, K_FOREVER);
                gpio_pin_toggle_dt(&led2);
                k_msleep(SLEEP_MS2);
		k_sem_give(&sem3);
        }
}

void led3_thread(void *a, void *b, void *c){
        while(1){
		k_sem_take(&sem3, K_FOREVER);
                gpio_pin_toggle_dt(&led3);
                k_msleep(SLEEP_MS3);
		k_sem_give(&sem4);
        }
}

void led4_thread(void *a, void *b, void *c){
        while(1){
		k_sem_take(&sem4, K_FOREVER);
                gpio_pin_toggle_dt(&led4);
                k_msleep(SLEEP_MS4);
		k_sem_give(&sem1);
        }
}

K_THREAD_DEFINE(led1_id, STACK_SIZE, led1_thread, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(led2_id, STACK_SIZE, led2_thread, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(led3_id, STACK_SIZE, led3_thread, NULL, NULL, NULL, PRIORITY, 0, 0);
K_THREAD_DEFINE(led4_id, STACK_SIZE, led4_thread, NULL, NULL, NULL, PRIORITY, 0, 0);

int main(){
	LOG_INF("LED threads started");
	if(!device_is_ready(led1.port) || !device_is_ready(led2.port) || !device_is_ready(led3.port) || !device_is_ready(led4.port)){
		LOG_ERR("One or more leds not ready");
		return -1;
	}

	gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
	gpio_pin_configure_dt(&led3, GPIO_OUTPUT_ACTIVE);
	gpio_pin_configure_dt(&led4, GPIO_OUTPUT);

	LOG_INF("Thread: LEDs configured");

	return 0;
}
