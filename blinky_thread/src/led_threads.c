/**/

#include "led_threads.h"

static const struct gpio_dt_spec led1=GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2=GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3=GPIO_DT_SPEC_GET(LED3_NODE, gpios);
static const struct gpio_dt_spec led4=GPIO_DT_SPEC_GET(LED4_NODE, gpios);

void led1_thread(void *a, void *b, void *c){
        while(1){
                gpio_pin_toggle_dt(&led1);
                k_msleep(SLEEP_MS1);
        }
}

void led2_thread(void *a, void *b, void *c){
        while(1){
                gpio_pin_toggle_dt(&led2);
                k_msleep(SLEEP_MS2);
        }
}

void led3_thread(void *a, void *b, void *c){
        while(1){
                gpio_pin_toggle_dt(&led3);
                k_msleep(SLEEP_MS3);
        }
}

void led4_thread(void *a, void *b, void *c){
        while(1){
                gpio_pin_toggle_dt(&led4);
                k_msleep(SLEEP_MS4);
        }
}
