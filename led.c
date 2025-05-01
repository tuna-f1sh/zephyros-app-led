#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>
#include <errno.h>
#include <math.h>
#include <stdlib.h>
#if IS_ENABLED(CONFIG_LED_STRIP)
#include <zephyr/drivers/led_strip.h>
#endif
#if IS_ENABLED(CONFIG_LED_PWM)
#include <zephyr/drivers/pwm.h>
#endif
#if IS_ENABLED(CONFIG_LED_GPIO)
#include <zephyr/drivers/gpio.h>
#endif

#include <app_led/led.h>

LOG_MODULE_REGISTER(app_led, CONFIG_APP_LED_LOG_LEVEL);

#if IS_ENABLED(CONFIG_LED_STRIP)
static void leds_strip_update(app_led_data_t *leds)
{
	if (k_mutex_lock(&leds->mutex, K_FOREVER) == 0) {
		if (led_strip_update_rgb(leds->app_led, leds->pixels, leds->hw_num_leds) != 0) {
			LOG_ERR("Couldn't update strip");
		}

		k_mutex_unlock(&leds->mutex);
	}
}

/* Set the device level LED data and update app_led
 *
 * Zephyr LED strip uses a different RGB struct to the app_led struct so convert
 * to that. Work is submitted to update the LED strip because the LED strip
 * driver is not ISR safe.
 * */
static int led_set_strip_pixels(app_led_data_t *leds, uint16_t start, uint16_t end, rgb_color_t c,
				uint8_t brightness, k_timeout_t block)
{
	static struct led_rgb c_rgb;

	if (start < leds->hw_num_leds && end <= leds->hw_num_leds) {
		// scale
		c.r = (uint8_t)((uint16_t)c.r * brightness / 255);
		c.g = (uint8_t)((uint16_t)c.g * brightness / 255);
		c.b = (uint8_t)((uint16_t)c.b * brightness / 255);

		leds->_color = c;

		c_rgb = (struct led_rgb){
			.r = c.r,
			.g = c.g,
			.b = c.b,
		};

		struct led_rgb *pixels = (struct led_rgb *)leds->pixels;
		for (int i = start; i < end; i++) {
			memcpy(&pixels[i], &c_rgb, sizeof(struct led_rgb));
			leds->state[i]._color = c;
		}

		return 0;
	} else {
		LOG_ERR("LED index out of range");
		return -EINVAL;
	}
}
#endif

#if IS_ENABLED(CONFIG_LED_PWM)
static int leds_set_pwm_brightness(app_led_data_t *leds, uint8_t i, uint8_t value,
				   k_timeout_t block)
{
	const struct app_led_pwm_config *config = leds->app_led->config;
	const struct pwm_dt_spec *dt_led;
	int err;

	if (i >= config->num_leds || value > 255) {
		return -EINVAL;
	}

	dt_led = &config->led[i];

	if (k_mutex_lock(&leds->mutex, block) == 0) {
		err = pwm_set_pulse_dt(&config->led[i], dt_led->period * value / 255);
		k_mutex_unlock(&leds->mutex);
		return err;
	} else {
		return -EBUSY;
	}
}
#endif

#if IS_ENABLED(CONFIG_LED_GPIO)
static int leds_set_gpio_brightness(app_led_data_t *leds, uint8_t i, uint8_t value,
				    k_timeout_t block)
{
	const struct led_gpio_config *config = leds->app_led->config;
	const struct gpio_dt_spec *led_gpio;
	int err;

	if ((i >= config->num_leds) || (value > 255)) {
		return -EINVAL;
	}

	if (k_mutex_lock(&leds->mutex, block) == 0) {
		led_gpio = &config->led[i];

		// on if value > 127
		err = gpio_pin_set_dt(led_gpio, value > 127);
		k_mutex_unlock(&leds->mutex);
		return err;
	} else {
		return -EBUSY;
	}
}
#endif

static int leds_set_brightness(app_led_data_t *leds, uint8_t i, uint8_t value, k_timeout_t block)
{
	switch (leds->hw_type) {
#if IS_ENABLED(CONFIG_LED_PWM)
	case APP_LED_TYPE_PWM:
		return leds_set_pwm_brightness(leds, i, value, block);
#endif
#if IS_ENABLED(CONFIG_LED_GPIO)
	case APP_LED_TYPE_GPIO:
		return leds_set_gpio_brightness(leds, i, value, block);
#endif
	default:
		LOG_ERR("Unsupported LED type: should not be here!");
		return -EINVAL;
	}
}

static int leds_set_pin_pixel(app_led_data_t *leds, uint16_t i, rgb_color_t c, uint8_t brightness,
			      k_timeout_t block)
{
	int err;

	if (leds->is_rgb) {
		/* hw base index is *3 */
		if ((i * 3 + 2) < leds->hw_num_leds) {
			leds->state[i]._color = c;
			err = leds_set_brightness(leds, i, c.r, block);
			if (!err)
				err = leds_set_brightness(leds, i + 1, c.g, block);
			if (!err)
				err = leds_set_brightness(leds, i + 2, c.b, block);

			if (err != 0) {
				return err;
			}
		} else {
			LOG_ERR("LED index out of range");
			return -EINVAL;
		}
	} else {
		if (i < leds->hw_num_leds) {
			leds->state[i]._color = c;
#if IS_ENABLED(CONFIG_APP_LED_GRAYSCALE_WEIGHTED)
			uint8_t grayscale_brightness =
				(uint8_t)(0.299 * c.r + 0.587 * c.g + 0.114 * c.b);
#elif IS_ENABLED(CONFIG_APP_LED_GRAYSCALE_AVERAGE)
			uint8_t grayscale_brightness = (uint8_t)(((uint16_t)c.r + c.g + c.b) / 3);
#else
			uint8_t grayscale_brightness = c.hex > 0 ? 255 : 0;
#endif
			err = leds_set_brightness(leds, i, grayscale_brightness, block);

			if (err != 0) {
				return err;
			}
		} else {
			LOG_ERR("LED index out of range");
			return -EINVAL;
		}
	}

	return 0;
}

static int leds_set_pin_pixels(app_led_data_t *leds, uint16_t start, uint16_t end, rgb_color_t c,
			       uint8_t brightness, k_timeout_t block)
{
	int err;
	if (start < leds->num_leds && end <= leds->num_leds) {
		// scale
		c.r = (uint8_t)((uint16_t)c.r * brightness / 255);
		c.g = (uint8_t)((uint16_t)c.g * brightness / 255);
		c.b = (uint8_t)((uint16_t)c.b * brightness / 255);
		// TODO this is legacy and not representative of the actual color if changing sector
		// it's just used for toggle whole strip
		leds->_color = c;

		for (int i = start; i < end; i++) {
			err = leds_set_pin_pixel(leds, i, c, brightness, block);

			if (err != 0) {
				return err;
			}
		}
	} else {
		LOG_ERR("LED index out of range");
		return -EINVAL;
	}

	return 0;
}

static int leds_set_pixels(app_led_data_t *leds, uint16_t start, uint16_t end, rgb_color_t c,
			   uint8_t brightness, k_timeout_t block)
{
	switch (leds->hw_type) {
#if IS_ENABLED(CONFIG_LED_STRIP)
	case APP_LED_TYPE_STRIP:
		return led_set_strip_pixels(leds, start, end, c, brightness, block);
#endif
#if IS_ENABLED(CONFIG_LED_PWM)
	case APP_LED_TYPE_PWM:
#endif
#if IS_ENABLED(CONFIG_LED_GPIO)
	case APP_LED_TYPE_GPIO:
#endif
		return leds_set_pin_pixels(leds, start, end, c, brightness, block);
	default:
		LOG_ERR("Unsupported LED type: should not be here!");
		return -EINVAL;
	}
}

/* Set a single pixel on the LED strip */
static inline int leds_set_pixel(app_led_data_t *leds, uint16_t i, rgb_color_t c,
				 uint8_t brightness, k_timeout_t block)
{
	return leds_set_pixels(leds, i, i + 1, c, brightness, block);
}

/* Get the current set RGB value of a pixel as a rgb_color_t */
int app_led_get_pixel_rgb(const app_led_data_t *const leds, uint16_t i, rgb_color_t *c)
{
	if (i < leds->num_leds) {
		*c = leds->state[i]._color;
		return 0;
	} else {
		return -EINVAL;
	}
}

/* Convert HSV to RGB */
rgb_color_t app_led_hsv_to_rgb(uint8_t hue, uint8_t sat, uint8_t value)
{
	uint8_t region, remainder, p, q, t;
	uint8_t r, g, b;

	if (sat == 0) {
		r = value;
		g = value;
		b = value;
		return (rgb_color_t){.r = r, .g = g, .b = b};
	}

	region = hue / 43;
	remainder = (hue - (region * 43)) * 6;

	p = (value * (255 - sat)) >> 8;
	q = (value * (255 - ((sat * remainder) >> 8))) >> 8;
	t = (value * (255 - ((sat * (255 - remainder)) >> 8))) >> 8;

	switch (region) {
	case 0:
		r = value;
		g = t;
		b = p;
		break;
	case 1:
		r = q;
		g = value;
		b = p;
		break;
	case 2:
		r = p;
		g = value;
		b = t;
		break;
	case 3:
		r = p;
		g = q;
		b = value;
		break;
	case 4:
		r = t;
		g = p;
		b = value;
		break;
	default:
		r = value;
		g = p;
		b = q;
		break;
	}

	return (rgb_color_t){.r = r, .g = g, .b = b};
}

/* Convert hue to RGB */
rgb_color_t app_led_hue_to_rgb(uint8_t hue)
{
	uint8_t r, g, b;
	uint8_t h = hue / 43;
	uint8_t f = (hue - (h * 43)) * 3;

	switch (h) {
	case 0:
		r = 255;
		g = f;
		b = 0;
		break;
	case 1:
		r = 255 - f;
		g = 255;
		b = 0;
		break;
	case 2:
		r = 0;
		g = 255;
		b = f;
		break;
	case 3:
		r = 0;
		g = 255 - f;
		b = 255;
		break;
	case 4:
		r = f;
		g = 0;
		b = 255;
		break;
	default:
		r = 255;
		g = 0;
		b = 255 - f;
		break;
	}

	return (rgb_color_t){.r = r, .g = g, .b = b};
}

/* Blend two colors by a percentage */
void blend_color(rgb_color_t *c1, const rgb_color_t *c2, uint8_t blend)
{
	c1->r = (c1->r * blend + c2->r * (255 - blend)) / 255;
	c1->g = (c1->g * blend + c2->g * (255 - blend)) / 255;
	c1->b = (c1->b * blend + c2->b * (255 - blend)) / 255;
}

/* Fade a color to a target color by a step amount
 *
 * Intended to be called repeatly until target is reached */
void fade_color(rgb_color_t *c, const rgb_color_t *target, uint8_t step)
{
	if ((c->r + step) < target->r) {
		c->r += step;
	} else if ((c->r - step) > target->r) {
		c->r -= step;
	} else {
		c->r = target->r;
	}

	if ((c->g + step) < target->g) {
		c->g += step;
	} else if ((c->g - step) > target->g) {
		c->g -= step;
	} else {
		c->g = target->g;
	}

	if ((c->b + step) < target->b) {
		c->b += step;
	} else if ((c->b - step) > target->b) {
		c->b -= step;
	} else {
		c->b = target->b;
	}
}

/* Set the color of a single LED at position i */
int app_led_set_index(app_led_data_t *leds, uint16_t i, rgb_color_t c, k_timeout_t block)
{
	if (i >= leds->num_leds)
		return -EINVAL;

	if (k_mutex_lock(&leds->mutex, block) == 0) {
		// set the state colour
		leds->state[i].color = c;
		k_mutex_unlock(&leds->mutex);
	}

	return leds_set_pixel(leds, i, c, leds->global_brightness, block);
}

/* Set the color of all LEDs outside of a sequence/blink state; the fallback
 * color */
int app_led_set_global_color(app_led_data_t *leds, rgb_color_t c, k_timeout_t block)
{
	if (k_mutex_lock(&leds->mutex, block) == 0) {
		leds->global_color = c;
		k_mutex_unlock(&leds->mutex);
	}

	if (leds->mode == Manual) {
		return leds_set_pixels(leds, 0, leds->num_leds, c, leds->global_brightness, block);
	} else {
		return 0;
	}
}

/* Set the global brightness of all LEDs in any state */
int app_led_set_global_brightness(app_led_data_t *leds, uint8_t brightness, k_timeout_t block)
{
	if (k_mutex_lock(&leds->mutex, block) == 0) {
		leds->global_brightness = brightness;
		k_mutex_unlock(&leds->mutex);
	}

	if (leds->mode == Manual) {
		return leds_set_pixels(leds, 0, leds->num_leds, leds->global_color, brightness,
				       block);
	} else {
		return 0;
	}
}

/* Fade all LEDs to a target color by a step amount
 *
 * Call at timed interval to fade to black all not updated, for example.
 * */
void app_led_fade_color(app_led_data_t *leds, uint8_t step, rgb_color_t target, k_timeout_t block)
{
	rgb_color_t update;

	for (int i = 0; i < leds->num_leds; i++) {
		app_led_get_pixel_rgb(leds, i, &update);
		fade_color(&update, &target, step);
		leds->state[i].color = update;
		leds_set_pixel(leds, i, update, leds->global_brightness, block);
	}
}

/* Blend all LEDs to a target color by a percentage */
void app_led_blend(app_led_data_t *leds, rgb_color_t c, uint8_t blend, k_timeout_t block)
{
	rgb_color_t update;

	for (int i = 0; i < leds->num_leds; i++) {
		app_led_get_pixel_rgb(leds, i, &update);
		blend_color(&update, &c, blend);
		leds->state[i].color = update;
		leds_set_pixel(leds, i, update, leds->global_brightness, block);
	}
}

/* Set the LedMode of the App LED */
void app_led_set_mode(app_led_data_t *leds, LedMode mode, k_timeout_t block)
{
	// if not already in requested mode to avoid replacing last_mode
	if (leds->mode != mode) {
		if (k_mutex_lock(&leds->mutex, block) == 0) {
			leds->last_mode = leds->mode;
			leds->mode = mode;
			if (leds->mode == Rainbow) {
				leds->rainbow = true;
			}
			k_mutex_unlock(&leds->mutex);
		}
	}

	// act on change
	switch (mode) {
	case Manual:
		leds_set_pixels(leds, 0, leds->num_leds, leds->global_color,
				leds->global_brightness, block);
	case Off:
		IF_ENABLED(CONFIG_APP_LED_SUSPEND_TASK_MANUAL, (k_work_cancel_delayable(&leds->dwork);))
		break;
	default:
		IF_ENABLED(CONFIG_APP_LED_SUSPEND_TASK_MANUAL, (k_work_reschedule(&leds->dwork, K_NO_WAIT);))
		break;
	}
}

/* Return to last mode */
void app_led_last_mode(app_led_data_t *leds, k_timeout_t block)
{
	int64_t now = k_uptime_get();
	struct app_led_state *led;
	LedMode last = leds->last_mode;

	switch (leds->last_mode) {
	case Blink:
		// if all not blinking, return to manual/rainbow - below will override if
		// any blinking
		if (leds->rainbow) {
			last = Rainbow;
		} else {
			last = Manual;
		}

		// loop through, any blinking will set to return to blink
		for (int i = 0; i < leds->num_leds; i++) {
			led = &leds->state[i];
			if ((led->on_time_ms_left < now) && (led->off_time_ms_left < now)) {
				last = Blink;
			}
		}
		break;
	case Sequence:
		// if sequence finished, return to manual/rainbow
		if (leds->sequence == NULL) {
			if (leds->rainbow) {
				last = Rainbow;
			} else {
				last = Manual;
			}
		}
		break;
	default:
		last = leds->last_mode;
		break;
	}

	app_led_set_mode(leds, last, block);
}

int app_led_toggle_index_color(app_led_data_t *leds, uint16_t i, rgb_color_t c, k_timeout_t block)
{
	rgb_color_t current;

	if (i >= leds->num_leds)
		return -EINVAL;

	app_led_get_pixel_rgb(leds, i, &current);

	if (current.hex == 0x000000) {
		return app_led_set_index(leds, i, c, block);
	} else {
		return app_led_set_index(leds, i, RGBHEX(Black), block);
	}
}

int app_led_toggle_index(app_led_data_t *leds, uint16_t i, k_timeout_t block)
{
	return app_led_toggle_index_color(leds, i, leds->state[i].color, block);
}

int app_led_toggle_color(app_led_data_t *leds, rgb_color_t c, k_timeout_t block)
{
	if (leds->_color.hex == 0x000000) {
		return app_led_set_global_color(leds, c, block);
	} else {
		return app_led_set_global_color(leds, RGBHEX(Black), block);
	}
}

int app_led_toggle(app_led_data_t *leds, k_timeout_t block)
{
	return app_led_toggle_color(leds, leds->global_color, block);
}

// toggles mode between last_mode and Off so maintains state of blink/sequence
// between toggles
//
// use led_toggle.. to turn LED off in manual mode
void app_led_toggle_mode(app_led_data_t *leds, k_timeout_t block)
{
	if (leds->mode == Off) {
		app_led_last_mode(leds, block);
	} else {
		app_led_set_mode(leds, Off, block);
	}
}

int app_led_off_index(app_led_data_t *leds, uint16_t i, k_timeout_t block)
{
	return app_led_set_index(leds, 0, RGBHEX(Black), block);
}

// user turn off led by settings manual color to black
int app_led_off(app_led_data_t *leds, k_timeout_t block)
{
	return app_led_set_global_color(leds, RGBHEX(Black), block);
}

// blink led with on_period and off_period in ms. state_override flag will
// prevent blinking if in Off, Sequence or Error state
int app_led_blink_index(app_led_data_t *leds, uint16_t i, rgb_color_t c, uint32_t on_period_ms,
			uint32_t off_period_ms, bool state_override, k_timeout_t block)
{
	int64_t now = k_uptime_get();
	bool change_mode = false;
	struct app_led_state *led;

	if (i >= leds->num_leds)
		return -EINVAL;

	if (!state_override && (leds->mode == Sequence || leds->mode == Off || leds->mode == Error))
		return -EALREADY;

	if (k_mutex_lock(&leds->mutex, block) == 0) {
		led = &leds->state[i];

		if ((led->on_time_ms_left < now) && (led->off_time_ms_left < now)) {
			led->off_time_ms_left = now + on_period_ms + off_period_ms;
			led->on_time_ms_left = now + on_period_ms;
			led->color = c;
			change_mode = true;
		}
		k_mutex_unlock(&leds->mutex);
	}

	// change mode if required - new blink periods set
	if (change_mode)
		app_led_set_mode(leds, Blink, block);

	return 0;
}

int app_led_blink(app_led_data_t *leds, rgb_color_t c, uint32_t on_period_ms,
		  uint32_t off_period_ms, bool state_override, k_timeout_t block)
{
	int err = 0;

	for (int i = 0; i < leds->num_leds; i++) {
		err = app_led_blink_index(leds, i, c, on_period_ms, off_period_ms, state_override,
					  block);
		if (err != 0)
			break;
	}

	return err;
}

// used to sync blink when changing colour
int app_led_blink_sync_index(app_led_data_t *leds, uint16_t i, rgb_color_t c, k_timeout_t block)
{
	struct app_led_state *led;

	if (i >= leds->num_leds) {
		LOG_ERR("LED index out of range");
		return -EINVAL;
	}

	if (k_mutex_lock(&leds->mutex, block) == 0) {
		led = &leds->state[i];
		led->off_time_ms_left = 0;
		led->on_time_ms_left = 0;
		led->color = c;
		k_mutex_unlock(&leds->mutex);
	}

	// put back if was in blink mode
	if (leds->mode == Blink) {
		app_led_last_mode(leds, block);
	}

	return 0;
}

int app_led_blink_sync(app_led_data_t *leds, rgb_color_t c, k_timeout_t block)
{
	int err = 0;

	for (int i = 0; i < leds->num_leds; i++) {
		err = app_led_blink_sync_index(leds, i, c, block);
		if (err != 0)
			break;
	}

	return err;
}

int app_led_fade_to(app_led_data_t *leds, rgb_color_t c, uint8_t end_brightness,
		    uint32_t fade_time_ms, k_timeout_t block)
{
	app_led_fade_sequence[0].color = leds->global_color;
	app_led_fade_sequence[0].start_brightness = leds->global_brightness;
	app_led_fade_sequence[0].end_brightness = 0;
	app_led_fade_sequence[0].time_in_10ms = fade_time_ms / 10;
	app_led_fade_sequence[1].color = c;
	app_led_fade_sequence[1].start_brightness = end_brightness;
	app_led_fade_sequence[1].end_brightness = end_brightness;
	app_led_run_sequence(leds, app_led_fade_sequence, 0, block);
	// do this after starting sequence so gets set in background
	app_led_set_global_color(leds, c, block);
	app_led_set_global_brightness(leds, end_brightness, block);

	return 0;
}

int app_led_fade_off(app_led_data_t *leds, uint32_t fade_time_ms, k_timeout_t block)
{
	return app_led_fade_to(leds, leds->global_color, 0, fade_time_ms, block);
}

int app_led_fade_on(app_led_data_t *leds, uint32_t fade_time_ms, k_timeout_t block)
{
	return app_led_fade_to(leds, leds->global_color, 255, fade_time_ms, block);
}

/* Run sequence now; will switch to sequence state and replace any currently
 * running sequence so mode != Sequence should be checked if not wishing to
 * replace */
void app_led_run_sequence(app_led_data_t *leds, const app_led_sequence_step_t *sequence,
			  int8_t num_repeat, k_timeout_t block)
{
	if (k_mutex_lock(&leds->mutex, block) == 0) {
		leds->sequence = sequence;
		leds->sequence_repeat_count = num_repeat;
		leds->time_sequence_next = 0;
		k_mutex_unlock(&leds->mutex);
	}

	app_led_set_mode(leds, Sequence, block);
}

/* Clear sequence; will switch to last mode if sequence was running */
void app_led_sequence_clear(app_led_data_t *leds, k_timeout_t block)
{
	if (k_mutex_lock(&leds->mutex, block) == 0) {
		leds->sequence = NULL;
		leds->sequence_step = 0;
		leds->sequence_repeat_count = 0;
		leds->time_sequence_next = 0;
		k_mutex_unlock(&leds->mutex);
	}

	// put last mode back if sequence was running
	if (leds->mode == Sequence) {
		app_led_last_mode(leds, block);
	}
}

/* Wait for sequence/blink to finish */
void app_led_wait_inactive(app_led_data_t *leds, k_timeout_t timeout)
{
	k_timepoint_t end = sys_timepoint_calc(timeout);

	do {
		timeout = sys_timepoint_timeout(end);
		k_sleep(K_MSEC(CONFIG_APP_LED_UPDATE_PERIOD));
	} while (!K_TIMEOUT_EQ(timeout, K_NO_WAIT) &&
		 (leds->mode == Sequence || leds->mode == Blink));
}

/* Wait for sequence to finish */
void app_led_wait_sequence(app_led_data_t *leds, k_timeout_t timeout)
{
	k_timepoint_t end = sys_timepoint_calc(timeout);

	do {
		timeout = sys_timepoint_timeout(end);
		k_sleep(K_MSEC(CONFIG_APP_LED_UPDATE_PERIOD));
	} while (!K_TIMEOUT_EQ(timeout, K_NO_WAIT) && leds->mode == Sequence);
}

/* Wait for blink to finish */
void app_led_wait_blink(app_led_data_t *leds, k_timeout_t timeout)
{
	k_timepoint_t end = sys_timepoint_calc(timeout);

	do {
		timeout = sys_timepoint_timeout(end);
		k_sleep(K_MSEC(CONFIG_APP_LED_UPDATE_PERIOD));
	} while (!K_TIMEOUT_EQ(timeout, K_NO_WAIT) && leds->mode == Blink);
}

/* Sequence function callbacks */

/* Generic sequence function to set step color to global color etc. */
void app_led_seq_fnc(void *const pleds, const void *const pstep, k_timeout_t block)
{
	app_led_data_t *leds = (app_led_data_t *)pleds;
	// just update the color to match manual color
	leds->sequence_data.color = leds->global_color;
}

/* Fade in and out to global color with hold at top/bottom like breating */
void app_led_breathe(void *const pleds, const void *const pstep, k_timeout_t block)
{
	static bool toggle = false;
	app_led_data_t *leds = (app_led_data_t *)pleds;
	app_led_sequence_data_t *data = &leds->sequence_data;
	data->color = leds->global_color;

	if (data->count >= leds->global_brightness) {
		data->count = 10;
		toggle = !toggle;
	}

	leds_set_pixels(leds, 0, leds->num_leds, data->color,
			toggle ? (leds->global_brightness - data->count) : data->count, block);
	data->count *= 2;
}

void app_led_chase(void *const pleds, const void *const pstep, k_timeout_t block)
{
	app_led_data_t *leds = (app_led_data_t *)pleds;
	app_led_sequence_step_t *step = (app_led_sequence_step_t *)pstep;
	app_led_sequence_data_t *data = &leds->sequence_data;
	data->color = leds->global_color;

	app_led_fade_color(leds, 4, RGBHEX(Black), block);

	if (k_uptime_get() - data->last_tick >= (step->time_in_10ms * 10) / leds->num_leds) {
		if (data->count > leds->num_leds)
			data->count = 0;

		leds_set_pixel(leds, data->count++, data->color, leds->global_brightness, block);
		data->last_tick = k_uptime_get();
	}
}

void app_led_fade_blink(void *const pleds, const void *const pstep, k_timeout_t block)
{
	app_led_data_t *leds = (app_led_data_t *)pleds;
	app_led_sequence_step_t *step = (app_led_sequence_step_t *)pstep;
	app_led_sequence_data_t *data = &leds->sequence_data;
	data->color = leds->global_color;

	app_led_fade_color(leds, 16, RGBHEX(Black), block);

	if (k_uptime_get() - data->last_tick >= (step->time_in_10ms * 10) / 2) {
		leds_set_pixels(leds, 0, leds->num_leds, data->color, leds->global_brightness,
				block);
		data->last_tick = k_uptime_get();
	}
}

void app_led_half_blink(void *const pleds, const void *const pstep, k_timeout_t block)
{
	app_led_data_t *leds = (app_led_data_t *)pleds;
	app_led_sequence_step_t *step = (app_led_sequence_step_t *)pstep;
	app_led_sequence_data_t *data = &leds->sequence_data;
	data->color = leds->global_color;

	if (data->count) {
		app_led_fade_color(leds, 16, RGBHEX(Black), block);
		leds_set_pixels(leds, 0, leds->num_leds / 2, data->color, leds->global_brightness,
				block);
	}

	if (k_uptime_get() - data->last_tick >= (step->time_in_10ms * 10) / 2) {
		leds_set_pixels(leds, 0, leds->num_leds, data->color, leds->global_brightness,
				block);
		if (data->count == 0) {
			data->count = 1;
		} else {
			data->count = 0;
		}
		data->last_tick = k_uptime_get();
	}
}

void app_led_sine(void *const pleds, const void *const pstep, k_timeout_t block)
{
	static bool toggle = false;
	app_led_data_t *leds = (app_led_data_t *)pleds;
	app_led_sequence_data_t *data = &leds->sequence_data;
	data->color = leds->global_color;

	app_led_fade_color(leds, 6, RGBHEX(Black), block);

	if (k_uptime_get() - data->last_tick >= data->count * 10) {
		if (data->count > leds->num_leds) {
			data->count = 0;
			toggle = !toggle;
		}

		leds_set_pixel(leds, toggle ? (leds->num_leds - ++data->count) : data->count++,
			       data->color, leds->global_brightness, block);
		data->last_tick = k_uptime_get();
	}
}

static uint32_t app_led_show_sequence_step(app_led_data_t *leds, uint8_t step_num,
					   k_timeout_t block)
{
	// exit if no sequence obtained
	uint32_t ret = 0xFF * 10;

	if (k_mutex_lock(&leds->mutex, block) == 0) {
		if (leds->sequence != NULL) {
			// get the sequence step
			const app_led_sequence_step_t *step = &leds->sequence[step_num];

			// clamp to current global brightness
			leds->sequence_data.brightness =
				step->start_brightness > leds->global_brightness &&
						leds->global_brightness != 0
					? leds->global_brightness
					: step->start_brightness;
			// set the sequence step colour
			leds->sequence_data.color = step->color;

			// calculate fade step if not stop frame or start != end since our
			// starting brightness is clamped it might not equal start anymore
			if (step->time_in_10ms != 0xFF &&
			    (step->start_brightness != step->end_brightness)) {
				if (step->time_in_10ms > 1) {
					leds->sequence_data.fade_step =
						ceil((double)abs(leds->sequence_data.brightness -
								 step->end_brightness) /
						     step->time_in_10ms);
				} else {
					leds->sequence_data.fade_step = 255;
				}
			} else {
				leds->sequence_data.fade_step = 0;
			}

			// set the sequence step colour
			if (step->fnc != NULL) {
				step->fnc(leds, step, block);
			} else {
				leds_set_pixels(leds, 0, leds->num_leds, leds->sequence_data.color,
						leds->sequence_data.brightness, block);
			}
			ret = 10 * step->time_in_10ms;
		}

		k_mutex_unlock(&leds->mutex);
	}

	return ret;
}

static void app_led_update_sequence(app_led_data_t *leds, k_timeout_t block)
{
	int64_t now = k_uptime_get();

	// if ready for next step in sequence
	if (now > leds->time_sequence_next) {
		// get time of next
		uint32_t t = app_led_show_sequence_step(leds, leds->sequence_step, block);

		// check if exit flag
		if (t == 0xFF * 10) {
			// check if repeat - not equal to zero allows -1 to run forever
			if (leds->sequence_repeat_count != 0) {
				if (k_mutex_lock(&leds->mutex, block) == 0) {
					// decrement if > 0 so we exit after count
					if (leds->sequence_repeat_count > 0) {
						leds->sequence_repeat_count--;
					}
					// start again
					leds->sequence_step = 0;
					k_mutex_unlock(&leds->mutex);
				}
			} else {
				// sequence over so go back to last mode and return
				app_led_sequence_clear(leds, block);
			}
			// else set next timeout and increment for next step
		} else {
			if (k_mutex_lock(&leds->mutex, block) == 0) {
				++leds->sequence_step; // do this not inline so step will not
						       // overflow on last
				leds->time_sequence_next = now + t;
				k_mutex_unlock(&leds->mutex);
			}
		}
	} else if (leds->sequence_step > 0) {
		if (k_mutex_lock(&leds->mutex, block) == 0) {
			// -1 because sequence_step is incremented for next call; it's the next
			// step index
			const app_led_sequence_step_t *step =
				&leds->sequence[leds->sequence_step - 1];

			// poll fnc
			if (step->fnc != NULL) {
				step->fnc(leds, step, block);
			}

			// adjust brightness for sequence - could modulate color based on sequence
			// brightness and step
			if (leds->sequence_data.brightness != step->end_brightness &&
			    leds->sequence_data.fade_step != 0 && step->time_in_10ms != 0xFF) {
				if ((leds->sequence_data.brightness -
				     leds->sequence_data.fade_step) > step->end_brightness) {
					leds->sequence_data.brightness -=
						leds->sequence_data.fade_step;
				} else if ((leds->sequence_data.brightness +
					    leds->sequence_data.fade_step) < step->end_brightness) {
					leds->sequence_data.brightness +=
						leds->sequence_data.fade_step;
				} else {
					leds->sequence_data.brightness = step->end_brightness;
				}
				leds_set_pixels(leds, 0, leds->num_leds, leds->sequence_data.color,
						leds->sequence_data.brightness, block);
			}

			k_mutex_unlock(&leds->mutex);
		}
	}
}

static void app_led_update_blink_mode(app_led_data_t *leds, k_timeout_t block)
{
	int64_t now = k_uptime_get();
	bool change_mode = true;
	struct app_led_state *led;

	for (int i = 0; i < leds->num_leds; i++) {
		led = &leds->state[i];
		// turn on if within on period, else off (off_period is just used to blank
		// app_led_indicate_act)
		app_led_set_index(leds, i, now < led->on_time_ms_left ? led->color : RGBHEX(Black),
				  block);

		// don't exit mode until all leds have elapsed off period
		if (led->off_time_ms_left >= now)
			change_mode = false;
	}

	// go back to last mode once off period elasped for all
	if (change_mode)
		app_led_last_mode(leds, block);
}

/**
 * @brief Update App LED state machine
 *
 * This function is called from the LED task thread to update the LED state machine and set the
 * LEDs. If not using the LED task, this function can be called from a App thread to update the
 * LEDs at a regular interval.
 */
void app_led_update(app_led_data_t *leds)
{
	switch (leds->mode) {
	case Manual:
		// if manual mode, just set the colour - will be suspended if
		// CONFIG_APP_LED_SUSPEND_TASK_MANUAL is set otherwise update ensures LED strip
		// is updated even if disconnected
		leds_set_pixels(leds, 0, leds->num_leds, leds->global_color,
				leds->global_brightness, K_FOREVER);
		break;
	case Rainbow:
		leds->hue++;
		leds_set_pixels(leds, 0, leds->num_leds, app_led_hsv_to_rgb(leds->hue, 255, 255),
				leds->global_brightness, K_FOREVER);
		break;
	case Blink:
		app_led_update_blink_mode(leds, K_FOREVER);
		break;
	case Sequence:
		app_led_update_sequence(leds, K_FOREVER);
		break;
	case Error:
		leds_set_pixels(leds, 0, leds->num_leds, RGBHEX(Red), leds->global_brightness,
				K_FOREVER);
		break;
	case Off:
	default:
		leds_set_pixels(leds, 0, leds->num_leds, RGBHEX(Black), leds->global_brightness,
				K_FOREVER);
		break;
	}
}

#if IS_ENABLED(CONFIG_APP_LED_USE_WORKQUEUE)
static void app_led_work_handler(struct k_work *work)
{
    struct k_work_delayable *dwork = k_work_delayable_from_work(work);
    app_led_data_t *leds = CONTAINER_OF(dwork, app_led_data_t, dwork);
    int64_t last_update_time = k_uptime_get();

    app_led_update(leds);

#if IS_ENABLED(CONFIG_LED_STRIP)
    leds_strip_update(leds);
#endif

    if ((leds->mode != Manual && leds->mode != Off) || !IS_ENABLED(CONFIG_APP_LED_SUSPEND_TASK_MANUAL)) {
         // Calculate next deadline based on period and execution time
         k_timeout_t delay = K_MSEC(MAX(0, CONFIG_APP_LED_UPDATE_PERIOD - k_uptime_delta(&last_update_time)));
         k_work_reschedule(&leds->dwork, delay);
    }
}
#endif

/**
 * @brief Initialize the App LED module
 *
 * The peripheral initialization is done by the LED/LED_PWM/LED_STRIP driver
 * with a post kernel init hook. We just need to check ready and start the task thread if using.
 */
int app_led_init(app_led_data_t *const leds)
{
	if (!device_is_ready(leds->app_led)) {
		LOG_ERR("Device %s is not ready", leds->app_led->name);
		return -ENODEV;
	}

#if IS_ENABLED(CONFIG_APP_LED_USE_WORKQUEUE)
	/* Init delayed work and call first time to schedule the work */
	k_work_init_delayable(&leds->dwork, app_led_work_handler);
	app_led_work_handler((struct k_work*) &leds->dwork);
#endif

	LOG_INF("App LED %s initialized", leds->app_led->name);

	return 0;
}
