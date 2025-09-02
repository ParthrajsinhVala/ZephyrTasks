/**
 * @file imu_sensor.c
 * @brief IMU (Accelerometer + Gyroscope) sensor processing module.
 *
 * This module provides functionality to fetch and process data
 * from the onboard IMU (e.g., LSM6DSL) using Zephyr's sensor API.
 *
 * It extracts both acceleration (X, Y, Z) and gyroscope (X, Y, Z) readings
 * and stores them in a user-provided @ref three_d struct.
 *
 * @date 15-08-2025
 * @author Parthrajsinh Vala
*/

//==============================================================================
// Includes
//==============================================================================

#include "imu_sensor.h"
#include "sensor_shared.h"
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

//==============================================================================
// Logging Module Register
//==============================================================================

LOG_MODULE_REGISTER(imu);

//==============================================================================
// Device Tree Bindings
//==============================================================================

#if DT_NODE_EXISTS(DT_ALIAS(imu_sensor))
#define IMU_NODE DT_ALIAS(imu_sensor)
const struct device *const imu_dev = DEVICE_DT_GET(DT_ALIAS(imu_sensor));
#else
#error ("IMU sensor not found.");
#endif

//==============================================================================
// Function Definitions
//==============================================================================

/**
 * @brief Process accelerometer and gyroscope samples from the IMU sensor.
 *
 * This function fetches the latest IMU sample data and reads values from both
 * the accelerometer and gyroscope channels. The raw sensor values are converted
 * to floating-point and stored in the shared sensor_data structure, protected
 * by a mutex to ensure thread safety.
 *
 * @note Error conditions are logged but not returned to the caller.
 *
 * @return void
*/

void imu_sensor_sample_process(void)
{
    if (sensor_sample_fetch(imu_dev) < 0) {
        LOG_ERR("Sensor sample update error");
        return;
    }
    char out_str[64];
    struct sensor_value accel_x, accel_y, accel_z;
    struct sensor_value gyro_x, gyro_y, gyro_z;

    /* lsm6dsl accel */
    sensor_sample_fetch_chan(imu_dev, SENSOR_CHAN_ACCEL_XYZ);
    sensor_channel_get(imu_dev, SENSOR_CHAN_ACCEL_X, &accel_x);
    sensor_channel_get(imu_dev, SENSOR_CHAN_ACCEL_Y, &accel_y);
    sensor_channel_get(imu_dev, SENSOR_CHAN_ACCEL_Z, &accel_z);

    sprintf(out_str, "accel x:%f ms/2 y:%f ms/2 z:%f ms/2", sensor_value_to_double(&accel_x),
            sensor_value_to_double(&accel_y), sensor_value_to_double(&accel_z));
    //LOG_INF("%s", out_str);

    k_mutex_lock(&sensor_data_mutex, K_FOREVER);
    sensor_data.accel_x = sensor_value_to_double(&accel_x);
    sensor_data.accel_y = sensor_value_to_double(&accel_y);
    sensor_data.accel_z = sensor_value_to_double(&accel_z);
    k_mutex_unlock(&sensor_data_mutex);

    /* lsm6dsl gyro */
    sensor_sample_fetch_chan(imu_dev, SENSOR_CHAN_GYRO_XYZ);
    sensor_channel_get(imu_dev, SENSOR_CHAN_GYRO_X, &gyro_x);
    sensor_channel_get(imu_dev, SENSOR_CHAN_GYRO_Y, &gyro_y);
    sensor_channel_get(imu_dev, SENSOR_CHAN_GYRO_Z, &gyro_z);

    sprintf(out_str, "gyro x:%f dps y:%f dps z:%f dps", sensor_value_to_double(&gyro_x),
            sensor_value_to_double(&gyro_y), sensor_value_to_double(&gyro_z));
    //LOG_INF("%s", out_str);

    k_mutex_lock(&sensor_data_mutex, K_FOREVER);
    sensor_data.gyro_x = sensor_value_to_double(&gyro_x);
    sensor_data.gyro_y = sensor_value_to_double(&gyro_y);
    sensor_data.gyro_z = sensor_value_to_double(&gyro_z);
    k_mutex_unlock(&sensor_data_mutex);
}

/**
 * @brief Initialize the IMU (accelerometer and gyroscope) sensor.
 *
 * This function checks whether the IMU device is ready and configures
 * the sampling frequency for both the accelerometer and gyroscope
 * channels to 104 Hz. After successful configuration, it triggers
 * an initial sample fetch using imu_sensor_sample_process() to
 * populate sensor_data.
 *
 * @return  0  Success, sensor ready and configured.  
 *         -1  Failure, device not ready or attribute configuration failed.
*/

int imu_sensor_init(void)
{

    if (!device_is_ready(imu_dev)) {
        LOG_ERR("sensor: device not ready.");
        return -1;
    }

    struct sensor_value odr_attr;

    /* set accel/gyro sampling frequency to 104 Hz */
    odr_attr.val1 = 104;
    odr_attr.val2 = 0;

    if (sensor_attr_set(imu_dev, SENSOR_CHAN_ACCEL_XYZ, SENSOR_ATTR_SAMPLING_FREQUENCY, &odr_attr) < 0) {
        LOG_ERR("Cannot set sampling frequency for accelerometer.");
        return -1;
    }

    if (sensor_attr_set(imu_dev, SENSOR_CHAN_GYRO_XYZ, SENSOR_ATTR_SAMPLING_FREQUENCY, &odr_attr) < 0) {
        LOG_ERR("Cannot set sampling frequency for gyro.");
        return -1;
    }

    imu_sensor_sample_process();
    return 0;
}