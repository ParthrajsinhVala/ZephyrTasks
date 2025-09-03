/**
 * @file hum_temp_sensor.c
 * @brief Humidity and Temperature Sensor Processing Module.
 *
 * This module provides functionality to fetch and process data
 * from the onboard HTS221 (or equivalent) humidity/temperature sensor
 * using Zephyr's sensor API.
 *
 * @date 15-08-2025
 * @author Parthrajsinh Vala
*/

//==============================================================================
// Includes
//==============================================================================

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include "hum_temp_sensor.h"
#include "sensor_shared.h"

//==============================================================================
// Logging Module Register
//==============================================================================

LOG_MODULE_REGISTER(hum_temp);

//==============================================================================
// Device Tree Bindings
//==============================================================================

#if DT_NODE_EXISTS(DT_ALIAS(ht_sensor))
#define HUM_TEMP_NODE DT_ALIAS(ht_sensor)
const struct device *const hts_dev = DEVICE_DT_GET(DT_ALIAS(ht_sensor));
#else
#error ("Humidity-Temperature sensor not found.");
#endif

//==============================================================================
// Function Definitions
//==============================================================================

/**
 * @brief Process a humidity and temperature sample from the HTS221 sensor.
 *
 * This function verifies that the sensor device is ready, fetches the
 * latest measurement, and retrieves both temperature and humidity values.
 * The values are converted to floating-point and stored in the shared
 * sensor_data structure (protected by a mutex).
 *
 * @note Error conditions are logged but not propagated to the caller.
 *
 * @return void
*/


void hum_temp_sensor_process_sample(void)
{
    if (!device_is_ready(hts_dev)) {
        LOG_ERR("sensor: %s device not ready.", hts_dev->name);
        return;
    }

    if (sensor_sample_fetch(hts_dev) < 0) {
        LOG_ERR("Sensor sample update error");
        return;
    }

    struct sensor_value temp, hum;
    if (sensor_channel_get(hts_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp) < 0) {
        LOG_ERR("Cannot read HTS221 temperature channel");
        return;
    }

    if (sensor_channel_get(hts_dev, SENSOR_CHAN_HUMIDITY, &hum) < 0) {
        LOG_ERR("Cannot read HTS221 humidity channel");
        return;
    }

    k_mutex_lock(&sensor_data_mutex, K_FOREVER);
    sensor_data.temperature = sensor_value_to_double(&temp);
    sensor_data.humidity = sensor_value_to_double(&hum);
    k_mutex_unlock(&sensor_data_mutex);

    /* display temperature */
    //LOG_INF("Temperature:%.1f C", sensor_value_to_double(&temp));

    /* display humidity */
    //LOG_INF("Relative Humidity:%.1f%%", sensor_value_to_double(&hum));
}

/**
 * @brief Initialize the HTS221 humidity and temperature sensor.
 *
 * This function checks whether the sensor device is ready for use.
 * If the device is available, it triggers an initial sample fetch
 * using hum_temp_sensor_process_sample() to populate sensor_data.
 *
 * @return  0  Success, sensor ready and initial sample processed.  
 *         -1  Failure, sensor device not ready.
*/


int hum_temp_sensor_init(void)
{
    if (!device_is_ready(hts_dev)) {
        LOG_ERR("sensor: %s device not ready.", hts_dev->name);
        return -1;
    }

    hum_temp_sensor_process_sample();

    return 0;
}
