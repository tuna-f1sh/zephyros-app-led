/**
 * @file main.c
 * @brief App LED mutli LED demo
 *
 * Demos using different LED types in the same application
 *
 */
#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/led_strip.h>

#include <app_led/led.h>

LOG_MODULE_REGISTER(demo_app_led, LOG_LEVEL_INF);

#if DT_HAS_ALIAS(led_strip) && IS_ENABLED(CONFIG_LED_STRIP)
#define STRIP_LED_NODE_ID DT_ALIAS(led_strip)
/* APP_LED_STATIC_STRIP_DEFINE can be used but this helper gets NUM_LEDS from the chain-length
 * property of the led_strip node.
 */
APP_LED_STATIC_STRIP_DEFINE(strpled, STRIP_LED_NODE_ID);
#endif

#if DT_HAS_COMPAT_STATUS_OKAY(pwm_leds) && IS_ENABLED(CONFIG_LED_PWM)
#define PWM_LED_NODE_ID	 DT_COMPAT_GET_ANY_STATUS_OKAY(pwm_leds)
#define PWM_LED_NUM_LEDS DT_CHILD_NUM(PWM_LED_NODE_ID)
APP_LED_STATIC_DEFINE(pwmled, PWM_LED_NODE_ID, PWM_LED_NUM_LEDS, CONFIG_APP_LED_PIN_RGB);
#endif

#if DT_HAS_COMPAT_STATUS_OKAY(gpio_leds) && IS_ENABLED(CONFIG_LED)
#define GPIO_LED_NODE_ID  DT_COMPAT_GET_ANY_STATUS_OKAY(gpio_leds)
#define GPIO_LED_NUM_LEDS DT_CHILD_NUM(GPIO_LED_NODE_ID)
APP_LED_STATIC_DEFINE(gpioled, GPIO_LED_NODE_ID, GPIO_LED_NUM_LEDS, CONFIG_APP_LED_PIN_RGB);
#endif

int main(void)
{
#if DT_HAS_COMPAT_STATUS_OKAY(pwm_leds) && IS_ENABLED(CONFIG_LED_PWM)
	LOG_INF("PWM LED found, initializing");
	if (app_led_init(&pwmled) != 0) {
		LOG_ERR("Failed to initialize %s", pwmled.app_led->name);
		return 1;
	}

	/* Set PWM RGB LED to Rainbow mode*/
	app_led_set_global_brightness(&pwmled, 127, K_MSEC(100));
	app_led_set_mode(&pwmled, Rainbow, K_MSEC(100));
#endif

#if DT_HAS_ALIAS(led_strip) && IS_ENABLED(CONFIG_LED_STRIP)
	LOG_INF("LED strip found, initializing");
	if (app_led_init(&strpled) != 0) {
		LOG_ERR("Failed to initialize %s", strpled.app_led->name);
		return 1;
	}

	/* Set strip to sine sequence, white */
	app_led_set_global_brightness(&strpled, 127, K_MSEC(100));
	app_led_set_global_color(&strpled, RGBHEX(White), K_MSEC(100));
	app_led_run_sequence(&strpled, app_led_sine_sequence, -1, K_MSEC(100));
#endif

#if DT_HAS_COMPAT_STATUS_OKAY(gpio_leds) && IS_ENABLED(CONFIG_LED)
	LOG_INF("GPIO LED found, initializing");
	if (app_led_init(&gpioled) != 0) {
		LOG_ERR("Failed to initialize %s", gpioled.app_led->name);
		return 1;
	}

	/* Use GPIO LED to indicate activity */
	while (1) {
		app_led_indicate_act(&gpioled, Orange);
		k_sleep(K_MSEC(100));
	}
#endif

	return 0;
}
