/**
 * @file sensor_share.c
 * @brief Shared sensor data handling module.
 *
 * This module defines the global sensor data structure and the
 * synchronization mechanism used to protect concurrent access.
 *
 * It provides a shared instance of `sensor_data_t` along with a
 * Zephyr mutex to ensure thread-safe operations when reading or
 * updating sensor data across multiple threads.
 *
 * @date 15-08-2025
 * @author Parthrajsinh Vala
*/

//==============================================================================
// Includes
//==============================================================================

#include "sensor_shared.h"

//==============================================================================
// Global Variables
//==============================================================================

/** 
 * @var sensor_data
 * @brief Global structure holding the latest sensor readings.
 *
 * This instance of ::sensor_data_t stores the most recent values 
 * fetched from all onboard sensors (temperature, humidity, pressure, 
 * accelerometer, gyroscope, etc.).  
 * 
 * It is shared across application modules and protected by 
 * ::sensor_data_mutex to ensure thread-safe access.
 */
struct sensor_data_t sensor_data;

/**
 * @var sensor_data_mutex
 * @brief Mutex for synchronizing access to ::sensor_data.
 *
 * This Zephyr kernel mutex ensures that only one thread at a time 
 * can read or update ::sensor_data, preventing race conditions and 
 * data corruption in multi-threaded environments.
 */
struct k_mutex sensor_data_mutex;

