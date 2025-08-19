#ifndef SENSOR_SHARED_H
#define SENSOR_SHARED_H

#include <zephyr/kernel.h>

struct sensor_data_t {
    float temperature;
    float humidity;
    float pressure;
    float accel_x;
    float accel_y;
    float accel_z;
    float gyro_x;
    float gyro_y;
    float gyro_z;
};

extern struct sensor_data_t sensor_data;
extern struct k_mutex sensor_data_mutex;

#endif
