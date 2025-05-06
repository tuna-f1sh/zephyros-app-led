.. _sample-multi-node:

multi_node sample
=================

Overview
--------

The `multi_node` sample demonstrates how to use the `app_led` API to control
multiple LEDs on a board. It initializes three LEDs and applies different
effects:

- LED0: blinks at 1 Hz.
- LED1: blinks at 2 Hz.
- LED2: breathes continuously.

The sample uses Device Tree aliases (``DT_ALIAS(appled0)``, etc.) to identify
the LEDs.

Prerequisites
-------------

The sample requires the following Zephyr components (enabled in prj.conf):

- CONFIG_LOG
- CONFIG_LED
- CONFIG_PWM
- CONFIG_LED_PWM
- CONFIG_APP_LED

Board Overlays
--------------

Board-specific overlay files are provided in the ``boards/`` directory to
define the LED Device Tree nodes and DT aliases for each supported board:

- ``nucleo_l432kc.overlay``
- ``stm32l476g_disco.overlay``
- ``nrf5340dk_nrf5340_cpuapp.overlay``

These overlays create the aliases ``appled0``, ``appled1``, and ``appled2``, which
are used by the application.

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

Use West to build and flash the sample:

.. code-block:: console

   west build -b <board> samples/multi_node
   west flash

Running
-------

After flashing, the application will run automatically on the board. Use a
serial terminal at 115200 baud to view the log messages and observe the LED
behaviors as described above.
