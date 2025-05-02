/**
 * @file main.c
 * @brief App LED demo application
 *
 * Demos main app_led API with a single RGB LED strip or GPIO/PWM LEDs
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

#if IS_ENABLED(CONFIG_LED_STRIP)
#define STRIP_LED_NODE_ID DT_ALIAS(led_strip)
/* APP_LED_STATIC_STRIP_DEFINE can be used but this helper gets NUM_LEDS from the chain-length
 * property of the led_strip node.
 */
APP_LED_STATIC_STRIP_DEFINE(strpled, STRIP_LED_NODE_ID);
#endif

#if IS_ENABLED(CONFIG_LED_PWM)
#define PWM_LED_NODE_ID	 DT_COMPAT_GET_ANY_STATUS_OKAY(pwm_leds)
#define PWM_LED_NUM_LEDS DT_CHILD_NUM(PWM_LED_NODE_ID)
APP_LED_STATIC_DEFINE(pwmled, PWM_LED_NODE_ID, PWM_LED_NUM_LEDS, CONFIG_APP_LED_PIN_RGB);
#endif

#if IS_ENABLED(CONFIG_LED)
#define GPIO_LED_NODE_ID  DT_COMPAT_GET_ANY_STATUS_OKAY(gpio_leds)
#define GPIO_LED_NUM_LEDS DT_CHILD_NUM(GPIO_LED_NODE_ID)
APP_LED_STATIC_DEFINE(gpioled, GPIO_LED_NODE_ID, GPIO_LED_NUM_LEDS, CONFIG_APP_LED_PIN_RGB);
#endif

int main(void)
{
	LOG_INF("Zephyros App_LED Demo Application Started");

	/* Initialize the app_led module and internal update thread */
#if IS_ENABLED(CONFIG_LED)
	int ret = app_led_init(&gpioled);
	if (ret != 0) {
		LOG_ERR("Failed to initialize %s", gpioled.app_led->name);
		return 1;
	}
#endif
#if IS_ENABLED(CONFIG_LED_PWM)
	ret = app_led_init(&pwmled);
	if (ret != 0) {
		LOG_ERR("Failed to initialize %s", pwmled.app_led->name);
		return 1;
	}
#endif
#if IS_ENABLED(CONFIG_LED_STRIP)
	ret = app_led_init(&strpled);
	if (ret != 0) {
		LOG_ERR("Failed to initialize %s", strpled.app_led->name);
		return 1;
	}
#endif

#if IS_ENABLED(CONFIG_LED_PWM)
	/* Set PWM RGB LED to Rainbow mode*/
	app_led_set_global_brightness(&pwmled, 127, K_MSEC(100));
	app_led_set_mode(&pwmled, Rainbow, K_MSEC(100));
#endif

#if IS_ENABLED(CONFIG_LED_STRIP)
	/* Set strip to chase sequence */
	app_led_set_global_brightness(&strpled, 127, K_MSEC(100));
	app_led_run_sequence(&strpled, app_led_sine_sequence, -1, K_MSEC(100));
#endif

#if IS_ENABLED(CONFIG_LED)
	/* Use GPIO LED to indicate activity */
	while (1) {
		app_led_indicate_act(&gpioled, Orange);
		k_sleep(K_MSEC(100));
	}
#endif

	return 0;
}
