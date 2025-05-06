/**
 * @file main.c
 * @brief App LED multi dts node demo application
 *
 * Demos using aliases of LED clusters to control independant App LEDs
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

APP_LED_STATIC_IDV_DEFINE(led0, DT_ALIAS(appled0));
APP_LED_STATIC_IDV_DEFINE(led1, DT_ALIAS(appled1));
APP_LED_STATIC_IDV_DEFINE(led2, DT_ALIAS(appled2));

int main(void)
{
	if (app_led_init(&led0) != 0) {
		LOG_ERR("Failed to initialize %s", led0.app_led->name);
		return 1;
	}
	if (app_led_init(&led1) != 0) {
		LOG_ERR("Failed to initialize %s", led1.app_led->name);
		return 1;
	}
	if (app_led_init(&led2) != 0) {
		LOG_ERR("Failed to initialize %s", led2.app_led->name);
		return 1;
	}

	/* 3rd LED set to breathe sequence forever */
	app_led_set_global_color(&led2, RGBHEX(White), K_MSEC(100));
	app_led_run_sequence(&led2, app_led_breathe_sequence, -1, K_MSEC(100));

	while (true) {
		/* Other two blink at different rates */
		app_led_blink(&led0, RGBHEX(White), 100, 100, false, K_MSEC(100));
		app_led_blink(&led1, RGBHEX(White), 50, 50, false, K_MSEC(100));
		k_sleep(K_MSEC(10));
	}

	return 0;
}
