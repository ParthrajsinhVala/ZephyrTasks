#ifndef SENSOR_STORAGE_H
#define SENSOR_STORAGE_H

int littlefs_save_sensor_data(const struct sensor_data_t *data);
int littlefs_read_sensor_data();
#endif /* SENSOR_STORAGE_H */