cmake_minimum_required(VERSION 3.20.5)

list(APPEND ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_LIST_DIR}/../../)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(multi_led)

target_sources(app PRIVATE src/main.c)
