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

#if FIXED_PARTITION_EXISTS(lfs1_partition)
#define LITTLE_FS FIXED_PARTITION_ID(lfs1_partition)
#else
#error("Partition not found")
#endif

LOG_MODULE_REGISTER(logger, CONFIG_LOG_DEFAULT_LEVEL);

/* Shared buffer from sensors */
extern struct shared_buf sensors_shared_buf;

/* Mount structure for littlefs */
FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(lfs_storage);
static struct fs_mount_t lfs_mount = 
{
	.type = FS_LITTLEFS,
	.mnt_point = "/lfs1",
	.fs_data = &lfs_storage;
	.storage_dev = LITTLE_FS;
};

static struct k_mutex log_mutex;

/* Logger thread function */
static void logger_thread(void *a, void *b, void *c)
{
	struct fs_file_t file;
	int ret;


	while (1) {
	}
}

/* Define logger thread */
K_THREAD_DEFINE(logger_tid, 2048, logger_thread, NULL, NULL, NULL, 5, 0, 0);

/* Init function */
void logger_init(void)
{
	k_mutex_init(&log_mutex);
}
