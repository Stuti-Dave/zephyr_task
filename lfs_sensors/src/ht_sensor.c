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
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
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

LOG_MODULE_REGISTER(hum_temp);

//==============================================================================
// Private Function Prototypes
//==============================================================================

int hum_temp_process(double *temp, double *hum);

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
int hum_temp_process(double *temp, double *hum){
	if (!device_is_ready(hts_dev)) {
		LOG_ERR("sensor: %s device not ready.", hts_dev->name);
		return -1;
	}

	if (sensor_sample_fetch(hts_dev) < 0) {
		LOG_ERR("sensor: %s sample update error", hts_dev->name);
		return -1;
	}

	struct sensor_value temperature, humidity;
	if (sensor_channel_get(hts_dev, SENSOR_CHAN_AMBIENT_TEMP, &temperature) < 0) {
		LOG_ERR("sensor: %s read temperature channel failed", hts_dev->name);
		return -1;
	}
	
	if (sensor_channel_get(hts_dev, SENSOR_CHAN_HUMIDITY, &humidity) < 0) {
		LOG_ERR("sensor: %s read humidity channel failed", hts_dev->name);
		return -1;
	}

	/* store temperature reading*/
	*temp = sensor_value_to_double(&temperature);

	/* store humidity reading*/
	*hum = sensor_value_to_double(&humidity);

	return 0;
}
