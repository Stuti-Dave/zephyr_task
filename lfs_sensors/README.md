# Zephyr Sensor Threads Example

This project demonstrates how to use **multiple threads in Zephyr RTOS** to read data from different sensors, store results in a **shared buffer**, and print them using `LOG_INF`.

## Features

- **Three sensor threads**:
  - Humidity + Temperature (`hum_temp_thread`)
  - Pressure (`pressure_thread`)
  - IMU (`imu_thread`)

- **Shared buffer** (`struct shared_buf`):
  - Stores temperature, humidity, pressure, accelerometer, and gyroscope data.
  - Protected with a **semaphore** to ensure safe concurrent access.

- **Main thread**:
  - Initializes the sensor threads.
  - Periodically prints the shared buffer values.

## File Overview

- `thread_struct.h`  
  Defines `struct shared_buf` and `struct three_d` used for storing sensor data.

- `sensor_threads.c`  
  Implements the three sensor threads and initializes the semaphore.

- `main.c`  
  Entry point of the program. Starts sensor threads and logs the shared buffer values.

- `hum_temp.c`, `pressure.c`, `imu.c`  
  Contain the actual sensor reading functions.

## How It Works

1. On startup, `main()` calls `sensor_threads()` which:
   - Initializes a semaphore (`k_sem`).
   - Starts the three sensor threads via `K_THREAD_DEFINE`.

2. Each sensor thread:
   - Takes the semaphore.
   - Reads data from its sensor.
   - Updates the shared buffer.
   - Releases the semaphore.
   - Sleeps for 1 second before repeating.

3. The main loop:
   - Periodically logs the contents of the shared buffer every 2 seconds.

## 📅 TODO list

- [x] Add Temperature-Humidity sensor
- [x] Add Pressure Sensor
- [x] Add IMU sensor
- [ ] Add basic interface in LVGL with styles
