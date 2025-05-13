.. _sample-demo-led:

demo_led sample
===============

Overview
--------

The ``demo_led`` sample exercises the Zephyros App LED API on a single LED
device, which may be exposed via one of the following drivers:

- GPIO-based LEDs (CONFIG_LED)
- PWM-driven RGB LED clusters (CONFIG_LED_PWM)
- SPI-based LED strips (CONFIG_LED_STRIP)

At runtime the sample picks the enabled driver and performs a variety of
LED operations in sequence:

- Manual color and brightness settings (Red, Green)
- Blink modes with blocking waits
- Activity indication blinks
- Custom sequences (test, sine)
- Rainbow mode
- Clearing sequences and fading off

Prerequisites
-------------

Ensure the following Zephyr features are enabled (in ``prj.conf`` and variants):

- CONFIG_LOG
- CONFIG_APP_LED
- At least one LED driver:
  - CONFIG_LED
  - CONFIG_PWM and CONFIG_LED_PWM
  - CONFIG_LED_STRIP

Board Overlays
--------------

Board-specific Device Tree overlay files in ``boards/`` define the LED nodes
and aliases used by the sample:

- ``gpio-leds`` node for discrete GPIO LEDs
- ``pwm-leds`` node for PWM-controlled RGB LEDs
- ``worldsemi,ws2812-spi`` node (alias ``led_strip``) for LED strips

Supported boards include:

- nucleo_l432kc
- stm32l476g_disco
- nrf5340dk_nrf5340_cpuapp

Configuration Variants
----------------------

- ``prj.conf``: Default configuration enabling GPIO LEDs
- ``prj_pwm.conf``: Enables PWM and PWM-based RGB LEDs
- ``prj_ws2812.conf``: Enables SPI-based LED strips

Test variants are defined in ``sample.yaml``, including a no-workqueue mode
(CONFIG_APP_LED_USE_WORKQUEUE=n) and driver-specific builds.

Project Structure
-----------------

- CMakeLists.txt       Sample build file
- Kconfig              Sample Kconfig options
- prj.conf             Default project configuration
- prj_pwm.conf         PWM LED variant configuration
- prj_ws2812.conf      WS2812 LED strip variant configuration
- sample.yaml          Metadata for Zephyr sample framework and tests
- boards/              Board-specific Device Tree overlays
- src/main.c           Application source code

Building and Flashing
---------------------

.. code-block:: console

   west build -b <board> samples/demo_led
   west flash

Running
-------

After flashing, the sample runs immediately. Open a serial console at 115200
baud to observe log messages and LED behaviors in the following order:

- "Zephyros App_LED Demo Application Started"
- "Setting color to Red", then "Green"
- "Setting mode to Blink..." (Blue, Pink)
- Activity blinks via app_led_indicate_act()
- "Setting mode to Sequence..." (test, sine)
- "Setting mode to Rainbow"
- Clearing and fade-off

To run all sample tests via Twister:

.. code-block:: console

   twister -s samples/demo_led