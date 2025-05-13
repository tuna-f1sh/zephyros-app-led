.. _sample-multi-led:

multi_led sample
================

Overview
--------

The ``multi_led`` sample demonstrates how to use the ``app_led`` API to control
multiple types of LEDs in a single application. It auto-detects and initializes:

- PWM-based RGB LED cluster: runs a rainbow animation.
- SPI-based LED strip (e.g., WS2812): runs a sine-wave sequence in white.
- GPIO-based user LEDs: indicates activity by blinking in "Orange" color.

When supported by the board (via Device Tree and enabled configs), all three
LED types run concurrently.

Prerequisites
-------------

Ensure the following Zephyr features are enabled in ``prj.conf``:

- CONFIG_LOG
- CONFIG_GPIO
- CONFIG_LED
- CONFIG_PWM
- CONFIG_LED_PWM
- CONFIG_LED_STRIP
- CONFIG_APP_LED

Board Overlays
--------------

Board-specific overlay files are provided in the ``boards/`` directory to define
LED Device Tree nodes and aliases:

- ``nucleo_l432kc.overlay``
- ``stm32l476g_disco.overlay``
- ``nrf5340dk_nrf5340_cpuapp.overlay``

These overlays define:

- A ``gpio-leds`` node for discrete GPIO LEDs.
- A ``pwm-leds`` node for PWM-controlled RGB LEDs.
- A ``worldsemi,ws2812-spi`` node (alias ``led_strip``) for LED strips.
- Appropriate DT aliases and labels for each LED node.

Configuration
-------------

Select the LED pin mode via ``Kconfig``:

- ``APP_LED_PIN_INDIVIDUAL``: Discrete LEDs on individual GPIO pins.
- ``APP_LED_PIN_RGB``: RGB LED connected to three pins.

The default is ``APP_LED_PIN_RGB``.

Project Structure
-----------------

- ``CMakeLists.txt``: Sample build file.
- ``prj.conf``: Project configuration file.
- ``Kconfig``: Sample Kconfig options.
- ``sample.yaml``: Metadata for Zephyr sample framework.
- ``boards/``: Board-specific Device Tree overlays.
- ``src/main.c``: Application source code.

Building and Flashing
---------------------

.. code-block:: console

   west build -b <board> samples/multi_led
   west flash

Running
-------

After flashing, the application runs automatically. Monitor the console at
115200 baud to view log messages and observe the LED behaviors:

- "PWM LED found, initializing": Rainbow animation on the PWM RGB LED.
- "LED strip found, initializing": Sine-wave sequence on the LED strip.
- "GPIO LED found, initializing": Activity indication blinking on the GPIO LED.

All sequences run concurrently, demonstrating multiple ``app_led`` instances.