An attempt to solve the problem of App LED management that is so often required in projects: blinking, sequences, fading etc. A blinkly LED is the 'hello world' of embedded but within a RTOS if often becomes complex. This library aims to provide a simple and clean way to manage LED's in your project.

Supports LEDs connected to GPIO `CONFIG_GPIO`/`CONFIG_LED`, PWM `CONFIG_PWM`/`CONFIG_LED_PWM` and LED `CONFIG_LED_STRIP` strip drivers all at once or exclusively.

## Usage

Enable the module (CONFIG_APP_LED=y) and include the App LED header in your application:

```
#include <app_led/led.h>
```

Define and initialize an LED instance (GPIO/PWM). Can be discrete LEDs or RGB LEDs:

```
/* For GPIO or PWM LEDs */
APP_LED_STATIC_DEFINE(app_led, DT_ALIAS(appled0), DT_CHILD_NUM(appled0), IS_RGB);
int err = app_led_init(&app_led);
if (err) { /* handle error */ }
```

Define and initialize an LED strip (WS2812):

```
/* For LED strips */
APP_LED_STATIC_STRIP_DEFINE(app_strip, DT_ALIAS(led_strip));
app_led_init(&app_strip);
```

Basic operations:

```
/* Manual brightness and color */
app_led_set_global_brightness(&app_led, 255, K_MSEC(100));
app_led_set_global_color(&app_led, RGBHEX(Red), K_MSEC(100));

/* Blink asynchronously (color, on_ms, off_ms, async, timeout) */
app_led_blink(&app_led, RGBHEX(Blue), 200, 200, true, K_MSEC(100));
app_led_wait_blink(&app_led, K_SECONDS(2));

/* Run built-in sequences (sine, breathe, test, etc.) */
app_led_run_sequence(&app_led, app_led_sine_sequence, 5, K_MSEC(50));
app_led_wait_sequence(&app_led, K_SECONDS(3));

/* Set mode (Rainbow, Manual) */
app_led_set_mode(&app_led, Rainbow, K_MSEC(100));

/* Activity indication (short blink) */
app_led_indicate_act(&app_led, Orange);

/* Clear sequences and fade off */
app_led_sequence_clear(&app_led, K_MSEC(100));
app_led_fade_off(&app_led, 1000, K_MSEC(100));
```

Key Kconfig options:

- CONFIG_APP_LED_USE_WORKQUEUE: Enable workqueue auto-updates (default: y).
- CONFIG_APP_LED_UPDATE_INTERVAL: LED update interval (ms).

See the samples under samples/multi_node, samples/multi_led, and samples/demo_led for complete examples.

## Work in Progress

I have ported this from projects into a module in an attempt to make it more generic and reusable. It's still a work in progress but I'll try to keep the API constant without major releases.

### TODO

- [x] Ability to use GPIO, PWM and LED strip drivers independently but also together with a common API but passed `app_led_data_t` struct for each.
- [x] ~~KConfig options to control priority, stack size, etc.~~ Now uses workqueue so system workqueue can configure this. Option to control update with `CONFIG_APP_LED_USE_WORKQUEUE=n`.
- [x] Samples.
- [ ] Tidy internal LED HW abstraction. Perhaps HW update callback in `app_led_data_t`?
- [ ] Test suite with ztest using native_sim or qemu.
