#include "sensors.h"
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <sys/printk.h>

extern struct data_buffer data_buf;
extern struct k_mutex buf_mutex;

/* Try to get the device by DT (common helper). If the symbol isn't present in your Zephyr tree,
   you may need to replace DEVICE_DT_GET_ANY(st_hts221) with the correct compatible node.
*/
#define HTS221_NODE DT_NODELABEL(hts221)

static const struct device *dev =HTS221_NODE;

int get_hts221_data(struct hts_data *out)
{
	if(!device_is_ready(dev))
		return -1;
	sensor_sample_fetch(dev);
	
	struct sensor_value temp, hum;
	sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
	sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &hum);

	out->temperature=sensor_value_to_double(&temp);
	out->humidity=sensor_value_to_double(&hum);
	return 0;
}

void hts221_thread(void *arg1,void *arg2,void *arg3){
	dev = DEVICE_DT_GET(HTS221_NODE);

	if(device_is_ready(dev)){
		while(1){
			struct hts_data read_data={0};
			if(dev && device_is_ready(dev)==true){
				if(get_hts221_reading(&read_data)==0){
					k_mutex_lock(&buf_mutex, K_FOREVER);
					data_buf.hts=read_data;
					k_mutex_unlock(&buf_mutex);
				}
			}
			k_msleep(1000);
		}
	}
}
