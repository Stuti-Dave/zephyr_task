#ifndef THREAD_STRUCT_H
#define THREAD_STRUCT_H

#include <stdint.h>

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

/**
 * @brief Start sensor threads.
 *
 * This function initializes and starts the sensor threads
 * that update the shared buffer.
 */
void sensor_threads(void);

#endif	/* THREAD_STRUCT_H */
