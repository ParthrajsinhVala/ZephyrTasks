/**
 * @file pressure_sensor.c
 * @brief Pressure sensor processing module.
 *
 * This module provides functionality to fetch and process data
 * from the onboard pressure sensor using Zephyr's sensor API.
 *
 * It checks device readiness, fetches sensor samples, and converts
 * the pressure value into a double (kPa). The processed data can then
 * be used by application threads or logging subsystems.
 *
 * @date 15-08-2025
 * @author Parthrajsinh Vala
*/

//==============================================================================
// Includes
//==============================================================================

#include "pressure_sensor.h"
#include "sensor_shared.h"
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>

//==============================================================================
// Logging Module Register
//==============================================================================

LOG_MODULE_REGISTER(pressure);


//==============================================================================
// Device Tree Bindings
//==============================================================================

#if DT_NODE_EXISTS(DT_ALIAS(pressure_sensor))
#define PRESSURE_NODE DT_ALIAS(pressure_sensor)
const struct device *const pressure_dev = DEVICE_DT_GET(DT_ALIAS(pressure_sensor));
#else
#error ("Pressure sensor not found.");
#endif

//==============================================================================
// Function Definitions
//==============================================================================

/**
 * @brief Process a pressure sample from the sensor.
 *
 * This function verifies that the pressure sensor device is ready,
 * fetches the latest measurement, and retrieves the pressure channel value.
 * The raw sensor value is converted to floating-point and stored in the
 * shared sensor_data structure, protected by a mutex for thread safety.
 *
 * @note Error conditions are logged but not propagated to the caller.
 *
 * @return void
*/

void pressure_sensor_process_sample(void)
{
    if (!device_is_ready(pressure_dev)) {
        LOG_ERR("sensor: %s device not ready.", pressure_dev->name);
        return;
    }

    if (sensor_sample_fetch(pressure_dev) < 0) {
        LOG_INF("Sensor sample update error");
        return;
    }

    struct sensor_value pressure;
    if (sensor_channel_get(pressure_dev, SENSOR_CHAN_PRESS, &pressure) < 0) {
        LOG_ERR("Cannot read pressure channel");
        return;
    }

    k_mutex_lock(&sensor_data_mutex, K_FOREVER);
    sensor_data.pressure = sensor_value_to_double(&pressure);
    k_mutex_unlock(&sensor_data_mutex);

    /* display pressure */
    //LOG_INF("Pressure:%.1f kPa", sensor_value_to_double(&pressure));
}

/**
 * @brief Initialize the pressure sensor.
 *
 * This function checks whether the pressure sensor device is ready.
 * If the device is available, it triggers an initial sample fetch
 * using pressure_sensor_process_sample() to populate sensor_data.
 *
 * @return  0  Success, sensor ready and initial sample processed.  
 *         -1  Failure, sensor device not ready.
*/

int pressure_sensor_init(void)
{
    if (!device_is_ready(pressure_dev)) {
        LOG_ERR("sensor: %s device not ready.", pressure_dev->name);
        return -1;
    }

    pressure_sensor_process_sample();

    return 0;
}
