/* blink.c custom blink
 * Author: Stuti
 * Date: 05-08-2025*/

#include <stdio.h>
#include<zephyr/kernel.h>
#include<zephyr/drivers/gpio.h>

#define DELAY_MS 1000
#define LED_LD4 DT_ALIAS(led3)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_LD4, gpios);


