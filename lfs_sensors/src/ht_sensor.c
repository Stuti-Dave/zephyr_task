/**
 * @file hum_temp_sensor.c
 * @brief Humidity and Temperature Sensor Processing Module.
 *
 * This module provides functionality to fetch and process data
 * from the onboard HTS221 (or equivalent) humidity/temperature sensor
 * using Zephyr's sensor API.
 *
 * @date 15-08-2025
 * @author Stuti Dave
 */

//==============================================================================
// Includes
//==============================================================================

#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include "main.h"
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <errno.h>
#include <stdint.h>

//==============================================================================
// Device Tree Definitions
//==============================================================================

#if DT_NODE_EXISTS(DT_ALIAS(ht_sensor))
#define HUM_TEMP_NODE DT_ALIAS(ht_sensor)
static const struct device *const hts_dev = DEVICE_DT_GET(HUM_TEMP_NODE);
#else
#error("Humidity-Temperature sensor not found in device tree.")
#endif

//==============================================================================
// Private Variables
//==============================================================================

LOG_MODULE_REGISTER(hum_temp, LOG_LEVEL_DBG);

//==============================================================================
// Private Function Prototypes
//==============================================================================

int hum_temp_process(hum_temp_data* data_struct);
void hum_temp_thread(void *, void *, void *);

//==============================================================================
// Private Function Implementation
//==============================================================================

/**
 * @brief Fetch and process humidity and temperature readings.
 *
 * This function checks whether the sensor device is ready, fetches
 * the latest sample, and extracts both temperature and humidity values.
 *
 * @param[out] temp Pointer to a double where the temperature (°C) will be stored.
 * @param[out] hum  Pointer to a double where the relative humidity (%) will be stored.
 *
 * @retval 0  Success, values stored in @p temp and @p hum.
 * @retval -1 Failure, check logs for details.
 */

#define MAX_MSGS	10
#define MSGQ_ALIGN	32
#define SENSOR_PRIORITY	5
#define HT_THREAD_STACK_SIZE	512

struct k_msgq ht_sensor_msgq;
K_MSGQ_DEFINE(ht_sensor_msgq, sizeof(hum_temp_data), MAX_MSGS, MSGQ_ALIGN);

int hum_temp_process(hum_temp_data* data_struct){
	if (sensor_sample_fetch(hts_dev) < 0)
	{
		LOG_ERR("Faileed to fetch HT sample");
		return -1;
	}
	struct sensor_value temp, hum;
	if (sensor_channel_get(hts_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp) < 0) {
		LOG_ERR("sensor: %s read temperature channel failed", hts_dev->name);
		return -1;
	}
	
	if (sensor_channel_get(hts_dev, SENSOR_CHAN_HUMIDITY, &hum) < 0) {
		LOG_ERR("sensor: %s read humidity channel failed", hts_dev->name);
		return -1;
	}

	/* store temperature reading*/
	data_struct->temperature = sensor_value_to_double(&temp);

	/* store humidity reading*/
	data_struct->humidity = sensor_value_to_double(&hum);

	return 0;
}


K_THREAD_DEFINE(hum_temp_tid, HT_THREAD_STACK_SIZE, hum_temp_thread, NULL, NULL, NULL, SENSOR_PRIORITY, 0, 0);
//message queue

void hum_temp_thread(void *, void *, void *)
{
	if (!device_is_ready(hts_dev)) {
                LOG_ERR("sensor: %s device not ready.", hts_dev->name);
                return;
        }

	hum_temp_data data_struct;
	LOG_INF("HT Thread started");

	while (1) 
	{
		if (hum_temp_process(&data_struct) == 0){
			k_msgq_put(&ht_sensor_msgq, &data_struct, K_MSEC(1000));
		}
		LOG_DBG("Humidity: %.2f, Temperature: %.2f", data_struct.humidity, data_struct.temperature);
		k_sleep(K_MSEC(5000));
	}

}
