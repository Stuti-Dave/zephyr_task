#ifndef LED_THREADS_H
#define LED_THREADS_H

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

static const struct gpio_dt_spec led1;
static const struct gpio_dt_spec led2;
static const struct gpio_dt_spec led3;
static const struct gpio_dt_spec led4;

void led1_thread(void *a, void *b, void *c);
void led2_thread(void *a, void *b, void *c);
void led3_thread(void *a, void *b, void *c);
void led4_thread(void *a, void *b, void *c);
#endif
