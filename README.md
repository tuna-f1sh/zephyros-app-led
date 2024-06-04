An attempt to solve the problem of App LED management that is so often required in projects: blinking, sequences, fading etc. A blinkly LED is the 'hello world' of embedded but within a RTOS if often becomes complex. This library aims to provide a simple and clean way to manage LED's in your project.

Supports LEDs connected to GPIO `CONFIG_GPIO`/`CONFIG_LED`, PWM `CONFIG_PWM`/`CONFIG_LED_PWM` and LED `CONFIG_LED_STRIP` strip drivers.

# Work in Progress

I have ported this from projects into a module in an attempt to make it more generic and reusable. I am currently working on the documentation and testing. I will be adding more features and examples as I go.

## TODO

- [ ] Ability to use GPIO, PWM and LED strip drivers independently but also together with a common API but passed `app_led_data_t` struct for each.
- [ ] KConfig options to control priority, stack size, etc.
- [ ] Example code.
- [ ] `twister` tests.
