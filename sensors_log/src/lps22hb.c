#include "lps22hb_sensor.h"
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <sys/printk.h>

extern struct data_buffer data_buf;
extern struct k_mutex buf_mutex;

#define LPS_NODE DT_NODELABEL(lps22hb)

static const struct device *dev=LPS_NODE;

int get_lps22hb_reading(struct lps_reading *out){
	if(!device_is_ready(dev))
		return -1;
	sensor_sample_fetch(dev);
	
	struct sensor_value pressure;
	sensor_channel_get(dev, SENSOR_CHAN_PRESS, &pressure);

	out->pressure=sensor_value_to_double(&pressure);
	return 0;
}

void lps22hb_thread(void *arg1, void *arg2, void *arg3){
	dev=DEVICE_DT_GET(LPS_NODE);
	if(device_is_ready(dev)){
		while(1){
			struct lps_reading read_data={0};
			if(dev && device_is_ready(dev)){
				if(get_lps22hb_data(&read_data)== 0){
					k_mutex_lock(&buf_mutex, K_FOREVER);
					data_buf.lps=read_data;
					k_mutex_unlock(&g_buf_mutex);
				}
			}
			k_msleep(1000);
		}
	}
}
