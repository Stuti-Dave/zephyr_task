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
#include <zephyr/kernel.h>
#include "main.h"
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <errno.h>

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

#define MSG_SIZE	sizeof(imu_sensor_data)
#define MAX_MSGS	10
#define MSG_ALIGN	32
#define SENSOR_PRIORITY	5
#define IMU_THREAD_STACK_SIZE	1024

struct k_msgq imu_sensor_msgq;
K_MSGQ_DEFINE(imu_sensor_msgq, MSG_SIZE, MAX_MSGS, MSG_ALIGN);
//==============================================================================
// Private Function Prototypes
//==============================================================================

int imu_sensor_process(imu_sensor_data *sensor_data);
void imu_thread(void *, void *, void *);

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

int imu_sensor_process(imu_sensor_data *sensor_data)
{

	if (sensor_sample_fetch(imu_dev) < 0) {
		LOG_ERR("sensor: %s sample update error", imu_dev->name);
		return -1;
	}

	/* Fetch channel for accelerometer and gyrometer */
	if (sensor_sample_fetch_chan(imu_dev, SENSOR_CHAN_ACCEL_XYZ) < 0) {
		LOG_ERR("Sensor: %s fetch failed.", imu_dev->name);
		return -1;
	}
	if (sensor_sample_fetch_chan(imu_dev, SENSOR_CHAN_GYRO_XYZ) < 0) {
		LOG_ERR("Sensor: %s fetch failed.", imu_dev->name);
		return -1;
	}
	
	struct sensor_value accel_x, accel_y, accel_z;
	struct sensor_value gyro_x, gyro_y, gyro_z;

	/* lsm6dsl accel */
	sensor_channel_get(imu_dev, SENSOR_CHAN_ACCEL_X, &accel_x);
	sensor_channel_get(imu_dev, SENSOR_CHAN_ACCEL_Y, &accel_y);
	sensor_channel_get(imu_dev, SENSOR_CHAN_ACCEL_Z, &accel_z);


	/* lsm6dsl gyro */
	sensor_channel_get(imu_dev, SENSOR_CHAN_GYRO_X, &gyro_x);
	sensor_channel_get(imu_dev, SENSOR_CHAN_GYRO_Y, &gyro_y);
	sensor_channel_get(imu_dev, SENSOR_CHAN_GYRO_Z, &gyro_z);

	/* Filling data to sensor structure*/
	sensor_data->accel.x = sensor_value_to_double(&accel_x);
	sensor_data->accel.y = sensor_value_to_double(&accel_y);
	sensor_data->accel.z = sensor_value_to_double(&accel_z);

	sensor_data->gyro.x = sensor_value_to_double(&gyro_x);
	sensor_data->gyro.y = sensor_value_to_double(&gyro_x);
	sensor_data->gyro.z = sensor_value_to_double(&gyro_x);

	return 0;
}


K_THREAD_DEFINE(imu_tid, IMU_THREAD_STACK_SIZE, imu_thread, NULL, NULL, NULL, SENSOR_PRIORITY, 0, 0);

void imu_thread(void *, void *, void *)
{
	LOG_INF("IMU sensor thread started");
	imu_sensor_data sensor_data;

	while(1) {
		if(imu_sensor_process(&sensor_data) == 0) {
			k_msgq_put(&imu_sensor_msgq, &sensor_data, K_MSEC(1000));
		}
		k_sleep(K_MSEC(1000));
	}
}


int imu_sensor_init(void)
{
	if (!device_is_ready(imu_dev)) {
		LOG_ERR("sensor: device not ready.\n");
		return -1;
	}
	
	struct sensor_value odr_attr;
        /* set accel/gyro sampling frequency to 104 Hz */
        odr_attr.val1 = 104;
        odr_attr.val2 = 0;

        if (sensor_attr_set(imu_dev, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_SAMPLING_FREQUENCY, &odr_attr) < 0) {
                LOG_ERR("Cannot set sampling frequency for accelerometer.\n");
                return -1;
        }

        if (sensor_attr_set(imu_dev, SENSOR_CHAN_GYRO_XYZ, SENSOR_ATTR_SAMPLING_FREQUENCY, &odr_attr) < 0) {
                LOG_ERR("Cannot set sampling frequency for gyro.\n");
                return -1;
        }
	LOG_INF("IMU sensor Initialized.");
	
	return 0;
}

