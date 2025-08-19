#include "hum_temp_sensor.h"
#include "imu_sensor.h"
#include "pressure_sensor.h"
#include "sensor_shared.h"
#include "sensor_storage.h"

#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>

#include <zephyr/logging/log.h>
#include <zephyr/shell/shell.h>

#include <stdbool.h>

#define STACK_SIZE 1024
#define PRIORITY   5
#define SLEEP_TIME K_SECONDS(5)

#define STORAGE_STACK_SIZE 1024 * 4
#define STORAGE_PRIORITY   4
#define STORAGE_INTERVAL   K_MINUTES(1)

K_THREAD_STACK_DEFINE(hum_temp_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(pressure_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(imu_stack, STACK_SIZE);
K_THREAD_STACK_DEFINE(storage_stack, STORAGE_STACK_SIZE);

static struct k_thread hum_temp_thread_data;
static struct k_thread pressure_thread_data;
static struct k_thread imu_thread_data;
static struct k_thread storage_thread_data;

static k_tid_t hum_temp_tid, pressure_tid, imu_tid, storage_tid;

static bool terminate_hum_temp_thread = false;
static bool terminate_pressure_thread = false;
static bool terminate_imu_thread = false;
static bool terminate_storage_thread = false;

LOG_MODULE_REGISTER(main);

int main(void)
{
    int ret;

    k_mutex_init(&sensor_data_mutex);

    ret = hun_temp_sensor_init();
    if (ret < 0) {
        LOG_ERR("Humidity-Temperature Sensor init failed");
        return ret;
    }

    ret = pressure_sensor_init();
    if (ret < 0) {
        LOG_ERR("Pressure Sensor init failed");
        return ret;
    }

    ret = imu_sensor_init();
    if (ret < 0) {
        LOG_ERR("IMU Sensor init failed");
        return ret;
    }

    return 0;
}

static void hun_temp_sensor_thread(void *a, void *b, void *c)
{
    LOG_INF("Humidity-Temperature sensor thread started.");

    while (!terminate_hum_temp_thread) {
        hum_temp_sensor_process_sample();
        k_sleep(SLEEP_TIME);
    }

    LOG_INF("Humidity-Temperature sensor thread stopped.");
}

static void pressure_sensor_thread(void *a, void *b, void *c)
{
    LOG_INF("Pressure sensor thread started.");

    while (!terminate_pressure_thread) {
        pressure_sensor_process_sample();
        k_sleep(SLEEP_TIME);
    }

    LOG_INF("Pressure sensor thread stopped.");
}

static void imu_sensor_thread(void *a, void *b, void *c)
{
    LOG_INF("IMU sensor thread started.");

    while (!terminate_imu_thread) {
        imu_sensor_sample_process();
        k_sleep(SLEEP_TIME);
    }

    LOG_INF("IMU sensor thread stopped.");
}

static int shell_start_hum_temp_thread(const struct shell *sh, size_t argc, char **argv, void *data)
{
    terminate_hum_temp_thread = false;
    hum_temp_tid = k_thread_create(&hum_temp_thread_data, hum_temp_stack, K_THREAD_STACK_SIZEOF(hum_temp_stack),
                                   hun_temp_sensor_thread, NULL, NULL, NULL, PRIORITY, 0, K_NO_WAIT);
    return 0;
}

static int shell_start_pressure_thread(const struct shell *sh, size_t argc, char **argv, void *data)
{
    terminate_pressure_thread = false;
    pressure_tid = k_thread_create(&pressure_thread_data, pressure_stack, K_THREAD_STACK_SIZEOF(pressure_stack),
                                   pressure_sensor_thread, NULL, NULL, NULL, PRIORITY, 0, K_NO_WAIT);
    return 0;
}

static int shell_start_imu_thread(const struct shell *sh, size_t argc, char **argv, void *data)
{
    terminate_imu_thread = false;
    imu_tid = k_thread_create(&imu_thread_data, imu_stack, K_THREAD_STACK_SIZEOF(imu_stack), (void *)imu_sensor_thread,
                              NULL, NULL, NULL, PRIORITY, 0, K_NO_WAIT);
    return 0;
}

static int shell_start_all_sensors(const struct shell *sh, size_t argc, char **argv, void *data)
{
    terminate_hum_temp_thread = false;
    terminate_pressure_thread = false;
    terminate_imu_thread = false;

    hum_temp_tid = k_thread_create(&hum_temp_thread_data, hum_temp_stack, K_THREAD_STACK_SIZEOF(hum_temp_stack),
                                   hun_temp_sensor_thread, NULL, NULL, NULL, PRIORITY, 0, K_NO_WAIT);
    pressure_tid = k_thread_create(&pressure_thread_data, pressure_stack, K_THREAD_STACK_SIZEOF(pressure_stack),
                                   pressure_sensor_thread, NULL, NULL, NULL, PRIORITY, 0, K_NO_WAIT);
    imu_tid = k_thread_create(&imu_thread_data, imu_stack, K_THREAD_STACK_SIZEOF(imu_stack), imu_sensor_thread, NULL,
                              NULL, NULL, PRIORITY, 0, K_NO_WAIT);
    return 0;
}

static int shell_stop_hum_temp_thread(const struct shell *sh, size_t argc, char **argv)
{
    terminate_hum_temp_thread = true;
    k_thread_join(hum_temp_tid, K_FOREVER);
    return 0;
}

static int shell_stop_pressure_thread(const struct shell *sh, size_t argc, char **argv)
{
    terminate_pressure_thread = true;
    k_thread_join(pressure_tid, K_FOREVER);
    return 0;
}

static int shell_stop_imu_thread(const struct shell *sh, size_t argc, char **argv)
{
    terminate_imu_thread = true;
    k_thread_join(imu_tid, K_FOREVER);
    return 0;
}

static int shell_stop_all_sensors(const struct shell *sh, size_t argc, char **argv)
{
    terminate_hum_temp_thread = true;
    terminate_pressure_thread = true;
    terminate_imu_thread = true;

    k_thread_join(hum_temp_tid, K_FOREVER);
    k_thread_join(pressure_tid, K_FOREVER);
    k_thread_join(imu_tid, K_FOREVER);

    return 0;
}

static void sensor_storage_thread(void *a, void *b, void *c)
{
    LOG_INF("Sensor storage thread started.");
    k_mutex_lock(&sensor_data_mutex, K_FOREVER);
    littlefs_save_sensor_data(&sensor_data);
    k_mutex_unlock(&sensor_data_mutex);
    // while (!terminate_storage_thread)
    // {
    //     k_mutex_lock(&sensor_data_mutex, K_FOREVER);
    //     int rc = littlefs_save_sensor_data(&sensor_data);
    //     k_mutex_unlock(&sensor_data_mutex);

    //     if (rc < 0)
    //     {
    //         LOG_ERR("Failed to save sensor data to LittleFS (%d)", rc);
    //     }
    //     else
    //     {
    //         LOG_INF("Sensor data saved to LittleFS.");
    //     }

    //     k_sleep(STORAGE_INTERVAL);
    // }

    LOG_INF("Sensor storage thread stopped.");
}

static int shell_start_storage_thread(const struct shell *sh, size_t argc, char **argv, void *data)
{
    terminate_storage_thread = false;
    storage_tid = k_thread_create(&storage_thread_data, storage_stack, K_THREAD_STACK_SIZEOF(storage_stack),
                                  sensor_storage_thread, NULL, NULL, NULL, STORAGE_PRIORITY, 0, K_NO_WAIT);
    return 0;
}

static int shell_stop_storage_thread(const struct shell *sh, size_t argc, char **argv)
{
    terminate_storage_thread = true;
    k_thread_join(storage_tid, K_FOREVER);
    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(
    sub_demo, SHELL_CMD(start_hum_temp, NULL, "Start HTS221 thread", shell_start_hum_temp_thread),
    SHELL_CMD(start_pressure, NULL, "Start LPS22HB thread", shell_start_pressure_thread),
    SHELL_CMD(start_imu, NULL, "Start LSM6DSL thread", shell_start_imu_thread),
    SHELL_CMD(start, NULL, "Start all sensor threads", shell_start_all_sensors),
    SHELL_CMD(stop_hum_temp, NULL, "Stop HTS221 thread", shell_stop_hum_temp_thread),
    SHELL_CMD(stop_pressure, NULL, "Stop LPS22HB thread", shell_stop_pressure_thread),
    SHELL_CMD(stop_imu, NULL, "Stop LSM6DSL thread", shell_stop_imu_thread),
    SHELL_CMD(stop, NULL, "Stop all sensor thread", shell_stop_all_sensors),
    SHELL_CMD(start_storage, NULL, "Start sensor storage thread", shell_start_storage_thread),
    SHELL_CMD(stop_storage, NULL, "Stop sensor storage thread", shell_stop_storage_thread), SHELL_SUBCMD_SET_END);
/* Creating root (level 0) command "demo" */
SHELL_CMD_REGISTER(sensor, &sub_demo, "Sensor Demo commands", NULL);
