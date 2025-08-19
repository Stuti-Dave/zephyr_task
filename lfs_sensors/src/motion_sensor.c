/**
 * @file imu_sensor.c
 * @brief IMU (Accelerometer + Gyroscope) sensor processing module.
 *
 * This module provides functionality to fetch and process data
 * from the onboard IMU (e.g., LSM6DSL) using Zephyr's sensor API.
 *
 * It extracts both acceleration (X, Y, Z) and gyroscope (X, Y, Z) readings
 * and stores them in a user-provided @ref three_d struct.
 *
 * @date 15-08-2025
 * author Stuti Dave
 */

//==============================================================================
// Includes
//==============================================================================

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <errno.h>
#include "thread_struct.h"

//==============================================================================
// Device Tree Definitions
//==============================================================================

#if DT_NODE_EXISTS(DT_ALIAS(imu_sensor))
#define IMU_NODE DT_ALIAS(imu_sensor)
static const struct device *const imu_dev = DEVICE_DT_GET(IMU_NODE);
#else
#error("IMU sensor not found in device tree.")
#endif

//==============================================================================
// Private Variables
//==============================================================================

LOG_MODULE_REGISTER(imu);

//==============================================================================
// Private Function Prototypes
//==============================================================================

int imu_sensor_process(struct three_d *accel, struct three_d *gyro);

//==============================================================================
// Private Function Implementation
//==============================================================================

/**
 * @brief Fetch and process IMU sensor readings (accelerometer + gyroscope).
 *
 * This function checks whether the IMU sensor device is ready,
 * fetches the latest sample, and extracts both acceleration and gyroscope
 * values across all three axes (X, Y, Z).
 *
 * @param[out] accel Pointer to a @ref three_d struct where accelerometer data will be stored.
 * @param[out] gyro  Pointer to a @ref three_d struct where gyroscope data will be stored.
 *
 * @retval 0  Success, values stored in @p accel and @p gyro.
 * @retval -1 Failure, check logs for details.
 */

int imu_sensor_process(imu_sensor_data *accel, imu_sensor_data *gyro)
{
	if (!device_is_ready(imu_dev)) {
		LOG_ERR("sensor: %s device not ready.", imu_dev->name);
		return -1;
	}

	if (sensor_sample_fetch(imu_dev) < 0) {
		LOG_ERR("sensor: %s sample update error", imu_dev->name);
		return -1;
	}

	/* Fetch channel for accelerometer and gyrometer */
	if (sensor_sample_fetch_chan(imu_dev, SENSOR_CHAN_ACCEL_XYZ) < 0) {
		LOG_ERR("Sensor: %s fetch failed.", imu_dev->name);
		return -1;
	}
	char out_str[64];	
	struct sensor_value accel_x, accel_y, accel_z;
	struct sensor_value gyro_x, gyro_y, gyro_z;

    /* lsm6dsl accel */
    sensor_sample_fetch_chan(imu_dev, SENSOR_CHAN_ACCEL_XYZ);
    sensor_channel_get(imu_dev, SENSOR_CHAN_ACCEL_X, &accel_x);
    sensor_channel_get(imu_dev, SENSOR_CHAN_ACCEL_Y, &accel_y);
    sensor_channel_get(imu_dev, SENSOR_CHAN_ACCEL_Z, &accel_z);

    sprintf(out_str, "accel x:%f ms/2 y:%f ms/2 z:%f ms/2", sensor_value_to_double(&accel_x),
            sensor_value_to_double(&accel_y), sensor_value_to_double(&accel_z));


    /* lsm6dsl gyro */
    //sensor_sample_fetch_chan(imu_dev, SENSOR_CHAN_GYRO_XYZ);
    sensor_channel_get(imu_dev, SENSOR_CHAN_GYRO_X, &gyro_x);
    sensor_channel_get(imu_dev, SENSOR_CHAN_GYRO_Y, &gyro_y);
    sensor_channel_get(imu_dev, SENSOR_CHAN_GYRO_Z, &gyro_z);

    sprintf(out_str, "gyro x:%f dps y:%f dps z:%f dps", sensor_value_to_double(&gyro_x),
            sensor_value_to_double(&gyro_y), sensor_value_to_double(&gyro_z));

    accel->x = sensor_value_to_double(&accel_x);
    accel->y = sensor_value_to_double(&accel_y);
    accel->z = sensor_value_to_double(&accel_z);

    gyro->x = sensor_value_to_double(&gyro_x);
    gyro->y = sensor_value_to_double(&gyro_x);
    gyro->z = sensor_value_to_double(&gyro_x);
    
    return 0;
}
