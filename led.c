// Some of this code is based on the led_pwm driver in Zephyr but modified to allow multi-threaded access
// and to allow for a sequence of colours to be displayed, RGB brightness etc.
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <errno.h>
#include <zephyr/sys/util.h>
#include <stdlib.h>
#include <math.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(app);

#include "led.h"

#if IS_ENABLED(CONFIG_WS2812_STRIP_SPI)
#include <zephyr/drivers/led_strip.h>
#define LED_NODE_ID		DT_ALIAS(led_strip)
#define STRIP_NUM_PIXELS	DT_PROP(DT_ALIAS(led_strip), chain_length)
#define NUM_LEDS STRIP_NUM_PIXELS
static struct led_rgb pixels[STRIP_NUM_PIXELS] = {0};

#elif IS_ENABLED(CONFIG_LED_PWM)

#include <zephyr/drivers/pwm.h>
#define LED_NODE_ID	 DT_COMPAT_GET_ANY_STATUS_OKAY(pwm_leds)
#define LED_LABEL(led_node_id) DT_PROP_OR(led_node_id, label, NULL),
const char *app_led_label[] = {
 DT_FOREACH_CHILD(LED_NODE_ID, LED_LABEL)
};
#define NUM_LEDS ARRAY_SIZE(app_led_label)
#if LED_NODE_ID > 0
#error "No pwm_leds node found. Check your DTS."

#endif

#elif IS_ENABLED(CONFIG_LED)

#include <zephyr/drivers/gpio.h>
#define LED_NODE_ID	 DT_COMPAT_GET_ANY_STATUS_OKAY(gpio_leds)
#define LED_LABEL(led_node_id) DT_PROP_OR(led_node_id, label, NULL),
const char *app_led_label[] = {
 DT_FOREACH_CHILD(LED_NODE_ID, LED_LABEL)
};
#define NUM_LEDS ARRAY_SIZE(app_led_label)
#if LED_NODE_ID > 0
#error "No gpio_leds node found. Check your DTS."
#endif

#else
#error "CONFIG_WS2812_STRIP_SPI, CONFIG_LED or CONFIG_LED_PWM must be set"
#endif

static struct app_led_state rgb_state[NUM_LEDS] = {
  [0 ... NUM_LEDS - 1] = {
    .color = { .hex = 0x000000 },
    ._color = { .hex = 0x000000 },
    .on_time_ms_left = 0,
    .off_time_ms_left = 0,
  }
};

app_led_data_t rgbled = {
  .mode = Manual,
  .last_mode = Manual,
  .mutex = Z_MUTEX_INITIALIZER(rgbled.mutex),
  .app_led = DEVICE_DT_GET(LED_NODE_ID),
  .num_leds = NUM_LEDS,
  .global_brightness = 0xFF,
  .global_color = {.r = 0, .g = 0, .b = 0 },
  ._color = {.r = 0, .g = 0, .b = 0 },
  .hue = 0,
  .rainbow = false,
  .state = rgb_state,
  .sequence_step = 0,
  .sequence = NULL,
  .time_sequence_next = 0,
  .sequence_repeat_count = 0,
  .sequence_data = { .count = 0, .last_tick = 0 },
};

#define PRIORITY  k_thread_priority_get(k_current_get()) + 3
#define STACKSIZE 1024

static k_tid_t app_led_task_tid;
static K_THREAD_STACK_DEFINE(app_led_task_stack, STACKSIZE);
static struct k_thread app_led_task_thread;

#if IS_ENABLED(CONFIG_WS2812_STRIP_SPI)
/* Work queue for updating the LED strip, so app_* API can be called from ISR */
static void leds_work(struct k_work *work) {
  if (k_mutex_lock(&rgbled.mutex, K_FOREVER) == 0) {
    if (led_strip_update_rgb(rgbled.app_led, pixels, STRIP_NUM_PIXELS) != 0) {
      LOG_ERR("Couldn't update strip");
    }

    k_mutex_unlock(&rgbled.mutex);
  }
}

K_WORK_DEFINE(app_led_work, leds_work);

/* Set the device level LED data and update app_led
 *
 * Zephyr LED strip uses a different RGB struct to the app_led struct so convert to that. Work is submitted to update the LED strip
 * because the LED strip driver is not ISR safe.
 * */
static int _app_led_set_pixels(app_led_data_t *leds, uint16_t start, uint16_t end, rgb_color_t c, uint8_t brightness, k_timeout_t block) {
  struct led_rgb c_rgb;

  if (start < leds->num_leds && end <= leds->num_leds) {
    c_rgb = (struct led_rgb) {
      .r = (uint16_t) c.r * brightness / 255,
      .g = (uint16_t) c.g * brightness / 255,
      .b = (uint16_t) c.b * brightness / 255,
    };

    leds->_color = c;

    for (int i = start; i < end; i++) {
      memcpy(&pixels[i], &c_rgb, sizeof(struct led_rgb));
      leds->state[i]._color = c;
    }

    k_work_submit(&app_led_work);
    return 0;
  } else {
    LOG_ERR("LED index out of range");
    return -EINVAL;
  }
}
#elif IS_ENABLED(CONFIG_LED_PWM)
static int _app_led_set_pwm_brightness(app_led_data_t *leds, uint8_t i, uint8_t value, k_timeout_t block) {
  const struct app_led_pwm_config *config = leds->app_led->config;
  const struct pwm_dt_spec *dt_led;
  int err;

  if (i >= config->num_leds || value > 255) {
    return -EINVAL;
  }

  dt_led = &config->led[i];

  if (k_mutex_lock(&leds->mutex, block) == 0) {
    err = pwm_set_pulse_dt(&config->led[i],
        dt_led->period * value / 255);
    k_mutex_unlock(&leds->mutex);
    return err;
  } else {
    return -EBUSY;
  }
}
#else
static int _app_led_set_gpio_brightness(app_led_data_t *leds, uint8_t i, uint8_t value, k_timeout_t block) {
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

#if (IS_ENABLED(CONFIG_LED_PWM) || IS_ENABLED(CONFIG_LED)) && !IS_ENABLED(CONFIG_WS2812_STRIP_SPI)
static inline int _app_led_set_brightness(app_led_data_t *leds, uint8_t i, uint8_t value, k_timeout_t block) {
#if IS_ENABLED(CONFIG_LED_PWM)
    return _app_led_set_pwm_brightness(leds, i, value, block);
#else
    return _app_led_set_gpio_brightness(leds, i, value, block);
#endif
}

static int _app_led_set_pixels(app_led_data_t *leds, uint16_t start, uint16_t end, rgb_color_t c, uint8_t brightness, k_timeout_t block) {
  int err;

  // TODO CONFIG for using passed color as single LED value for chains of GPIO LEDs, eg r + g + b > 127 == on, else off
  // would not multiply by 3 this in this case
  // * 3 for RGB LED
  end *= 3;
  if (start < leds->num_leds && end <= leds->num_leds) {
    // scale
    c.r = (uint16_t) c.r * brightness / 255;
    c.g = (uint16_t) c.g * brightness / 255;
    c.b = (uint16_t) c.b * brightness / 255;

    leds->_color = c;

    for (int i = start; i < end; i += 3) {
      leds->state[i]._color = c;
      err = _app_led_set_brightness(leds, i, c.r, block);
      err = _app_led_set_brightness(leds, i+1, c.g, block);
      err = _app_led_set_brightness(leds, i+2, c.b, block);

      if (err != 0) {
        return err;
      }
    }
  }

  return 0;
}
#endif

/* Set a single pixel on the LED strip */
static inline int _app_led_set_pixel(app_led_data_t *leds, uint16_t i, rgb_color_t c, uint8_t brightness, k_timeout_t block) {
  return _app_led_set_pixels(leds, i, i + 1, c, brightness, block);
}

/* Get the current set RGB value of a pixel as a rgb_color_t */
int app_led_get_pixel_rgb(const app_led_data_t *const leds, uint16_t i, rgb_color_t *c) {
  if (i < leds->num_leds) {
    *c = leds->state[i]._color;
    return 0;
  } else {
    return -EINVAL;
  }
}

/* Convert HSV to RGB */
rgb_color_t app_led_hsv_to_rgb(uint8_t hue, uint8_t sat , uint8_t value) {
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
rgb_color_t app_led_hue_to_rgb(uint8_t hue) {
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
void blend_color(rgb_color_t *c1, const rgb_color_t *c2, uint8_t blend) {
  c1->r = (c1->r * blend + c2->r * (255 - blend)) / 255;
  c1->g = (c1->g * blend + c2->g * (255 - blend)) / 255;
  c1->b = (c1->b * blend + c2->b * (255 - blend)) / 255;
}

/* Fade a color to a target color by a step amount
 *
 * Intended to be called repeatly until target is reached */
void fade_color(rgb_color_t *c, const rgb_color_t *target, uint8_t step) {
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
int app_led_set_index(app_led_data_t *leds, uint16_t i, rgb_color_t c, k_timeout_t block) {
  if (i >= leds->num_leds)
    return -EINVAL;

  if (k_mutex_lock(&leds->mutex, block) == 0) {
    // set the state colour
    leds->state[i].color = c;
    k_mutex_unlock(&leds->mutex);
  }

  return _app_led_set_pixel(leds, i, c, leds->global_brightness, block);
}

/* Set the color of all LEDs outside of a sequence/blink state; the fallback color */
int app_led_set_global_color(app_led_data_t *leds, rgb_color_t c, k_timeout_t block) {
  if (k_mutex_lock(&leds->mutex, block) == 0) {
    leds->global_color = c;
    k_mutex_unlock(&leds->mutex);
  }

  if (leds->mode == Manual) {
    return _app_led_set_pixels(leds, 0, leds->num_leds, c, leds->global_brightness, block);
  } else {
    return 0;
  }
}

/* Set the global brightness of all LEDs in any state */
int app_led_set_global_brightness(app_led_data_t *leds, uint8_t brightness, k_timeout_t block) {
  if (k_mutex_lock(&leds->mutex, block) == 0) {
    leds->global_brightness = brightness;
    k_mutex_unlock(&leds->mutex);
  }

  if (leds->mode == Manual) {
    return _app_led_set_pixels(leds, 0, leds->num_leds, leds->global_color, brightness, block);
  } else {
    return 0;
  }
}

/* Fade all LEDs to a target color by a step amount 
 *
 * Call at timed interval to fade to black all not updated, for example.
 * */
void app_led_fade_color(app_led_data_t *leds, uint8_t step, rgb_color_t target, k_timeout_t block) {
  rgb_color_t update;

  for (int i = 0; i < leds->num_leds; i++) {
    app_led_get_pixel_rgb(leds, i, &update);
    fade_color(&update, &target, step);
    leds->state[i].color = update;
    _app_led_set_pixel(leds, i, update, leds->global_brightness, block);
  }
}

/* Blend all LEDs to a target color by a percentage */
void app_led_blend(app_led_data_t *leds, rgb_color_t c, uint8_t blend, k_timeout_t block) {
  rgb_color_t update;

  for (int i = 0; i < leds->num_leds; i++) {
    app_led_get_pixel_rgb(leds, i, &update);
    blend_color(&update, &c, blend);
    leds->state[i].color = update;
    _app_led_set_pixel(leds, i, update, leds->global_brightness, block);
  }
}

/* Set the LedMode of the App LED */
void app_led_set_mode(app_led_data_t *leds, LedMode mode, k_timeout_t block) {
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
      _app_led_set_pixels(leds, 0, leds->num_leds, leds->global_color, leds->global_brightness, block);
      if (IS_ENABLED(CONFIG_LED_SUSPEND_TASK_MANUAL))
        k_thread_suspend(app_led_task_tid);
      break;
    default:
      k_thread_resume(app_led_task_tid);
      break;
  }
}

/* Return to last mode */
void app_led_last_mode(app_led_data_t *leds, k_timeout_t block) {
  uint32_t now = k_uptime_get();
  struct app_led_state *led;
  LedMode last = leds->last_mode;

  switch (leds->last_mode) {
    case Blink:
      // if all not blinking, return to manual/rainbow - below will override if any blinking
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

int app_led_toggle_index_color(app_led_data_t *leds, uint16_t i, rgb_color_t c, k_timeout_t block) {
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

int app_led_toggle_index(app_led_data_t *leds, uint16_t i, k_timeout_t block) {
  return app_led_toggle_index_color(leds, i, leds->state[i].color, block);
}

int app_led_toggle_color(app_led_data_t *leds, rgb_color_t c, k_timeout_t block) {
  if (leds->_color.hex == 0x000000) {
    return app_led_set_global_color(leds, c, block);
  } else {
    return app_led_set_global_color(leds, RGBHEX(Black), block);
  }
}

int app_led_toggle(app_led_data_t *leds, k_timeout_t block) {
  return app_led_toggle_color(leds, leds->global_color, block);
}

// toggles mode between last_mode and Off so maintains state of blink/sequence
// between toggles
//
// use led_toggle.. to turn LED off in manual mode
void app_led_toggle_mode(app_led_data_t *leds, k_timeout_t block) {
  if (leds->mode == Off) {
    app_led_last_mode(leds, block);
  } else {
    app_led_set_mode(leds, Off, block);
  }
}

int app_led_off_index(app_led_data_t *leds, uint16_t i, k_timeout_t block) {
  return app_led_set_index(leds, 0, RGBHEX(Black), block);
}

// user turn off led by settings manual color to black
int app_led_off(app_led_data_t *leds, k_timeout_t block) {
  return app_led_set_global_color(leds, RGBHEX(Black), block);
}

// blink led with on_period and off_period in ms. state_override flag will prevent blinking if in Off, Sequence or Error state
int app_led_blink_index(app_led_data_t *leds, uint16_t i, rgb_color_t c, uint32_t on_period_ms, uint32_t off_period_ms, bool state_override, k_timeout_t block) {
  uint32_t now = k_uptime_get();
  bool change_mode = false;
  struct app_led_state *led;

  if (i >= leds->num_leds)
    return -EINVAL;

  if (!state_override &&
      (leds->mode == Sequence || leds->mode == Off || leds->mode == Error))
    return -EALREADY;

  if (k_mutex_lock(&leds->mutex, block) == 0) {
    led = &leds->state[i];

    if ( (led->on_time_ms_left < now) && (led->off_time_ms_left < now) ) {
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

int app_led_blink(app_led_data_t *leds, rgb_color_t c, uint32_t on_period_ms, uint32_t off_period_ms, bool state_override, k_timeout_t block) {
  int err = 0;

  for (int i = 0; i < leds->num_leds; i++) {
    err = app_led_blink_index(leds, i, c, on_period_ms, off_period_ms, state_override, block);
    if (err != 0)
      break;
  }

  return err;
}

// used to sync blink when changing colour
int app_led_blink_sync_index(app_led_data_t *leds, uint16_t i, rgb_color_t c, k_timeout_t block) {
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

int app_led_blink_sync(app_led_data_t *leds, rgb_color_t c, k_timeout_t block) {
  int err = 0;

  for (int i = 0; i < leds->num_leds; i++) {
    err = app_led_blink_sync_index(leds, i, c, block);
    if (err != 0)
      break;
  }

  return err;
}

/* Run sequence now; will switch to sequence state and replace any currently running sequence so mode != Sequence should be checked if not wishing to replace */
void app_led_run_sequence(app_led_data_t *leds, const app_led_sequence_step_t *sequence, int8_t num_repeat, k_timeout_t block) {
  if (k_mutex_lock(&leds->mutex, block) == 0) {
    leds->sequence = sequence;
    leds->sequence_repeat_count = num_repeat;
    leds->time_sequence_next = 0;
    k_mutex_unlock(&leds->mutex);
  }

  app_led_set_mode(leds, Sequence, block);
}

/* Clear sequence; will switch to last mode if sequence was running */
void app_led_sequence_clear(app_led_data_t *leds, k_timeout_t block) {
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
void app_led_wait_inactive(app_led_data_t *leds, k_timeout_t wait_ms) {
  int64_t entry = k_uptime_get();

  // block for wait_ms or forever if 0 for sequence mode to exit (sequence finished)
  while((k_uptime_get() - entry < wait_ms.ticks || wait_ms.ticks == 0)
      && (leds->mode == Sequence || leds->mode == Blink))
    ;;
}

/* Wait for sequence to finish */
void app_led_wait_sequence(app_led_data_t *leds, k_timeout_t wait_ms) {
  int64_t entry = k_uptime_get();

  // block for wait_ms or forever if 0 for sequence mode to exit (sequence finished)
  while((k_uptime_get() - entry < wait_ms.ticks || wait_ms.ticks == 0)
      && leds->mode == Sequence)
    ;;
}

/* Wait for blink to finish */
void app_led_wait_blink(app_led_data_t *leds, k_timeout_t wait_ms) {
  int64_t entry = k_uptime_get();

  // block for wait_ms or forever if 0 for sequence mode to exit (sequence finished)
  while((k_uptime_get() - entry < wait_ms.ticks || wait_ms.ticks == 0)
      && leds->mode == Blink)
    ;;
}

/* Sequence function callbacks */

/* Generic sequence function to set step color to global color etc. */
void app_led_seq_fnc(void *const pleds, const void *const pstep, k_timeout_t block) {
  app_led_data_t *leds = (app_led_data_t *) pleds;
  // just update the color to match manual color
  leds->sequence_data.color = leds->global_color;
}

/* Fade in and out to global color with hold at top/bottom like breating */
void app_led_breathe(void *const pleds, const void *const pstep, k_timeout_t block) {
  static bool toggle = false;
  app_led_data_t *leds = (app_led_data_t *) pleds;
  app_led_sequence_data_t *data = &leds->sequence_data;
  data->color = leds->global_color;

  if (data->count >= leds->global_brightness) {
    data->count = 10;
    toggle = !toggle;
  }

  _app_led_set_pixels(leds, 0, leds->num_leds, data->color, toggle ? (leds->global_brightness - data->count) : data->count, block);
  data->count *= 2;
}

void app_led_chase(void *const pleds, const void *const pstep, k_timeout_t block) {
  app_led_data_t *leds = (app_led_data_t *) pleds;
  app_led_sequence_step_t *step = (app_led_sequence_step_t *) pstep;
  app_led_sequence_data_t *data = &leds->sequence_data;
  data->color = leds->global_color;

  app_led_fade_color(leds, 4, RGBHEX(Black), block);

  if (k_uptime_get() - data->last_tick >= (step->time_in_10ms * 10) / leds->num_leds) {
    if (data->count > leds->num_leds)
      data->count = 0;

    _app_led_set_pixel(leds, data->count++, data->color, leds->global_brightness, block);
    data->last_tick = k_uptime_get();
  }
}

void app_led_fade_blink(void *const pleds, const void *const pstep, k_timeout_t block) {
  app_led_data_t *leds = (app_led_data_t *) pleds;
  app_led_sequence_step_t *step = (app_led_sequence_step_t *) pstep;
  app_led_sequence_data_t *data = &leds->sequence_data;
  data->color = leds->global_color;

  app_led_fade_color(leds, 16, RGBHEX(Black), block);

  if (k_uptime_get() - data->last_tick >= (step->time_in_10ms * 10) / 2) {
    _app_led_set_pixels(leds, 0, leds->num_leds, data->color, leds->global_brightness, block);
    data->last_tick = k_uptime_get();
  }
}


void app_led_half_blink(void *const pleds, const void *const pstep, k_timeout_t block) {
  app_led_data_t *leds = (app_led_data_t *) pleds;
  app_led_sequence_step_t *step = (app_led_sequence_step_t *) pstep;
  app_led_sequence_data_t *data = &leds->sequence_data;
  data->color = leds->global_color;

  if (data->count) {
    app_led_fade_color(leds, 16, RGBHEX(Black), block);
    _app_led_set_pixels(leds, 0, leds->num_leds / 2, data->color, leds->global_brightness, block);
  }

  if (k_uptime_get() - data->last_tick >= (step->time_in_10ms * 10) / 2) {
    _app_led_set_pixels(leds, 0, leds->num_leds, data->color, leds->global_brightness, block);
    if (data->count == 0) {
      data->count = 1;
    } else {
      data->count = 0;
    }
    data->last_tick = k_uptime_get();
  }
}

void app_led_sine(void *const pleds, const void *const pstep, k_timeout_t block) {
  static bool toggle = false;
  app_led_data_t *leds = (app_led_data_t *) pleds;
  app_led_sequence_data_t *data = &leds->sequence_data;
  data->color = leds->global_color;

  app_led_fade_color(leds, 6, RGBHEX(Black), block);

  if (k_uptime_get() - data->last_tick >= data->count * 10) {
    if (data->count > leds->num_leds) {
      data->count = 0;
      toggle = !toggle;
    }

    _app_led_set_pixel(leds, toggle ? (leds->num_leds - ++data->count) : data->count++, data->color, leds->global_brightness, block);
    data->last_tick = k_uptime_get();
  }
}

static uint32_t app_led_show_sequence_step(app_led_data_t *leds, uint8_t step_num, k_timeout_t block) {
  // exit if no sequence obtained
  uint32_t ret = 0xFF * 10;

  if (k_mutex_lock(&leds->mutex, block) == 0) {
    if (leds->sequence != NULL) {
      // get the sequence step
      const app_led_sequence_step_t *step = &leds->sequence[step_num];

      // clamp to current global brightness
      leds->sequence_data.brightness = step->start_brightness > leds->global_brightness && leds->global_brightness != 0 ? leds->global_brightness : step->start_brightness;
      // set the sequence step colour
      leds->sequence_data.color = step->color;

      // calculate fade step if not stop frame or start != end since our starting brightness is clamped it might not equal start anymore
      if (step->time_in_10ms != 0xFF && (step->start_brightness != step->end_brightness)) {
        if (step->time_in_10ms > 1) {
          leds->sequence_data.fade_step = ceil((double) abs(leds->sequence_data.brightness - step->end_brightness) / step->time_in_10ms);
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
        _app_led_set_pixels(leds, 0, leds->num_leds, leds->sequence_data.color, leds->sequence_data.brightness, block);
      }
      ret = 10 * step->time_in_10ms;
    }

    k_mutex_unlock(&leds->mutex);
  }

  return ret;
}

static void app_led_update_sequence(app_led_data_t *leds, k_timeout_t block) {
  uint32_t now = k_uptime_get();

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
        ++leds->sequence_step; // do this not inline so step will not overflow on last
        leds->time_sequence_next = now + t;
        k_mutex_unlock(&leds->mutex);
      }
    }
  } else if (leds->sequence_step > 0) {
    if (k_mutex_lock(&leds->mutex, block) == 0) {
      // -1 because sequence_step is incremented for next call; it's the next step index
      const app_led_sequence_step_t *step = &leds->sequence[leds->sequence_step - 1];

      // poll fnc
      if (step->fnc != NULL) {
        step->fnc(leds, step, block);
      }

      // adjust brightness for sequence - could modulate color based on sequence brightness and step
      if (leds->sequence_data.brightness != step->end_brightness
          && leds->sequence_data.fade_step != 0
          && step->time_in_10ms != 0xFF) {
        if ((leds->sequence_data.brightness - leds->sequence_data.fade_step) > step->end_brightness) {
          leds->sequence_data.brightness -= leds->sequence_data.fade_step;
        } else if ((leds->sequence_data.brightness + leds->sequence_data.fade_step) < step->end_brightness) {
          leds->sequence_data.brightness += leds->sequence_data.fade_step;
        } else {
          leds->sequence_data.brightness = step->end_brightness;
        }
        _app_led_set_pixels(leds, 0, leds->num_leds, leds->sequence_data.color, leds->sequence_data.brightness, block);
      }

      k_mutex_unlock(&leds->mutex);
    }
  }
}

static void app_led_update_blink_mode(app_led_data_t *leds, k_timeout_t block) {
  uint32_t now = k_uptime_get();
  bool change_mode = true;
  struct app_led_state *led;

  for (int i = 0; i < leds->num_leds; i++) {
    led = &leds->state[i];
    // turn on if within on period, else off (off_period is just used to blank app_led_indicate_act)
    app_led_set_index(leds, i, now < led->on_time_ms_left ? led->color : RGBHEX(Black), block);

    // don't exit mode until all leds have elapsed off period
    if (led->off_time_ms_left >= now)
      change_mode = false;
  }

  // go back to last mode once off period elasped for all
  if (change_mode)
    app_led_last_mode(leds, block);
}

// should be called at least once per 1 ms (systick)
void leds_update(app_led_data_t *leds) {
  switch (leds->mode) {
    case Manual:
      // if manual mode, just set the colour - will be suspended if CONFIG_LED_SUSPEND_TASK_MANUAL is set otherwise update ensures LED strip is updated even if disconnected
      _app_led_set_pixels(leds, 0, leds->num_leds, leds->global_color, leds->global_brightness, K_FOREVER);
      break;
    case Rainbow:
      leds->hue++;
      _app_led_set_pixels(leds, 0, leds->num_leds, app_led_hsv_to_rgb(leds->hue, 255, 255), leds->global_brightness, K_FOREVER);
      break;
    case Blink:
      app_led_update_blink_mode(leds, K_FOREVER);
      break;
    case Sequence:
      app_led_update_sequence(leds, K_FOREVER);
      break;
    case Error:
      _app_led_set_pixels(leds, 0, leds->num_leds, RGBHEX(Red), leds->global_brightness, K_FOREVER);
      break;
    case Off:
    default:
      _app_led_set_pixels(leds, 0, leds->num_leds, RGBHEX(Black), leds->global_brightness, K_FOREVER);
      break;
  }
}

void app_led_task_worker(void *p1, void *p2, void *p3) {
  int last = k_uptime_get();

  // this whole led system would probably be better as a worker with queue of
  // LED commands to process, app_led_sequence_step_t for example but it sort of
  // works...
  while (1) {
    last = k_uptime_get();
    leds_update(&rgbled);
    k_sleep(K_MSEC(10 - (k_uptime_get() - last)));
  }
}

// The peripheral initialization is done by the LED/LED_PWM driver
// with a post kernel init hook. We just need to start the task.
int app_led_init(void) {
  if (!device_is_ready(rgbled.app_led)) {
    LOG_ERR("Device %s is not ready", rgbled.app_led->name);
    return -ENODEV;
  }

  app_led_task_tid = k_thread_create(&app_led_task_thread, app_led_task_stack,
      K_THREAD_STACK_SIZEOF(app_led_task_stack),
      app_led_task_worker,
      NULL, NULL, NULL,
      PRIORITY, 0, K_NO_WAIT);

  LOG_INF("LED task started");

  return 0;
}
