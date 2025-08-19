/**
 * @file main.c
 * @author Ankit Modi (ankit.5036@gmail.com)
 * @brief Entry point for the Zephyr LVGL Showcase Application.
 *
 * This file contains the main function for the Zephyr LVGL Showcase Application.
 * It initializes the logging module and prints a startup message indicating
 * the board on which the application is running.
 *
 * @version 0.1
 * @date 12-08-2025
 *
 * @copyright Copyright (c) 2025
 *
 */
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include "thread_struct.h"

//------------------------------------------------------------------------------
// Private Variables
//------------------------------------------------------------------------------

LOG_MODULE_REGISTER(main);

//------------------------------------------------------------------------------
// External Variables
//------------------------------------------------------------------------------

extern struct shared_buf sensors_shared_buf;
//extern void logger_init(void);

//------------------------------------------------------------------------------
// Main Entry Function
//------------------------------------------------------------------------------

/**
 * @brief Main entry point of the application.
 * @return int Returns 0 upon successful execution.
 */

int main(void)
{
    LOG_INF("Zephyr LVGL Showcase Application Started on %s.", CONFIG_BOARD);
    
    /* Init logger mutex */
    //logger_init();
    
    /* Initialize sensor threads (semaphore + K_THREAD_DEFINE inside) */
    sensor_threads();

    while (1) {
        /* Print current shared buffer values */
        LOG_INF("Hum=%.2f Temp=%.2f Press=%.2f "
                "Accel[x=%.2f y=%.2f z=%.2f] "
                "Gyro[x=%.2f y=%.2f z=%.2f]",
                sensors_shared_buf.hum,
                sensors_shared_buf.temp,
                sensors_shared_buf.pressure,
                sensors_shared_buf.accel.x, sensors_shared_buf.accel.y, sensors_shared_buf.accel.z,
                sensors_shared_buf.gyro.x, sensors_shared_buf.gyro.y, sensors_shared_buf.gyro.z);

        k_msleep(2000); /* print every 2s */
    }
    return 0;
}
