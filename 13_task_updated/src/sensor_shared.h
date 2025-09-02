/**
 * @file sensor_shared.h
 * @brief Shared data structures and synchronization primitives for sensor modules.
 *
 * This header defines the common sensor data structure used across
 * temperature, humidity, pressure, and IMU sensor modules. It also
 * declares a global mutex to ensure thread-safe access to sensor data.
 *
 * @date 15-08-2025
 * @author Parthrajsinh Vala
*/

#ifndef SENSOR_SHARED_H
#define SENSOR_SHARED_H

#include <zephyr/kernel.h>

/**
 * @struct sensor_data_t
 * @brief Structure to hold all sensor readings.
 *
 * This structure contains measurements from multiple sensors:
 * - Temperature and humidity
 * - Barometric pressure
 * - Accelerometer (X, Y, Z)
 * - Gyroscope (X, Y, Z)
*/

struct sensor_data_t {
    float temperature; /* Temperature reading in degrees Celsius. */
    float humidity;    /* Humidity reading in percentage (%RH). */
    float pressure;    /* Pressure reading in hPa (hectopascal). */
    float accel_x;     /* Acceleration in X-axis. */
    float accel_y;     /* Acceleration in Y-axis. */
    float accel_z;     /* Acceleration in Z-axis. */
    float gyro_x;      /* Gyroscope reading in X-axis. */
    float gyro_y;      /* Gyroscope reading in Y-axis. */
    float gyro_z;      /* Gyroscope reading in Z-axis. */
};

/**
 * @brief Global sensor data instance.
 *
 * This variable holds the latest sensor readings.
 * Access must be synchronized using @ref sensor_data_mutex.
*/

extern struct sensor_data_t sensor_data;

/**
 * @brief Mutex for protecting access to @ref sensor_data.
 *
 * Ensures thread-safe updates and reads of sensor data.
*/

extern struct k_mutex sensor_data_mutex;

#endif /* SENSOR_SHARED_H */
