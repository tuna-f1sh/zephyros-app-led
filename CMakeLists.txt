if (CONFIG_APP_LED)
  zephyr_include_directories(include)
  zephyr_library_sources(led.c)
  zephyr_library_sources(led_sequence.c)
endif()
