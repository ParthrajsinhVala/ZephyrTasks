/**
 * @file main.c
 * @brief Main module for logger initialization.
 *
 * This module initalizes the mounting and logging
 * the sensor data from respective sensors to the 
 * files in Little FS.
 *
 * @date 15-08-2025
 * @author Parthrajsinh Vala
*/

//==============================================================================
// Includes
//==============================================================================

#include "hum_temp_sensor.h"
#include "imu_sensor.h"
#include "pressure_sensor.h"
#include "sensor_shared.h"
#include "sensor_storage.h"
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/logging/log.h>
#include <stdbool.h>

//==============================================================================
// Configuration Constants
//==============================================================================

/**
 * @brief Generic thread configuration.
 *
 * Defines the default stack size, priority, and sleep interval
 * for standard threads handling sensor operations.
*/

#define STACK_SIZE         1024         // Default stack size for generic threads
#define PRIORITY           5            // Default thread priority
#define SLEEP_TIME         K_SECONDS(5) // Delay between sensor operations

/**
 * @brief Storage thread configuration.
 *
 * Defines the stack size, priority, and execution interval
 * for the thread responsible for writing sensor data to storage.
*/

#define STORAGE_STACK_SIZE (1024 * 4)   // Stack size for storage thread
#define STORAGE_PRIORITY   4            // Thread priority for storage task
#define STORAGE_INTERVAL   K_MINUTES(1) // Interval for storing sensor data

/**
 * @brief Reader thread configuration.
 *
 * Defines the stack size, priority, and execution interval
 * for the thread responsible for reading stored sensor data.
*/

#define READER_STACK_SIZE  1024         // Stack size for reader thread
#define READER_PRIORITY    5            // Thread priority for reader task
#define READER_INTERVAL    K_MINUTES(2) // Interval for reading stored data

/**
 * @brief Thread stack memory definitions.
 *
 * Defines dedicated stack areas for all sensor, storage, and reader threads.
 * Stack sizes are determined by the corresponding configuration macros.
*/

K_THREAD_STACK_DEFINE(hum_temp_stack, STACK_SIZE);       // Humidity & temperature sensor thread stack
K_THREAD_STACK_DEFINE(pressure_stack, STACK_SIZE);       // Pressure sensor thread stack
K_THREAD_STACK_DEFINE(imu_stack, STACK_SIZE);            // IMU (accelerometer/gyroscope) sensor thread stack
K_THREAD_STACK_DEFINE(storage_stack, STORAGE_STACK_SIZE); // Storage thread stack
K_THREAD_STACK_DEFINE(reader_stack, READER_STACK_SIZE);   // Reader thread stack


/**
 * @brief Thread control structures, IDs, and termination flags.
 *
 * Each sensor and system task has its own thread data, thread ID,
 * and a termination flag used to signal graceful shutdown.
*/

static struct k_thread hum_temp_thread_data;      // Humidity & temperature sensor thread control block
static struct k_thread pressure_thread_data;      // Pressure sensor thread control block
static struct k_thread imu_thread_data;           // IMU (accelerometer/gyroscope) sensor thread control block
static struct k_thread storage_thread_data;       // Storage thread control block
static struct k_thread reader_thread_data;        // Reader thread control block

static k_tid_t hum_temp_tid, pressure_tid, imu_tid, storage_tid, reader_tid; // Thread IDs for all tasks

static bool terminate_hum_temp_thread = false;    // Termination flag for humidity & temperature thread
static bool terminate_pressure_thread = false;    // Termination flag for pressure thread
static bool terminate_imu_thread = false;         // Termination flag for IMU thread
static bool terminate_storage_thread = false;     // Termination flag for storage thread
static bool terminate_reader_thread = false;      // Termination flag for reader thread

//==============================================================================
// Logging Module Register
//==============================================================================

LOG_MODULE_REGISTER(main);

//==============================================================================
// Function Definitions
//==============================================================================

/**
 * @brief Humidity & Temperature sensor thread.
 *
 * This thread periodically fetches humidity and temperature samples
 * from the sensor by calling hum_temp_sensor_process_sample().
 * The thread runs until its termination flag is set, sleeping for
 * @ref SLEEP_TIME between each sample.
 *
 * @param a Unused thread parameter.
 * @param b Unused thread parameter.
 * @param c Unused thread parameter.
 *
 * @return void
*/

static void hun_temp_sensor_thread(void *a, void *b, void *c)
{
    LOG_INF("Humidity-Temperature sensor thread started.");

    while (!terminate_hum_temp_thread) {
        hum_temp_sensor_process_sample();
        k_sleep(SLEEP_TIME);
    }

    LOG_INF("Humidity-Temperature sensor thread stopped.");
}

/**
 * @brief Thread handler for the pressure sensor.
 *
 * This thread continuously reads pressure data from the sensor
 * by invoking pressure_sensor_process_sample(). It runs in a loop
 * until the termination flag @ref terminate_pressure_thread is set.
 *
 * The thread sleeps for @ref SLEEP_TIME between sensor reads
 * to control sampling frequency.
 *
 * @param a Unused thread parameter (reserved for Zephyr API).
 * @param b Unused thread parameter (reserved for Zephyr API).
 * @param c Unused thread parameter (reserved for Zephyr API).
 *
 * @return void
*/

static void pressure_sensor_thread(void *a, void *b, void *c)
{
    LOG_INF("Pressure sensor thread started.");

    while (!terminate_pressure_thread) {
        pressure_sensor_process_sample();
        k_sleep(SLEEP_TIME);
    }

    LOG_INF("Pressure sensor thread stopped.");
}

/**
 * @brief Thread function for IMU sensor sampling and processing.
 *
 * This thread continuously processes IMU sensor samples at a fixed interval
 * until the termination flag is set. Once the termination condition is met,
 * the thread exits gracefully.
 *
 * @param a Unused thread parameter.
 * @param b Unused thread parameter.
 * @param c Unused thread parameter.
 *
 * @note Uses the global variable @c terminate_imu_thread as the stop condition.
 * @note Calls @c imu_sensor_sample_process() for each iteration.
*/

static void imu_sensor_thread(void *a, void *b, void *c)
{
    LOG_INF("IMU sensor thread started.");

    while (!terminate_imu_thread) {
        imu_sensor_sample_process();
        k_sleep(SLEEP_TIME);
    }

    LOG_INF("IMU sensor thread stopped.");
}

/**
 * @brief Thread handler for storing sensor data.
 *
 * This thread periodically saves the latest sensor readings
 * to persistent storage using littlefs_save_sensor_data().
 * Access to the shared @ref sensor_data structure is protected
 * with a mutex during storage operations.
 *
 * The thread runs until the termination flag
 * @ref terminate_storage_thread is set, sleeping for
 * @ref STORAGE_INTERVAL between write operations.
 *
 * @param a Unused thread parameter (reserved for Zephyr API).
 * @param b Unused thread parameter (reserved for Zephyr API).
 * @param c Unused thread parameter (reserved for Zephyr API).
 *
 * @return void
*/

static void sensor_storage_thread(void *a, void *b, void *c)
{
    LOG_INF("Sensor storage thread started.");

    while (!terminate_storage_thread) {
        k_mutex_lock(&sensor_data_mutex, K_FOREVER);
        littlefs_save_sensor_data(&sensor_data);
        k_mutex_unlock(&sensor_data_mutex);
        k_sleep(STORAGE_INTERVAL);
    }

    LOG_INF("Sensor storage thread stopped.");
}

/**
 * @brief Thread handler for reading stored sensor data.
 *
 * This thread periodically retrieves sensor data from
 * persistent storage using littlefs_read_sensor_data().
 *
 * The thread runs until the termination flag
 * @ref terminate_reader_thread is set, sleeping for
 * @ref READER_INTERVAL between read operations.
 *
 * @param a Unused thread parameter (reserved for Zephyr API).
 * @param b Unused thread parameter (reserved for Zephyr API).
 * @param c Unused thread parameter (reserved for Zephyr API).
 *
 * @return void
*/

static void reader_thread(void *a, void *b, void *c)
{
    LOG_INF("Reader thread started.");

    while (!terminate_reader_thread) {
        littlefs_read_sensor_data();
        k_sleep(READER_INTERVAL);
    }

    LOG_INF("Reader thread stopped.");
}

/**
 * @brief Main entry point of the application.
 *
 * This function initializes the global sensor data mutex,
 * sets up all sensor devices, and creates the required
 * threads for sensor data acquisition, storage, and reading.
 *
 * The following threads are started:
 * - Humidity & Temperature sensor thread
 * - Pressure sensor thread
 * - IMU sensor thread
 * - Sensor storage thread
 * - Reader thread
 *
 * Each thread runs independently until its corresponding
 * termination flag is set.
 *
 * @return 0 on success, negative error code otherwise.
*/

int main(void)
{
    int ret;

    k_mutex_init(&sensor_data_mutex);

    /* Initialize sensors */
    ret = hun_temp_sensor_init();
    if (ret < 0) {
        LOG_ERR("Humidity-Temperature Sensor init failed");
    }

    ret = pressure_sensor_init();
    if (ret < 0) {
        LOG_ERR("Pressure Sensor init failed");
    }

    ret = imu_sensor_init();
    if (ret < 0) {
        LOG_ERR("IMU Sensor init failed");
    }

    /* Start sensor threads */
    terminate_hum_temp_thread = false;
    hum_temp_tid = k_thread_create(&hum_temp_thread_data, hum_temp_stack, K_THREAD_STACK_SIZEOF(hum_temp_stack),
                                   hun_temp_sensor_thread, NULL, NULL, NULL, PRIORITY, 0, K_NO_WAIT);

    terminate_pressure_thread = false;
    pressure_tid = k_thread_create(&pressure_thread_data, pressure_stack, K_THREAD_STACK_SIZEOF(pressure_stack),
                                   pressure_sensor_thread, NULL, NULL, NULL, PRIORITY, 0, K_NO_WAIT);

    terminate_imu_thread = false;
    imu_tid = k_thread_create(&imu_thread_data, imu_stack, K_THREAD_STACK_SIZEOF(imu_stack),
                              imu_sensor_thread, NULL, NULL, NULL, PRIORITY, 0, K_NO_WAIT);

    /* Start storage thread */
    terminate_storage_thread = false;
    storage_tid = k_thread_create(&storage_thread_data, storage_stack, K_THREAD_STACK_SIZEOF(storage_stack),
                                  sensor_storage_thread, NULL, NULL, NULL, STORAGE_PRIORITY, 0, K_NO_WAIT);
                
    /* Start reader thread */
    terminate_reader_thread = false;
    reader_tid = k_thread_create(&reader_thread_data, reader_stack, K_THREAD_STACK_SIZEOF(reader_stack),
                             reader_thread, NULL, NULL, NULL, READER_PRIORITY, 0, K_NO_WAIT);

    return 0;
}