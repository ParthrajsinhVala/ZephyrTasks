/**
 * @file sensor_storage.h
 * @brief Interface for saving and reading sensor data using LittleFS.
 *
 * This header provides functions to store and retrieve sensor data
 * in a LittleFS-backed file system for persistent storage.
 * 
 * @date 15-08-2025
 * @author Parthrajsinh Vala
*/

#ifndef SENSOR_STORAGE_H
#define SENSOR_STORAGE_H

/**
 * @brief Save sensor data to LittleFS.
 *
 * This function stores the given sensor data structure into persistent storage.
 * Each call appends or writes sensor readings for later retrieval.
 *
 * @param[in] data Pointer to the sensor_data_t structure containing sensor readings.
 * @retval 0 on success.
 * @retval -errno on failure (standard error codes).
*/

int littlefs_save_sensor_data(const struct sensor_data_t *data);

/**
 * @brief Read sensor data from LittleFS.
 *
 * This function reads previously saved sensor data from persistent storage.
 * The implementation may log or print the results depending on configuration.
 *
 * @retval 0 on success.
 * @retval -errno on failure (standard error codes).
*/

int littlefs_read_sensor_data();

#endif /* SENSOR_STORAGE_H */