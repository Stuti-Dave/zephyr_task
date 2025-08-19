/**
 * @file logger.c
 * @brief Logger thread with file rotation using LittleFS.
 *
 * @date 18-08-2025
 * @author Stuti Dave
 */

#include <zephyr/kernel.h>
#include <zephyr/fs/fs.h>
#include <zephyr/logging/log.h>
#include <zephyr/storage/flash_map.h>
#include <zephyr/fs/littlefs.h>
#include "thread_struct.h"

LOG_MODULE_REGISTER(logger, CONFIG_LOG_DEFAULT_LEVEL);

/* Shared buffer from sensors */
extern struct shared_buf g_shared_buf;

/* Mount structure for littlefs */
FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(lfs_storage);
static struct fs_mount_t littlefs_mnt = {
    .type = FS_LITTLEFS,
    .mnt_point = "/lfs",
    .fs_data = &lfs_storage,
};

static struct k_mutex log_mutex;

/* Logger thread function */
static void logger_thread(void *a, void *b, void *c)
{
	struct fs_file_t file;
	int ret;

	/* Mount FS */
	ret = fs_mount(&littlefs_mnt);
	if (ret < 0) {
		LOG_ERR("Mount failed: %d", ret);
		return;
	}

	fs_file_t_init(&file);

	while (1) {
		k_mutex_lock(&log_mutex, K_FOREVER);

		ret = fs_open(&file, "/lfs/log.txt", FS_O_CREATE | FS_O_WRITE | FS_O_APPEND);
		if (ret < 0) {
			LOG_ERR("File open failed: %d", ret);
			k_mutex_unlock(&log_mutex);
			k_msleep(1000);
			continue;
		}
		char buf[128];
		int len = snprintk(buf, sizeof(buf),
                           "T: %.2f H: %.2f P: %.2f "
                           "AX: %.2f AY: %.2f AZ: %.2f "
                           "GX: %.2f GY: %.2f GZ: %.2f\n",
                           g_shared_buf.temp, g_shared_buf.hum, g_shared_buf.pressure,
                           g_shared_buf.accel.x, g_shared_buf.accel.y, g_shared_buf.accel.z,
                           g_shared_buf.gyro.x, g_shared_buf.gyro.y, g_shared_buf.gyro.z);

		ret = little_fs_write(&file, buf, len);
		if (ret < 0) {
			LOG_ERR("Write failed: %d", ret);
		}

		k_mutex_unlock(&log_mutex);

		k_msleep(1000);
	}
}

/* Define logger thread */
K_THREAD_DEFINE(logger_tid, 2048, logger_thread, NULL, NULL, NULL, 5, 0, 0);

/* Init function */
void logger_init(void)
{
    k_mutex_init(&log_mutex);
}
