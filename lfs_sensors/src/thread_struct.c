/**
 * @file sensor_threads.c
 * @brief Sensor threads for humidity/temperature, pressure, and IMU.
 *
 * This module defines three threads, each responsible for reading
 * sensor data (humidity/temperature, pressure, IMU). The results are
 * stored in a shared buffer protected by a semaphore to ensure
 * thread-safe access.
 *
 * @date 15-08-2025
 * @author Stuti Dave
 */

#include <zephyr/kernel.h>
#include <stdint.h>

/* -------------------------------------------------------------------------- */
/* Structure and shared buffer definitions for threads                        */
/* -------------------------------------------------------------------------- */

/**
 * @brief Structure for storing IMU sensor data (x, y, z).
 */
typedef struct {
    double x;
    double y;
    double z;
}imu_sensor_data;

/**
 * @brief Shared buffer for sensor readings.
 *
 * This structure stores humidity, temperature, pressure,
 * accelerometer, and gyroscope values.
 */
struct shared_buf {
    double hum;
    double temp;
    double pressure;
    imu_sensor_data accel;
    imu_sensor_data gyro;
};

/* -------------------------------------------------------------------------- */
/* External Sensor Functions (Implemented in Sensor Drivers)                  */
/* -------------------------------------------------------------------------- */

/**
 * @brief Fetch humidity and temperature readings.
 *
 * @param[out] temp Pointer to store temperature reading.
 * @param[out] hum  Pointer to store humidity reading.
 * @return 0 on success, -1 on failure.
 */
int hum_temp_process(double *temp, double *hum);

/**
 * @brief Fetch pressure reading.
 *
 * @param[out] press Pointer to store pressure reading.
 * @return 0 on success, -1 on failure.
 */
int pressure_sensor_process(double *press);

/**
 * @brief Fetch accelerometer and gyroscope readings.
 *
 * @param[out] accel Pointer to store accelerometer data (x, y, z).
 * @param[out] gyro  Pointer to store gyroscope data (x, y, z).
 * @return 0 on success, -1 on failure.
 */
int imu_sensor_process(imu_sensor_data *accel, imu_sensor_data *gyro);

/* -------------------------------------------------------------------------- */
/* Shared Resources                                                           */
/* -------------------------------------------------------------------------- */

/** Shared buffer holding latest sensor readings. */
struct shared_buf sensors_shared_buf;

/** Semaphore to protect access to shared buffer. */
struct k_sem buffer_sem;

/* -------------------------------------------------------------------------- */
/* Sensor Threads                                                             */
/* -------------------------------------------------------------------------- */

/**
 * @brief Thread for humidity and temperature sensor.
 *
 * Periodically fetches humidity and temperature values and updates
 * the shared buffer safely using the semaphore.
 */
static void hum_temp_thread(void *a, void *b, void *c)
{
	while (1) {
		k_sem_take(&buffer_sem, K_FOREVER);
		hum_temp_process(&sensors_shared_buf.temp, &sensors_shared_buf.hum);
		k_sem_give(&buffer_sem);
		k_msleep(1000); /* 1 second delay */
	}
}

/**
 * @brief Thread for pressure sensor.
 *
 * Periodically fetches pressure value and updates
 * the shared buffer safely using the semaphore.
 */
static void pressure_thread(void *a, void *b, void *c)
{
	while (1) {
		k_sem_take(&buffer_sem, K_FOREVER);
		pressure_sensor_process(&sensors_shared_buf.pressure);
		k_sem_give(&buffer_sem);
		k_msleep(1000); /* 1 second delay */
	}
}

/**
 * @brief Thread for IMU sensor (accelerometer + gyroscope).
 *
 * Periodically fetches accelerometer and gyroscope values and updates
 * the shared buffer safely using the semaphore.
 */
static void imu_thread(void *a, void *b, void *c)
{
	while (1) {
		k_sem_take(&buffer_sem, K_FOREVER);
		imu_sensor_process(&sensors_shared_buf.accel, &sensors_shared_buf.gyro);
		k_sem_give(&buffer_sem);
		k_msleep(1000); /* 1 second delay */
	}
}

/* -------------------------------------------------------------------------- */
/* Thread Definitions (Auto-start with K_THREAD_DEFINE)                       */
/* -------------------------------------------------------------------------- */

K_THREAD_DEFINE(hum_temp_tid, 1024, hum_temp_thread, NULL, NULL, NULL, 5, 0, 0);
K_THREAD_DEFINE(pressure_tid, 1024, pressure_thread, NULL, NULL, NULL, 5, 0, 0);
K_THREAD_DEFINE(imu_tid, 1024, imu_thread, NULL, NULL, NULL, 5, 0, 0);

/* -------------------------------------------------------------------------- */
/* Initialization                                                             */
/* -------------------------------------------------------------------------- */

/**
 * @brief Initialize sensor threads and synchronization mechanisms.
 *
 * This function must be called before sensor threads start updating
 * the shared buffer. It initializes the semaphore used to guard
 * concurrent access.
 */
void sensor_threads(void)
{
	k_sem_init(&buffer_sem, 1, 1);
}
