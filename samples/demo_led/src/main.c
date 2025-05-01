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

#include <app_led/led.h>

LOG_MODULE_REGISTER(demo_app_led, LOG_LEVEL_INF);

#if IS_ENABLED(CONFIG_LED_STRIP)
#include <zephyr/drivers/led_strip.h>
#define LED_NODE_ID	 DT_ALIAS(led_strip)

#elif IS_ENABLED(CONFIG_LED_PWM)

#include <zephyr/drivers/pwm.h>
#define LED_NODE_ID	       DT_COMPAT_GET_ANY_STATUS_OKAY(pwm_leds)
#define NUM_LEDS	       DT_CHILD_NUM(LED_NODE_ID)

#elif IS_ENABLED(CONFIG_LED)

#include <zephyr/drivers/gpio.h>
#define LED_NODE_ID	       DT_COMPAT_GET_ANY_STATUS_OKAY(gpio_leds)
#define NUM_LEDS	       DT_CHILD_NUM(LED_NODE_ID)

#else
#error "CONFIG_LED_STRIP, CONFIG_LED or CONFIG_LED_PWM must be set"
#endif

#if IS_ENABLED(CONFIG_LED_STRIP)
/* APP_LED_STATIC_STRIP_DEFINE can be used but this helper gets NUM_LEDS from the chain-length
 * property of the led_strip node.
 */
APP_LED_STATIC_STRIP_DEFINE(aled, LED_NODE_ID);
#else
APP_LED_STATIC_DEFINE(aled, LED_NODE_ID, NUM_LEDS, CONFIG_APP_LED_PIN_RGB);
#endif

int main(void)
{
	LOG_INF("Zephyros App_LED Demo Application Started");

	/* Initialize the app_led module and internal update thread */
	int ret = app_led_init(&aled);
	if (ret != 0) {
		LOG_ERR("App LED module initialization failed: %d", ret);
		return 1;
	}

	/* Set solid colors and brightness in manual control mode (default)
	 * acts like a normal LED so no state machine.
	 */
	LOG_INF("Setting color to Red");
	app_led_set_global_brightness(&aled, 255, K_MSEC(100));
	app_led_set_global_color(&aled, RGBHEX(Red), K_MSEC(100));
	k_sleep(K_SECONDS(1));

	LOG_INF("Setting color to Green");
	app_led_set_global_brightness(&aled, 128, K_MSEC(100));
	app_led_set_global_color(&aled, RGBHEX(Green), K_MSEC(100));
	k_sleep(K_SECONDS(1));

	/* Blink the LED async from App logic */
	LOG_INF("Setting mode to Blink (Blue, 200/200 ms on/off)");
	app_led_blink(&aled, RGBHEX(Blue), 200, 200, true, K_MSEC(100));
	/* The app_led_wait_x() functions are blocking callthat will wait for the
	 * blink to finish or the timeout to expire - so this will block for ~400 ms
	 *
	 * Useful to wait for blink to complete before blinking again as below
	 */
	app_led_wait_blink(&aled, K_SECONDS(2));
	LOG_INF("Setting mode to Blink (Pink, 600/200 ms on/off)");
	app_led_blink(&aled, RGBHEX(DarkCyan), 600, 400, true, K_MSEC(100));
	app_led_wait_blink(&aled, K_SECONDS(2));

	/* Example of using the app_led_indicate_act() function to indicate activity
	 * with a short blink of the LED. This is not a blocking call, so it can be
	 * used in a loop or in an interrupt context.
	 *
	 * It will maintain blink period (50 ms) by not overriding so visable even if called at faster rate.
	 */
	for (int i = 0; i < 2000; i++) {
		app_led_indicate_act(&aled, Orange);
		k_sleep(K_MSEC(1));
	}

	/* Run a sequence async from App - see led.h for more details of other sequences */
	LOG_INF("Setting mode to Sequence (Test Sequence, repeat forever)");
	/* -1 is repeat indefinitely, 0 is run once, >0 is number of repeats */
	app_led_run_sequence(&aled, app_led_test_sequence, -1,
			     K_MSEC(100));
	/* Block again, since it's -1 it will wait for 3 seconds to let the sequence run */
	app_led_wait_sequence(&aled, K_SECONDS(3));

	/* Run sine wave 5 times */
	app_led_run_sequence(&aled, app_led_sine_sequence, 5,
			     K_MSEC(100));
	app_led_wait_sequence(&aled, K_SECONDS(5));

	/* Run rainbow mode; increases hue with each app_led task update to create a rainbow
	 * effect async to App.
	 */
	LOG_INF("Setting mode to Rainbow");
	app_led_set_mode(&aled, Rainbow, K_MSEC(100));
	app_led_set_global_brightness(&aled, 255, K_MSEC(100));
	/* Block App for 5 seconds to show rainbow */
	k_sleep(K_SECONDS(5));
	app_led_set_mode(&aled, Manual, K_MSEC(100));

	/* Clear and fade off the LED */
	LOG_INF("Clearing sequence/blink and turning off");
	/*  Clear sequence - we know already clear but just for demo */
	app_led_sequence_clear(&aled, K_MSEC(100));
	app_led_set_global_color(&aled, RGBHEX(White), K_MSEC(100));
	app_led_fade_off(&aled, 1000, K_MSEC(100));

	LOG_INF("Demo finished cycle.");

	return 0;
}
