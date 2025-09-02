/**
 * @file sensor_storage.c
 * @brief Logs the data to file in littlefs partition.
 *
 * This file provides the functionality of logging the data 
 * in the shared buffer of sensors to a file created in a little-fs 
 * partition.
 *
 * @date 15-08-2025
 * @author Parthrajsinh Vala
*/

//==============================================================================
// Includes
//==============================================================================

#include "sensor_shared.h"
#include <zephyr/fs/fs.h>
#include <zephyr/logging/log.h>
#include <zephyr/storage/flash_map.h>
#include <stdio.h>

//==============================================================================
// Logging Module Register
//==============================================================================

LOG_MODULE_REGISTER(sensor_storage);

//==============================================================================
// Configuration Constants
//==============================================================================

/** Path to the file where sensor data will be stored */
#define SENSOR_DATA_FILE "/lfs/SensorData"   // Sensor data file path in LittleFS

/** Maximum size of the line buffer for formatted sensor data */
#define LINE_BUFFER_SIZE 256                 // Buffer size for writing one line

//==============================================================================
// Function Definitions
//==============================================================================

/**
 * @brief Save sensor data to LittleFS storage.
 *
 * This function formats the given sensor data into a human-readable string
 * with uptime information and appends it to the file defined by 
 * ::SENSOR_DATA_FILE in the LittleFS filesystem.
 *
 * The data includes temperature, humidity, pressure, accelerometer 
 * (X, Y, Z), and gyroscope (X, Y, Z) readings. A timestamp in seconds 
 * since system start is prepended to each entry.
 *
 * @param[in] data Pointer to a ::sensor_data_t structure containing the 
 *                 latest sensor readings.
 *
 * @retval  0        Success, data was written to the file.
 * @retval -EINVAL   Failure, buffer formatting error (data too large).
 * @retval <0        Failure, filesystem-related error (see return code).
*/

int littlefs_save_sensor_data(const struct sensor_data_t *data)
{
    struct fs_file_t file;
    int rc;
    char line_buffer[LINE_BUFFER_SIZE];
    
    /* Get uptime in seconds */
    uint64_t uptime_sec = k_uptime_get() / 1000;


    int len = snprintf(line_buffer, LINE_BUFFER_SIZE,
                       "[%llu s] Temperature: %.2f, Humidity: %.2f, Pressure: %.2f, "
                       "Accel X: %.2f, Accel Y: %.2f, Accel Z: %.2f, "
                       "Gyro X: %.2f, Gyro Y: %.2f, Gyro Z: %.2f\n",uptime_sec,
                       (double)data->temperature, (double)data->humidity, (double)data->pressure,
                       (double)data->accel_x, (double)data->accel_y, (double)data->accel_z,
                       (double)data->gyro_x, (double)data->gyro_y, (double)data->gyro_z);

    if (len < 0 || len >= LINE_BUFFER_SIZE) {
        LOG_ERR("Failed to format sensor data line");
        return -EINVAL;
    }

    fs_file_t_init(&file);
    LOG_INF("Saving sensor data to LittleFS");

    rc = fs_open(&file, SENSOR_DATA_FILE, FS_O_CREATE | FS_O_WRITE | FS_O_APPEND);
    if (rc < 0) {
        LOG_ERR("fs_open() Failed (error: %d)", rc);
        return rc;
    }

    rc = fs_write(&file, line_buffer, len);
    if (rc < 0) {
        LOG_ERR("fs_write() Failed (error: %d)", rc);
        fs_close(&file);
        return rc;
    }

    fs_close(&file);
    LOG_INF("Sensor data saved successfully");

    
    return 0;
}

/**
 * @brief Reads and prints sensor data from LittleFS.
 *
 * This function opens the sensor data file stored in LittleFS,
 * reads its contents in chunks, and prints them to the console.
 *
 * @return int
 * - 0 on success
 * - Negative error code if file open or read fails
*/

int littlefs_read_sensor_data(void)
{
    struct fs_file_t file;
    char buf[128];
    int rc;

    fs_file_t_init(&file);
    rc = fs_open(&file, SENSOR_DATA_FILE, FS_O_READ);
    if (rc < 0) {
        LOG_ERR("Failed to open file for reading (err %d)", rc);
        return rc;
    }

    LOG_INF("Reading data from %s", SENSOR_DATA_FILE);

    while ((rc = fs_read(&file, buf, sizeof(buf) - 1)) > 0) {
        buf[rc] = '\0';       /* Null terminate so it prints cleanly */
        printk("%s", buf);    /* Print directly to console */
    }

    fs_close(&file);
    return 0;
}