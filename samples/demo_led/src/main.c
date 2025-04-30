#include <zephyr/device.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "led.h"

LOG_MODULE_REGISTER(demo_app_led, LOG_LEVEL_INF);

int main(void)
{
	LOG_INF("Zephyros App_LED Demo Application Started");

	/* Initialize the app_led module and internal update thread */
	int ret = app_led_init();
	if (ret != 0) {
		LOG_ERR("App LED module initialization failed: %d", ret);
		return 1;
	}

	// --- Cycle through different LED modes and actions ---

	// 1. Set solid RED color (Manual Mode)
	LOG_INF("Setting color to Red");
	app_led_set_global_brightness(&rgbled, 255, K_MSEC(100));
	app_led_set_global_color(&rgbled, RGBHEX(Red), K_MSEC(100));
	k_sleep(K_SECONDS(1));

	// 2. Set solid GREEN color
	LOG_INF("Setting color to Green");
	app_led_set_global_color(&rgbled, RGBHEX(Green), K_MSEC(100));
	app_led_set_global_brightness(&rgbled, 128, K_MSEC(100));
	k_sleep(K_SECONDS(1));

	// 3. Blink BLUE
	LOG_INF("Setting mode to Blink (Blue, 200ms on/off)");
	app_led_blink(&rgbled, RGBHEX(Blue), 200, 200, true, K_MSEC(100));
	app_led_wait_blink(&rgbled, K_SECONDS(2));
	app_led_blink(&rgbled, RGBHEX(Blue), 200, 200, true, K_MSEC(100));
	app_led_wait_blink(&rgbled, K_SECONDS(2));

	// 4. Run the Test Sequence (Red, Green, Blue) - repeat forever
	LOG_INF("Setting mode to Sequence (Test Sequence, repeat forever)");
	// Note: app_led_run_sequence automatically sets mode to Sequence
	app_led_run_sequence(&rgbled, app_led_test_sequence, -1,
			     K_MSEC(100)); // -1 repeats forever
	app_led_wait_sequence(&rgbled, K_SECONDS(5));

	// 5. Run Rainbow mode
	LOG_INF("Setting mode to Rainbow");
	app_led_set_mode(&rgbled, Rainbow, K_MSEC(100));
	app_led_set_global_brightness(&rgbled, 255, K_MSEC(100));
	k_sleep(K_SECONDS(5)); // Let the rainbow run

	// 6. Clear sequence/blink and turn off (back to Manual, color Black)
	LOG_INF("Clearing sequence/blink and turning off");
	app_led_sequence_clear(&rgbled, K_MSEC(100));		 // Clears sequence state
	app_led_blink_sync(&rgbled, RGBHEX(Black), K_MSEC(100)); // Clears blink state
	app_led_set_mode(&rgbled, Manual,
			 K_MSEC(100)); // Set back to manual explicitly
	app_led_set_global_color(&rgbled, RGBHEX(White),
				 K_MSEC(100)); // Set color to Black
	app_led_fade_off(&rgbled, 1000, K_MSEC(100));

	LOG_INF("Demo finished cycle.");

	return 0;
}
