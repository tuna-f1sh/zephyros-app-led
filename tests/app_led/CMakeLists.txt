cmake_minimum_required(VERSION 3.20.0)
set(ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_LIST_DIR}/../../)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(app_led_test)

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})
