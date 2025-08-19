#include "sensor_shared.h"
#include <zephyr/fs/fs.h>
#include <zephyr/logging/log.h>
#include <zephyr/storage/flash_map.h>
#include <stdio.h>

LOG_MODULE_REGISTER(sensor_storage);

#define SENSOR_DATA_FILE "/lfs1/sensor_data.txt"
#define LINE_BUFFER_SIZE 256

int littlefs_save_sensor_data(const struct sensor_data_t *data)
{
    struct fs_file_t file;
    int rc;
    char line_buffer[LINE_BUFFER_SIZE];

    int len = snprintf(line_buffer, LINE_BUFFER_SIZE,
                       "Temperature: %.2f, Humidity: %.2f, Pressure: %.2f, "
                       "Accel X: %.2f, Accel Y: %.2f, Accel Z: %.2f, "
                       "Gyro X: %.2f, Gyro Y: %.2f, Gyro Z: %.2f\n",
                       (double)data->temperature, (double)data->humidity, (double)data->pressure, (double)data->accel_x,
                       (double)data->accel_y, (double)data->accel_z, (double)data->gyro_x, (double)data->gyro_y,
                       (double)data->gyro_z);

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

    LOG_INF("File opened successfully, writing data");

    rc = fs_write(&file, line_buffer, len);
    if (rc < 0) {
        LOG_ERR("fs_write() Failed (error: %d)", rc);
        fs_close(&file);
        return rc;
    }

    rc = fs_close(&file);
    if (rc < 0) {
        LOG_ERR("Failed to close file (error: %d)", rc);
        return rc;
    }

    LOG_INF("Sensor data saved successfully");
    return 0;
}
