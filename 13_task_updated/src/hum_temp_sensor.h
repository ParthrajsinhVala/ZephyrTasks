/**
 * @file hum_temp_sensor.h
 * @brief Humidity and Temperature Sensor Interface.
 *
 * This header provides function declarations for initializing and 
 * processing samples from a humidity and temperature sensor.
 *
 * @date 15-08-2025
 * @author Parthrajsinh Vala
*/

#ifndef HUM_TEMP_SENSOR_H
#define HUM_TEMP_SENSOR_H

/**
 * @brief Process a single sample from the humidity and temperature sensor.
 *
 * This function should be called whenever a new sensor reading is available.
 * It handles reading, processing, and possibly logging or storing the values.
*/

void hum_temp_sensor_process_sample(void);

/**
 * @brief Initialize the humidity and temperature sensor.
 *
 * Sets up the sensor device and prepares it for sampling.
 *
 * @return 0 on success, or a negative error code on failure.
*/

int hum_temp_sensor_init(void);

#endif /* HUM_TEMP_SENSOR_H */
