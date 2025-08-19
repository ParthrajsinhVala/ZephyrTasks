cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(13_Sensor_Data_Logging_using_LittleFS)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})