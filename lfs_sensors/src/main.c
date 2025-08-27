/**
 * @file main.c
 * @brief Main module for logger initialization.
 *
 * This module initalizes the mounting and logging
 * the sensor data from respective sensors to the 
 * files in Little FS.
 *
 * @date 15-08-2025
 * @author Stuti Dave
 */

//==============================================================================
// Includes
//==============================================================================

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

//==============================================================================
// Logging Module Register
//==============================================================================

LOG_MODULE_REGISTER(main);

//==============================================================================
// Function Definition
//==============================================================================

/**
 * @brief Main Application
 *
 * This is a functionality to initialize the logger application for 
 * mounting the filesystem on board and log sensor data into files.
 *
 * Returns: 0  Success, logger .
 *         -1  Failure, error logged.
 */

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
