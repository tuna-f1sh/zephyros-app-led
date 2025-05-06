#include <zephyr/ztest.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/gpio/gpio_emul.h>
#include <zephyr/input/input.h>
#include <string.h>
#include <stdlib.h>
#include <zephyr/logging/log.h>

#include <app_led/led.h>

LOG_MODULE_DECLARE(app_led_test, LOG_LEVEL_DBG);

// --- Test Configuration & Nodes ---
#define GPIO_LED_NODE  DT_ALIAS(gpio_leds)
#define RGB_GPIO_NODE  DT_ALIAS(rgb_gpio_leds)
#define GPIO_EMUL_NODE DT_ALIAS(gpio_emulator)

// --- Define Test Instances using the app_led macro ---
// Kconfig handles max array size now. Pass explicit HW LED count.
APP_LED_STATIC_DEFINE(gpio_led_inst, GPIO_LED_NODE, 1, 0);     // 1 HW LED, not RGB
APP_LED_STATIC_DEFINE(rgb_gpio_led_inst, RGB_GPIO_NODE, 3, 1); // 3 HW LEDs, is RGB

// --- Test Fixture ---
struct app_led_gpio_fixture {
	app_led_data_t *gpio;		    // Pointer to the single GPIO LED instance
	app_led_data_t *rgb_gpio;	    // Pointer to the RGB GPIO LED instance
	const struct device *gpio_emul_dev; // Emulator device instance
};

// --- Emulator Helper ---
// static int event_count;
// static uint16_t last_code;
// static bool last_val;
// static void test_gpio_keys_cb_handler(struct input_event *evt, void *user_data)
// {
// 	LOG_INF("GPIO_KEY %s pressed, zephyr_code=%u, value=%d\n",
// 		 evt->dev->name, evt->code, evt->value);
// 	event_count++;
// 	last_code = evt->code;
// 	last_val = evt->value;
// }
// INPUT_CALLBACK_DEFINE(DEVICE_DT_GET(GPIO_EMUL_NODE), test_gpio_keys_cb_handler, NULL);

// Gets the output state (0 or 1) of an emulated GPIO pin
static int get_gpio_pin_state(const struct device *emul_dev, uint32_t pin)
{
	return gpio_emul_output_get(emul_dev, pin);
}

// --- Test Setup ---
static void *app_led_gpio_setup(void)
{
	// Called once before all tests
	struct app_led_gpio_fixture *fixture = malloc(sizeof(struct app_led_gpio_fixture));

	fixture->gpio = &gpio_led_inst;
	fixture->rgb_gpio = &rgb_gpio_led_inst;
	fixture->gpio_emul_dev = DEVICE_DT_GET(GPIO_EMUL_NODE);

	// Initialize app_led instances (should check underlying GPIO device readiness)
	zassert_ok(app_led_init(fixture->gpio), "GPIO LED Init failed");
	zassert_ok(app_led_init(fixture->rgb_gpio), "RGB GPIO LED Init failed");

	return fixture;
}

static void app_led_gpio_before(void *f)
{
	// Called before each test function
	struct app_led_gpio_fixture *fixture = f;

	// Reset LED state to known defaults
	app_led_set_mode(fixture->gpio, Manual, K_NO_WAIT);
	app_led_set_global_color(fixture->gpio, RGBHEX(Black), K_NO_WAIT); // Off
	app_led_set_global_brightness(fixture->gpio, 0xFF, K_NO_WAIT);	   // Full brightness
	app_led_sequence_clear(fixture->gpio, K_NO_WAIT);
	// Reset blink state if needed (e.g., zero out timers in state array)
	fixture->gpio->state[0].on_time_ms_left = 0;
	fixture->gpio->state[0].off_time_ms_left = 0;

	app_led_set_mode(fixture->rgb_gpio, Manual, K_NO_WAIT);
	app_led_set_global_color(fixture->rgb_gpio, RGBHEX(Black), K_NO_WAIT); // Off
	app_led_set_global_brightness(fixture->rgb_gpio, 0xFF, K_NO_WAIT);     // Full brightness
	app_led_sequence_clear(fixture->rgb_gpio, K_NO_WAIT);
	// Reset blink state if needed...

#if IS_ENABLED(CONFIG_APP_LED_USE_WORKQUEUE)
	// Ensure work items are cancelled before each test
	(void)k_work_cancel_delayable(&fixture->gpio->dwork);
	(void)k_work_cancel_delayable(&fixture->rgb_gpio->dwork);
	// Allow cancellations to process
	k_sleep(K_MSEC(1));
#endif
}

static void app_led_teardown(void *f)
{
	free(f);
}

// Helper to run update logic (assuming work queue)
static void run_update(app_led_data_t *leds)
{
#if IS_ENABLED(CONFIG_APP_LED_USE_WORKQUEUE)
	// Directly call the handler to simulate the work queue running
	// This bypasses scheduler delays for immediate testing of state changes
	k_sleep(K_MSEC(20));
#else
	// If using thread, need different mechanism (e.g., yield + sleep)
	app_led_update(leds); // Call directly if no async mechanism
#endif
}

// --- Test Suite Definition ---
ZTEST_SUITE(app_led_gpio, NULL, app_led_gpio_setup, app_led_gpio_before, NULL, app_led_teardown);

ZTEST_F(app_led_gpio, test_gpio_led_brightness)
{
	app_led_set_global_brightness(fixture->gpio, 50, K_NO_WAIT);	   // Set to 50% brightness
	app_led_set_global_color(fixture->gpio, RGBHEX(White), K_NO_WAIT); // Set color to white
	run_update(fixture->gpio); // Run update logic
	zassert_equal(fixture->gpio->global_brightness, 50, "Brightness not set correctly");
	zassert_equal(get_gpio_pin_state(fixture->gpio_emul_dev, 0), 1, "GPIO pin not set to high");
}
