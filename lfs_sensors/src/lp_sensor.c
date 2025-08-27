/**
 * @file pressure_sensor.c
 * @brief Pressure sensor processing module.
 *
 * This module provides functionality to fetch and process data
 * from the onboard pressure sensor using Zephyr's sensor API.
 *
 * It checks device readiness, fetches sensor samples, and converts
 * the pressure value into a double (kPa). The processed data can then
 * be used by application threads or logging subsystems.
 *
 * @date 15-08-2025
 * @author Stuti Dave
 */

//==============================================================================
// Includes
//==============================================================================

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <errno.h>

//==============================================================================
// Device Tree Bindings
//==============================================================================

#if DT_NODE_EXISTS(DT_ALIAS(pressure_sensor))
#define PRESSURE_NODE DT_ALIAS(pressure_sensor)
static const struct device *const pressure_dev = DEVICE_DT_GET(PRESSURE_NODE);
#else
#error("Pressure sensor not found.");
#endif

//==============================================================================
// Logging Module Register
//==============================================================================

LOG_MODULE_REGISTER(pressure, CONFIG_APP_LOG_LEVEL_DBG);

//==============================================================================
// Configuration Constants
//==============================================================================

#define MAX_MSGS			10 	// Maximum number of sensor samples in queue
#define MSGQ_ALIGN      		32 	// Align message queue entries
#define SENSOR_PRIORITY			5 	// Thread priority for sensor task
#define PRESSURE_THREAD_STACK_SIZE	512 	// Stack size for sensor thread

//==============================================================================
// Sensor-data structure
//==============================================================================

typedef struct {
        double pressure;
} press_data;

//==============================================================================
// Message Queue
//==============================================================================

// Queue for transferring pressure readings between threads
struct k_msgq lp_sensor_msgq;
K_MSGQ_DEFINE(lp_sensor_msgq, sizeof(press_data), MAX_MSGS, MSGQ_ALIGN);

//==============================================================================
// Function Prototypes
//==============================================================================

int pressure_sensor_process(press_data *data_struct);
void pressure_thread(void *, void *, void *);

//==============================================================================
// Function Definitions
//==============================================================================

/**
 * @brief Fetch and process the pressure sensor reading.
 *
 * This function checks whether the pressure sensor device is ready,
 * fetches the latest sample, and extracts the pressure value in kPa.
 *
 * Input: data_struct Pointer to store the fetched pressure.
 *
 * Return: 0  Success, value stored in data_struct pressure element.
 * 	  -1  Failure, error logged.
 */

int pressure_sensor_process(press_data *data_struct)
{
	if (sensor_sample_fetch(pressure_dev) < 0) {
		LOG_INF("sensor: %s sample update error", pressure_dev->name);
		return -1;
	}

	struct sensor_value pressure;
	if (sensor_channel_get(pressure_dev, SENSOR_CHAN_PRESS, &pressure) < 0) {
		LOG_ERR("sensor: %s cannot read pressure channel", pressure_dev->name);
		return -1;
	}

	/* store pressure reading*/
	data_struct->pressure = sensor_value_to_double(&pressure);
	return 0;
}

//==============================================================================
// Thread Definition
//==============================================================================

/*
 * Definition of Pressure thread
 */

K_THREAD_DEFINE(pressure_tid, PRESSURE_THREAD_STACK_SIZE, pressure_thread, 
		NULL, NULL, NULL, 
		SENSOR_PRIORITY, 0, 0);

//==============================================================================
// Thread Implementation
//==============================================================================

/**
 * @brief Thread function for pressure sampling.
 *
 * Workflow:
 *  1. Ensure the pressure sensor is ready.
 *  2. Periodically fetch a sample and process it.
 *  3. Push the result to the message queue.
 *  4. Sleep before the next iteration.
 */

void pressure_thread(void *, void *, void *)
{
	if (!device_is_ready(pressure_dev)) {
                LOG_ERR("sensor: %s device not ready.", pressure_dev->name);
                return;
        }

	LOG_INF("Pressure Thread started");
	press_data data_struct;

	while (1) {
		if(pressure_sensor_process(&data_struct) == 0) {
			k_msgq_put(&lp_sensor_msgq, &data_struct, K_MSEC(1000));
		}
		LOG_DBG("Pressure: %f", data_struct.pressure);
		k_sleep(K_MSEC(5000));
	}
}
