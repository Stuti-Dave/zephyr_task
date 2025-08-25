/**
 * @file logger.c
 * @brief Logs the data to file in littlefs partition.
 *
 * This file provides the functionality of logging the data 
 * in the shared buffer of sensors to a file created in a little-fs 
 * partition.
 *
 * @date 15-08-2025
 * author Stuti Dave
 */

//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>
#include <zephyr/logging/log.h>
#include <zephyr/storage/flash_map.h>

#include "main.h"
#include <errno.h>

LOG_MODULE_REGISTER(logger);

extern struct k_msgq ht_sensor_msgq;
extern struct k_msgq lp_sensor_msgq;
extern struct k_msgq imu_sensor_msgq;

#define SENSORS_BUF_SIZE		sizeof(sensors_shared_buf)
#define SENSORS_THREADS_PRIORITY	5
#define LOGGER_THREAD_STACK_SIZE	1024

void logger_func(sensors_shared_buf *shared_buf)
{
	struct fs_file_t file;
	fs_file_t_init(&file);
	
	int ret = fs_open(&file, "/lfs/sensor.log",FS_O_CREATE | FS_O_WRITE | FS_O_APPEND);
	if (ret < 0) {
		LOG_ERR("Failed to open file: %d", ret);
		return;
	}

	ret = fs_write(&file, shared_buf, SENSORS_BUF_SIZE);
	if (ret < 0) {
		LOG_ERR("Failed to write file: %d", ret);
	} else {
		fs_write(&file, "\n", 1);
	}
	fs_close(&file);
}

void logger_thread(void *, void *, void *)
{
	sensors_shared_buf shared_buf;
	LOG_INF("Logger Thread started");
	while (1) {
		k_msgq_get(&ht_sensor_msgq, &shared_buf.hts_data, K_MSEC(1000));
		k_msgq_get(&lp_sensor_msgq, &shared_buf.lps_data, K_MSEC(1000));
		k_msgq_get(&imu_sensor_msgq, &shared_buf.imu_data, K_MSEC(1000));
			
		logger_func(&shared_buf);
		k_sleep(K_SECONDS(60));
	}
}

K_THREAD_DEFINE(logger_tid, LOGGER_THREAD_STACK_SIZE, logger_thread, NULL, NULL, NULL, SENSORS_THREADS_PRIORITY, 0, 0);


/* *
 * Mount Little FS 
 * */

FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(lfs1);
/* LittleFS mount point */
static struct fs_mount_t lfs_mount_pt = {
	.type = FS_LITTLEFS,
	.fs_data = &lfs1,
	.storage_dev = (void *)FIXED_PARTITION_ID(lfs1_partition),
	.mnt_point = "/lfs",
};

static int littlefs_flash_erase(unsigned int id)
{
        const struct flash_area *pfa;
        int rc;

        rc = flash_area_open(id, &pfa);
        if (rc < 0) {
                LOG_ERR("FAIL: unable to find flash area %u: %d\n",id, rc);
                return rc;
        }

        LOG_INF("Area %u at 0x%x on %s for %u bytes\n",
                   id, (unsigned int)pfa->fa_off, pfa->fa_dev->name,(unsigned int)pfa->fa_size);

        /* Optional wipe flash contents */
        rc = flash_area_flatten(pfa, 0, pfa->fa_size);
        LOG_INF("Erasing flash area ... %d", rc);

        flash_area_close(pfa);
        return rc;
}

static int littlefs_mount(struct fs_mount_t *mp)
{
        int rc;

        rc = littlefs_flash_erase((uintptr_t)mp->storage_dev);
        if (rc < 0) {
                return rc;
        }

        /* Do not mount if auto-mount has been enabled */
        rc = fs_mount(mp);
        if (rc < 0) {
                LOG_ERR("FAIL: mount id %" PRIuPTR " at %s: %d\n",
                       (uintptr_t)mp->storage_dev, mp->mnt_point, rc);
                return rc;
        }
        LOG_INF("%s mount: %d\n", mp->mnt_point, rc);
	return 0;
}

int logger_init(void)
{
	int rc;
	    rc = fs_mount(&lfs_mount_pt);
	    if (rc < 0) {
		    LOG_ERR("FAIL: mount id %" PRIuPTR " at %s: %d",(uintptr_t)lfs_mount_pt.storage_dev, lfs_mount_pt.mnt_point, rc);
		    return rc;
	    }
	    LOG_INF("%s mount: %d", lfs_mount_pt.mnt_point, rc);

	    /* Create all 4 threads */
	    hts_init();
	    lps_init();
	    imu_sensor_init();

	    return 0;
}
