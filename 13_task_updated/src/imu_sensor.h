/**
 * @file imu_sensor.h
 * @brief Interface for IMU (Inertial Measurement Unit) sensor handling.
 *
 * This header provides the function declarations for initializing
 * and processing samples from the IMU sensor. It ensures modular
 * and consistent integration of IMU sensor data collection in
 * embedded applications.
 *
 * @date 15-08-2025
 * @author Parthrajsinh Vala
*/

#ifndef IMU_SENSOR_H
#define IMU_SENSOR_H

/**
 * @brief Process a new IMU sensor sample.
 *
 * This function should be called whenever a new IMU sensor
 * measurement is available. It handles acquisition, filtering,
 * and preparing the data for further use or storage.
*/

void imu_sensor_sample_process(void);

/**
 * @brief Initialize the IMU sensor.
 *
 * Sets up the IMU sensor hardware, configures registers,
 * and prepares it for sampling operations.
 *
 * @return int 0 on success, negative error code on failure.
*/

int imu_sensor_init(void);

#endif /* IMU_SENSOR_H */
