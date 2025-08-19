#ifndef SENSOR_DATA_H
#define SENSOR_DATA_H

#include <zephyr/kernel.h>
#include <stdint.h>
#include <stdbool.h>

struct hts_data{
	double temperature;
	double humidity;
};

struct lps_data{
	double pressure_hpa;
};

struct lsm_data{
	double x_axes, y_axes, z_axes;
};

struct data_buffer{
	struct hts_data hts;
	struct lps_data lps;
	struct lsm_data lsm;
};

/* Exported globals */
extern struct data_buffer data_buf;
extern struct k_mutex buf_mutex;

/*HTS221*/
int get_hts221_data(struct hts_data *out);
void hts221_thread(void *arg1, void *arg2, void *arg3);

/* LPS22HB*/
int get_lps22hb_data(struct lps_data *out);
void lps22hb_thread(void *arg1, void *arg2, void *arg3);

/*LSM6DSL*/
int get_lsm6dsl_data(struct lsm_data *out);
void lsm6dsl_thread(void *arg1, void *arg2, void *arg3);

#endif
