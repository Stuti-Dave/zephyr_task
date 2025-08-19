/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   2000

/* The devicetree node identifier for the "led0" alias. */
#define LED0_NODE DT_ALIAS(led0)
#define LED1_NODE DT_ALIAS(led1)
#define LED2_NODE DT_ALIAS(led2)
#define LED3_NODE DT_ALIAS(led3)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led4 = GPIO_DT_SPEC_GET(LED3_NODE, gpios);

int main(void)
{
	int ret1, ret2, ret3, ret4;
	bool led_state = true;

	if (!gpio_is_ready_dt(&led1)) {
		return 0;
	}

	if (!gpio_is_ready_dt(&led2)) {
                return 0;
        }

	
	if (!gpio_is_ready_dt(&led3)) {
                return 0;
        }

	if (!gpio_is_ready_dt(&led4)){
		return 0;
	}

	ret1 = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
	ret2 = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);
	ret3 = gpio_pin_configure_dt(&led3, GPIO_OUTPUT_ACTIVE);
	ret4 = gpio_pin_configure_dt(&led4, GPIO_OUTPUT_ACTIVE);
	if (ret1<0 || ret2<0 )
	{
		return 0;
	}

	ret4 = gpio_pin_toggle_dt(&led4);
	while (1) {
		/*All off*/
		ret1 = gpio_pin_toggle_dt(&led1);
		ret2 = gpio_pin_toggle_dt(&led2);
		ret3 = gpio_pin_toggle_dt(&led3);
		ret4 = gpio_pin_toggle_dt(&led4);
		if (ret1<0 || ret2<0 || ret3<0 || ret4<0) {
			return 0;
		}

		led_state = !led_state;
		printf("LED state: %s\n", led_state ? "ON" : "OFF");
	
		led_state = !led_state;
		k_msleep(SLEEP_TIME_MS);

		/*
		ret4 = gpio_pin_toggle_dt(&led4);
		k_msleep(SLEEP_TIME_MS);*/

		//ret4 = gpio_pin_toggle_dt(&led4);
		
		ret1 = gpio_pin_toggle_dt(&led1);
		k_msleep(SLEEP_TIME_MS);

		ret1 = gpio_pin_toggle_dt(&led1);
		ret2 = gpio_pin_toggle_dt(&led2);
		k_msleep(SLEEP_TIME_MS);

		ret2 = gpio_pin_toggle_dt(&led2);
		ret3 = gpio_pin_toggle_dt(&led3);
		k_msleep(SLEEP_TIME_MS);

		ret3 = gpio_pin_toggle_dt(&led3);
		ret4 = gpio_pin_toggle_dt(&led4);
		k_msleep(SLEEP_TIME_MS);

		ret4 = gpio_pin_toggle_dt(&led4);
		k_msleep(SLEEP_TIME_MS);
	}
	return 0;
}
