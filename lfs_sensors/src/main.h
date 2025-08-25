#ifndef MAIN_H
#define MAIN_H

/* All the sensor structure definitions */
typedef struct {
        double humidity;
        double temperature;
} hum_temp_data;

typedef struct {
        double pressure;
} press_data;

typedef struct {
    double x, y, z;
} imu_data_t;

typedef struct {
	imu_data_t accel;
	imu_data_t gyro;
} imu_sensor_data;

typedef struct {
        hum_temp_data hts_data;
        press_data lps_data;
        imu_sensor_data imu_data;
} sensors_shared_buf;

int hts_init(void);
int lps_init(void);
int imu_sensor_init(void);
int logger_init(void);
#endif
