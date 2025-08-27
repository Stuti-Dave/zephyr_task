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
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <errno.h>
#include <stdint.h>

//==============================================================================
// Device Tree Bindings
//==============================================================================

#if DT_NODE_EXISTS(DT_ALIAS(ht_sensor))
#define HUM_TEMP_NODE DT_ALIAS(ht_sensor)
static const struct device *const hts_dev = DEVICE_DT_GET(HUM_TEMP_NODE);
#else
#error("Humidity-Temperature sensor not found in device tree.")
#endif

//==============================================================================
// Logging Module Register
//==============================================================================

LOG_MODULE_REGISTER(hum_temp, CONFIG_APP_LOG_LEVEL);

//==============================================================================
// Configuration Constants
//==============================================================================

#define MAX_MSGS              10     // Maximum number of sensor samples in queue
#define MSGQ_ALIGN            32     // Align message queue entries
#define SENSOR_PRIORITY       5      // Thread priority for sensor task
#define HT_THREAD_STACK_SIZE  512    // Stack size for sensor thread

//==============================================================================
// Sensor-data structure
//==============================================================================

typedef struct {
        double humidity;
        double temperature;
} hum_temp_data;

//==============================================================================
// Message Queue
//==============================================================================

// Queue for transferring humidity/temperature readings between threads
struct k_msgq ht_sensor_msgq;
K_MSGQ_DEFINE(ht_sensor_msgq, sizeof(hum_temp_data), MAX_MSGS, MSGQ_ALIGN);

//==============================================================================
// Function Prototypes
//==============================================================================

static int hum_temp_process(hum_temp_data* data_struct);
static void hum_temp_thread(void *, void *, void *);

//==============================================================================
// Function Definitions
//==============================================================================

/**
 * @brief Fetch a humidity and temperature sample from the sensor.
 *
 * This function checks readiness, fetches the latest measurement,
 * and converts it to floating-point values stored in data_struct.
 *
 * Input: data_struct Pointer to store the fetched temperature & humidity.
 *
 * Returns: 0  Success, value stored in data_struct humidity and temperature elements.
 * 	   -1  Failure, error logged.
 */

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

	/* Store temperature and humidity readings to the data struct */
	data_struct->temperature = sensor_value_to_double(&temp);
	data_struct->humidity = sensor_value_to_double(&hum);

	return 0;
}


//==============================================================================
// Thread Definition
//==============================================================================

/*
 * Definition of Humidty and Temperature thread
 */

K_THREAD_DEFINE(hum_temp_tid, HT_THREAD_STACK_SIZE, hum_temp_thread, 
		NULL, NULL, NULL, 
		SENSOR_PRIORITY, 0, 0);

//==============================================================================
// Thread Function
//==============================================================================

/**
 * @brief Thread function for humidity-temperature sampling.
 *
 * Periodically:
 *  1. Validates sensor readiness.
 *  2. Reads humidity and temperature.
 *  3. Pushes results into the message queue.
 *  4. Sleeps before the next cycle.
 */

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
