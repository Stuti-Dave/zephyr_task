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
// Device Tree Bindings
//==============================================================================

#if DT_NODE_EXISTS(DT_ALIAS(imu_sensor))
#define IMU_NODE DT_ALIAS(imu_sensor)
static const struct device *const imu_dev = DEVICE_DT_GET(IMU_NODE);
#else
#error("IMU sensor not found in device tree.")
#endif

//==============================================================================
// Logging Module Register
//==============================================================================

LOG_MODULE_REGISTER(imu, LOG_LEVEL_DBG);

//==============================================================================
// Sensor-data structure
//==============================================================================

typedef struct {
    double x, y, z;
} imu_data_t;

typedef struct {
        imu_data_t accel;
        imu_data_t gyro;
} imu_sensor_data;

//==============================================================================
// Configuration Constants
//==============================================================================

#define MAX_MSGS		10	// Maximum number of sensor samples in queue
#define MSGQ_ALIGN		32	// Align message queue entries
#define SENSOR_PRIORITY		5	// Thread priority for sensor task
#define IMU_THREAD_STACK_SIZE	1024	// Stack size for sensor thread

//==============================================================================
// Message Queue
//==============================================================================

// Queue for transferring Acelerometer/Gyroscope readings between threads

struct k_msgq imu_sensor_msgq;
K_MSGQ_DEFINE(imu_sensor_msgq, MSG_SIZE, MAX_MSGS, MSG_ALIGN);

//==============================================================================
// Function Prototypes
//==============================================================================

int imu_sensor_process(imu_sensor_data *sensor_data);
void imu_thread(void *, void *, void *);

//==============================================================================
// Function Definition
//==============================================================================

/**
 * @brief Fetch and process IMU sensor readings (accelerometer + gyroscope).
 *
 * This function checks whether the IMU sensor device is ready,
 * fetches the latest sample, and extracts both acceleration and gyroscope
 * values across all three axes (X, Y, Z).
 *
 * Input:  data_struct Pointer to a imu_data_t struct where accelerometer and gyroscope data will be stored.
 *
 * Returns: 0  Success, values stored in accel and gyro elements.
 * 	   -1  Failure, error logged.
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

	/* Get sensor channel for lsm6dsl accel */
	sensor_channel_get(imu_dev, SENSOR_CHAN_ACCEL_X, &accel_x);
	sensor_channel_get(imu_dev, SENSOR_CHAN_ACCEL_Y, &accel_y);
	sensor_channel_get(imu_dev, SENSOR_CHAN_ACCEL_Z, &accel_z);


	/* Get sensor channel for lsm6dsl gyro */
	sensor_channel_get(imu_dev, SENSOR_CHAN_GYRO_X, &gyro_x);
	sensor_channel_get(imu_dev, SENSOR_CHAN_GYRO_Y, &gyro_y);
	sensor_channel_get(imu_dev, SENSOR_CHAN_GYRO_Z, &gyro_z);

	/* Store accel and gyro readings to the data struct */
	sensor_data->accel.x = sensor_value_to_double(&accel_x);
	sensor_data->accel.y = sensor_value_to_double(&accel_y);
	sensor_data->accel.z = sensor_value_to_double(&accel_z);

	sensor_data->gyro.x = sensor_value_to_double(&gyro_x);
	sensor_data->gyro.y = sensor_value_to_double(&gyro_x);
	sensor_data->gyro.z = sensor_value_to_double(&gyro_x);

	return 0;
}


//==============================================================================
// Thread Definition
//==============================================================================

/*
 * Definition of Accelerometer and Gyroscope thread
 */

K_THREAD_DEFINE(imu_tid, IMU_THREAD_STACK_SIZE, imu_thread, 
		NULL, NULL, NULL, 
		SENSOR_PRIORITY, 0, 0);

//==============================================================================
// Thread Implementation
//==============================================================================

/**
 * @brief Thread function for IMU sampling.
 *
 * Workflow:
 *  1. Ensure the IMU is ready.
 *  2. Configure sensor attributes (sampling frequency).
 *  3. Periodically fetch accel + gyro readings.
 *  4. Publish results to the message queue.
 */

void imu_thread(void *, void *, void *)
{
	LOG_INF("IMU sensor thread started");
	imu_sensor_data sensor_data;

	if (!device_is_ready(imu_dev)) {
                LOG_ERR("sensor: device not ready.\n");
                return;
        }

        struct sensor_value odr_attr;
        /* set accel/gyro sampling frequency to 104 Hz */
        odr_attr.val1 = 104;
        odr_attr.val2 = 0;

        if (sensor_attr_set(imu_dev, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_SAMPLING_FREQUENCY, &odr_attr) < 0) {
                LOG_ERR("Cannot set sampling frequency for accelerometer.\n");
                return;
        }

        if (sensor_attr_set(imu_dev, SENSOR_CHAN_GYRO_XYZ, SENSOR_ATTR_SAMPLING_FREQUENCY, &odr_attr) < 0) {
                LOG_ERR("Cannot set sampling frequency for gyro.\n");
                return;
        }
        LOG_INF("IMU sensor Initialized.");

	while(1) {
		if(imu_sensor_process(&sensor_data) == 0) {
			k_msgq_put(&imu_sensor_msgq, &sensor_data, K_MSEC(1000));
		}
		LOG_DBG("Accel: {x:%.2f y:%.2f z:%.2f], Gyro: [x:%.2f y:%.2f z:%.2f]", 
			sensor_data.accel.x, sensor_data.accel.y, sensor_data.accel.z, 
			sensor_data.gyro.x, sensor_data.gyro.y, sensor_data.gyro.z);
		k_sleep(K_MSEC(5000));
	}
}
