/**
 * @file pressure_sensor.h
 * @brief Interface for the pressure sensor driver.
 *
 * This header provides function declarations for initializing
 * the pressure sensor and processing sensor samples.
 */

#ifndef PRESSURE_SENSOR_H
#define PRESSURE_SENSOR_H

/**
 * @brief Process a pressure sensor sample.
 *
 * Reads raw data from the pressure sensor, applies any necessary
 * conversions or filtering, and makes the processed data available
 * for logging or further use.
 *
 * @date 15-08-2025
 * @author Parthrajsinh Vala
*/

void pressure_sensor_process_sample(void);

/**
 * @brief Initialize the pressure sensor.
 *
 * Configures the pressure sensor hardware and prepares it for use.
 *
 * @return int 0 if initialization is successful, negative error code otherwise.
*/

int pressure_sensor_init(void);

#endif /* PRESSURE_SENSOR_H */
