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

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/fs/fs.h>
#include <zephyr/fs/littlefs.h>
#include <zephyr/logging/log.h>
#include <zephyr/storage/flash_map.h>
#include <errno.h>
#include <stdio.h>

//==============================================================================
// Logging Module Register
//==============================================================================

LOG_MODULE_REGISTER(logger);

//==============================================================================
// Structure definitons of sensors
//==============================================================================

/*
 * Structures of HTS221, LPS22HB and LSM6DSL
 */

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

//==============================================================================
// External Sensor Queues
//==============================================================================
//
extern struct k_msgq ht_sensor_msgq;
extern struct k_msgq lp_sensor_msgq;
extern struct k_msgq imu_sensor_msgq;

//==============================================================================
// Configuration Constants
//==============================================================================

/*
 * Constants for sensor data struct and thread
 */
#define SENSORS_BUF_SIZE		sizeof(sensors_shared_buf)
#define SENSORS_THREADS_PRIORITY	5
#define LOGGER_THREAD_STACK_SIZE	(2 * 1024)

/*
 * Constants for File
 */
#define FILE_SIZE			1024		// Max file size in bytes
#define MAX_FILES			10		// Maximum number of log files

//==============================================================================
// Function Prototypes
//==============================================================================

static void logger_func(sensors_shared_buf *shared_buf);
static void logger_thread(void *, void *, void *);

//==============================================================================
// Internal Helper Functions
//==============================================================================

/**
 * @brief Print a single sensor data record.
 *
 * Logs formatted output of all sensor values for human readability.
 *
 * Input:  fptr			Index of the sample in the file.
 * 	   sensor_buffer	Pointer to the sensor buffer to print.
 */
static void print_sensor_data(size_t *fptr, sensors_shared_buf *sensor_buffer)
{
	// Prints sensor data on console
	LOG_INF("|Sample%d |	Humidity: %.2f	|	Temperature: %.2f |	Pressure: %.2f	|	Accel: [x:%.2f, y:%.2f, z:%.2f]	|	Gyro: [x:%.2f, y:%.2f, z:%.2f] |", 
			*fptr, sensor_buffer->hts_data.humidity, sensor_buffer->hts_data.temperature, sensor_buffer->lps_data.pressure,
			sensor_buffer->imu_data.accel.x, sensor_buffer->imu_data.accel.y, sensor_buffer->imu_data.accel.z, 
			sensor_buffer->imu_data.gyro.x, sensor_buffer->imu_data.gyro.y, sensor_buffer->imu_data.gyro.z);
}

//==============================================================================
// Function Definitions
//==============================================================================

/**
 * @brief Append sensor data to a log file.
 *
 * Opens a LittleFS file, appends new sensor data, and if the file size exceeds
 * a limit, rotates to a new file. Afterwards, it re-reads the file contents
 * and prints them for verification.
 *
 * Input: shared_buf Pointer to the data structure containing all sensor data.
 */


static void logger_func(sensors_shared_buf *shared_buf)
{
	struct fs_file_t file;
	fs_file_t_init(&file);

	char filename[32];
	uint8_t file_num = 1;
	snprintf(filename, sizeof(filename), "/lfs/sensor%d.log", file_num);
	
	int ret = fs_open(&file, filename, FS_O_CREATE | FS_O_WRITE | FS_O_APPEND);
	if (ret < 0) {
		LOG_ERR("Failed to open file /lfs/sensor%d.log: %d", file_num,ret);
		return;
	}

	struct fs_dirent file_entry;
	fs_stat(filename, &file_entry);
	if (file_entry.size >= FILE_SIZE) {
		fs_close(&file);
		
		file_num++;
		
		snprintf(filename, sizeof(filename), "/lfs/sensor%d.log", file_num);
		fs_file_t_init(&file);

		ret = fs_open(&file, filename, FS_O_CREATE | FS_O_WRITE | FS_O_APPEND);
		if (ret < 0) {
			LOG_ERR("Failed to open file /lfs/sensor%d.log: %d", file_num,ret);
			return;
		}
		ret = fs_write(&file, shared_buf, sizeof(sensors_shared_buf));
		file_num++;
	} else {
		ret = fs_write(&file, shared_buf, sizeof(sensors_shared_buf));
                if (ret != sizeof(sensors_shared_buf)) {
                        LOG_ERR("Failed to write file /lfs/sensors.log: %d", ret);
                        // need to close file and return

                        fs_close(&file);
                        return;
                }
	}

	off_t current_fp = fs_tell(&file);
	current_fp = fs_tell(&file);
	fs_seek(&file, 0, FS_SEEK_SET);
	sensors_shared_buf temp_buf = {0};

	for (size_t fptr = 0; fptr < (current_fp / sizeof(sensors_shared_buf)); fptr++ ) {
		if ( fs_read(&file, &temp_buf, sizeof(sensors_shared_buf)) != sizeof(sensors_shared_buf)) {
			LOG_ERR("Incorrect read");
			fs_close(&file);

			return;
		}
		// print sensor data
		print_sensor_data(&fptr, &temp_buf);
	}
	fs_close(&file);
}

//==============================================================================
// Thread Definition
//==============================================================================

/*
 * Definition of Logger thread
 */

K_THREAD_DEFINE(logger_tid, LOGGER_THREAD_STACK_SIZE, logger_thread, 
		NULL, NULL, NULL, 
		SENSORS_THREADS_PRIORITY, 0, 1000);

//==============================================================================
// Thread Function
//==============================================================================

/**
 * @brief Logger thread.
 *
 * Collects data from all sensor queues, aggregates into a single buffer,
 * and writes it into LittleFS periodically.
 */

void logger_thread(void *, void *, void *)
{
	sensors_shared_buf shared_buf;
	LOG_INF("Logger Thread started");
	while (1) {
		k_msgq_get(&ht_sensor_msgq, &shared_buf.hts_data, K_SECONDS(10));
		k_msgq_get(&lp_sensor_msgq, &shared_buf.lps_data, K_SECONDS(10));
		k_msgq_get(&imu_sensor_msgq, &shared_buf.imu_data, K_SECONDS(10));
			
		logger_func(&shared_buf);
		k_sleep(K_SECONDS(60));
	}
}

//==============================================================================
// LittleFS Mount & Flash Management
//==============================================================================

/* Default LittleFS Configuration */
FS_LITTLEFS_DECLARE_DEFAULT_CONFIG(lfs1);

/* LittleFS mount point */
static struct fs_mount_t lfs_mount_pt = {
	.type = FS_LITTLEFS,
	.fs_data = &lfs1,
	.storage_dev = (void *)FIXED_PARTITION_ID(lfs1_partition),
	.mnt_point = "/lfs",
};

/**
 * @brief Erase flash area used by LittleFS.
 *
 * Input: id Partition ID of the flash area to erase.
 *
 * Returns: 0  Success
 * 	   <0  Error code
 */

static int littlefs_flash_erase(unsigned int id)
{
        const struct flash_area *pfa;
        int rc;

        rc = flash_area_open(id, &pfa);
        if (rc < 0) {
                LOG_ERR("FAIL: unable to find flash area %u: %d\n",id, rc);
                return rc;
        }

        LOG_INF("Area %u at 0x%x on %s for %u bytes\n", id, (unsigned int)pfa->fa_off, pfa->fa_dev->name,(unsigned int)pfa->fa_size);

        /* Optional wipe flash contents */
        rc = flash_area_flatten(pfa, 0, pfa->fa_size);
        LOG_INF("Erasing flash area ... %d", rc);

        flash_area_close(pfa);
        return rc;
}

/**
 * @brief Mount LittleFS at given mount point.
 *
 * Attempts to mount; if it fails, erases flash and retries.
 *
 * Input: mp Mount point structure.
 *
 * Returns:  0  Success
 *	    <0  Error code
 */

static int littlefs_mount(struct fs_mount_t *mp)
{
        int rc;

        /* Do not mount if auto-mount has been enabled */
        rc = fs_mount(mp);
        if (rc < 0) {
		rc = littlefs_flash_erase((uintptr_t)mp->storage_dev);
		if (rc < 0) {
			LOG_ERR("Flash erase failed.");
			return rc;
		}
		rc = fs_mount(mp);
                if (rc < 0) {
			LOG_ERR("FAIL: mount id %" PRIuPTR " at %s: %d\n",(uintptr_t)mp->storage_dev, mp->mnt_point, rc);
			return rc;
		}
		LOG_INF("%s is mounted: %d\n", mp->mnt_point, rc);
        } else {
		LOG_INF("%s is mounted: %d\n", mp->mnt_point, rc);
	}
	return 0;
}

//==============================================================================
// Logger Initialization
//==============================================================================

/**
 * @brief Initialize logger module.
 *
 * Mounts LittleFS filesystem before starting logging operations.
 *
 * Returns: 0  Success
 * 	   <0  Error code
 */

int logger_init(void)
{
	int rc;
	    rc = littlefs_mount(&lfs_mount_pt);
	    if (rc < 0) {
		    LOG_ERR("FAIL: mount id %" PRIuPTR " at %s: %d",(uintptr_t)lfs_mount_pt.storage_dev, lfs_mount_pt.mnt_point, rc);
		    return rc;
	    }
	    LOG_INF("%s is mounted: %d", lfs_mount_pt.mnt_point, rc);

	    return 0;
}
