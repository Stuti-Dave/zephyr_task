#include "lsm6dsl_sensor.h"
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <sys/printk.h>

extern struct data_buffer data_buf;
extern struct k_mutex buf_mutex;

#define LSM_NODE DT_NODELABEL(lsm6dsl)

static const struct device *dev;

int get_lsm6dsl_data(struct lsm_data *out){
	if(!device_is_ready(dev))
		return -1;

	sensor_sample_fetch(lsm_dev);

	struct sensor_value accel[3];
	sensor_channel_get(lsm_dev, SENSOR_CHAN_ACCEL_XYZ, accel);
	
	out->x_axes= sensor_value_to_double(&accel[0]);
	out->y_axes=sensor_value_to_double(&accel[1]);
	out->z_axes=sensor_value_to_double(&accel[2]);
	
	return 0;
}

void lsm6dsl_thread(void *arg1, void *arg2, void *arg3){
	dev=DEVICE_DT_GET(LSM_NODE);

	if(device_is_ready(dev)){
		while(1){
			struct lsm_data read_data={0};
			if(dev && device_is_ready(dev)){
				if(get_lsm6dsl_reading(&read_data)==0){
					k_mutex_lock(&buf_mutex, K_FOREVER);
					data_buf.lsm= read_data;
					k_mutex_unlock(&buf_mutex);
				}
			}
			k_msleep(100);
		}
	}
}
