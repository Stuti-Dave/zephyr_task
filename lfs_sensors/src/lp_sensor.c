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
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <errno.h>

//==============================================================================
// Device Tree Definitions
//==============================================================================

#if DT_NODE_EXISTS(DT_ALIAS(pressure_sensor))
#define PRESSURE_NODE DT_ALIAS(pressure_sensor)
const struct device *const pressure_dev = DEVICE_DT_GET(DT_ALIAS(pressure_sensor));
#else
#error("Pressure sensor not found.");
#endif

//==============================================================================
// Private Variables
//==============================================================================

LOG_MODULE_REGISTER(pressure);

//==============================================================================
// Private Function Prototypes
//==============================================================================

int pressure_sensor_process(double *press);

//==============================================================================
// Private Function Implementation
//==============================================================================

/**
 * @brief Fetch and process the pressure sensor reading.
 *
 * This function checks whether the pressure sensor device is ready,
 * fetches the latest sample, and extracts the pressure value in kPa.
 *
 * @param[out] press Pointer to a double where the pressure value will be stored.
 *
 * @retval 0  Success, value stored in @p press.
 * @retval -1 Failure, check logs for details.
 */

int pressure_sensor_process(double *press)
{
	if (!device_is_ready(pressure_dev)) {
		LOG_ERR("sensor: %s device not ready.", pressure_dev->name);
		return -1;
	}

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
	*press = sensor_value_to_double(&pressure);
	return 0;
}
