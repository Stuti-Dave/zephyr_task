#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include "main.h"
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main);

int main(void)
{
	k_msleep(5000);
	LOG_INF("Welcome to zephyr\n");

	if (logger_init() != 0) {
		LOG_ERR("Logger init failed");
		return -1;
	}
	return 0;
}
