#include <zephyr/kernel.h>

#include <app_led/led.h>

const app_led_sequence_step_t app_led_test_sequence[] = {
	{.fnc = NULL,
	 .color = RGBHEX(Red),
	 .time_in_10ms = 20,
	 .start_brightness = 0xFF,
	 .end_brightness = 0xFF},
	{.fnc = NULL,
	 .color = RGBHEX(Green),
	 .time_in_10ms = 20,
	 .start_brightness = 0xFF,
	 .end_brightness = 0xFF},
	{.fnc = NULL,
	 .color = RGBHEX(Blue),
	 .time_in_10ms = 20,
	 .start_brightness = 0xFF,
	 .end_brightness = 0xFF},
	{.fnc = NULL,
	 .color = RGBHEX(Black),
	 .time_in_10ms = 10,
	 .start_brightness = 0xFF,
	 .end_brightness = 0xFF},
	{.fnc = NULL,
	 .color = RGBHEX(Black),
	 .time_in_10ms = 0xFF,
	 .start_brightness = 0xFF,
	 .end_brightness = 0xFF},
};

const app_led_sequence_step_t app_led_error_sequence[] = {
	{.fnc = NULL,
	 .color = RGBHEX(DarkRed),
	 .time_in_10ms = 30,
	 .start_brightness = 0xFF,
	 .end_brightness = 0xFF},
	{.fnc = NULL,
	 .color = RGBHEX(Black),
	 .time_in_10ms = 10,
	 .start_brightness = 0xFF,
	 .end_brightness = 0xFF},
	{.fnc = NULL,
	 .color = RGBHEX(Black),
	 .time_in_10ms = 0xFF,
	 .start_brightness = 0xFF,
	 .end_brightness = 0xFF},
};

// run multiple times to create blanking sequence
const app_led_sequence_step_t app_led_blank_sequence[] = {
	{
		.fnc = NULL,
		.color = RGBHEX(Black),
		.time_in_10ms = 1,
		.start_brightness = 0xFF,
	},
	{.fnc = NULL, .color = RGBHEX(Black), .time_in_10ms = 0xFF},
};

app_led_sequence_step_t app_led_charging_sequence[] = {
	{.fnc = NULL,
	 .color = RGBHEX(Green),
	 .time_in_10ms = 250,
	 .start_brightness = 25,
	 .end_brightness = 255},
	{.fnc = NULL,
	 .color = RGBHEX(Green),
	 .time_in_10ms = 50,
	 .start_brightness = 255,
	 .end_brightness = 25},
	{.fnc = NULL, .color = RGBHEX(Green), .time_in_10ms = 0xFF},
};

// Sequence to do boot fade
// both fade and breathe call the same function, to update the color based on
// current global value
app_led_sequence_step_t app_led_fade_sequence[] = {
	{.fnc = &app_led_seq_fnc,
	 .color = RGBHEX(Black),
	 .time_in_10ms = 150,
	 .start_brightness = 0x0,
	 .end_brightness = 0xFF},
	{.fnc = &app_led_seq_fnc,
	 .color = RGBHEX(Black),
	 .time_in_10ms = 0xFF,
	 .start_brightness = 0xFF,
	 .end_brightness = 0xFF},
};

const app_led_sequence_step_t app_led_breathe_sequence[] = {
	{.fnc = &app_led_seq_fnc,
	 .color = RGBHEX(Black),
	 .time_in_10ms = 250,
	 .start_brightness = 4,
	 .end_brightness = 127},
	{.fnc = &app_led_seq_fnc,
	 .color = RGBHEX(Black),
	 .time_in_10ms = 25,
	 .start_brightness = 127,
	 .end_brightness = 127},
	{.fnc = &app_led_seq_fnc,
	 .color = RGBHEX(Black),
	 .time_in_10ms = 250,
	 .start_brightness = 127,
	 .end_brightness = 4},
	{.fnc = &app_led_seq_fnc,
	 .color = RGBHEX(Black),
	 .time_in_10ms = 25,
	 .start_brightness = 4,
	 .end_brightness = 4},
	{.fnc = &app_led_seq_fnc, .color = RGBHEX(Black), .time_in_10ms = 0xFF},
};

const app_led_sequence_step_t app_led_bike_blink_sequence[] = {
	{.fnc = &app_led_seq_fnc,
	 .color = RGBHEX(White),
	 .time_in_10ms = 40,
	 .start_brightness = 0xFF,
	 .end_brightness = 0xFF},
	{.fnc = NULL,
	 .color = RGBHEX(Black),
	 .time_in_10ms = 10,
	 .start_brightness = 0xFF,
	 .end_brightness = 0xFF},
	{.fnc = NULL,
	 .color = RGBHEX(Black),
	 .time_in_10ms = 0xFF,
	 .start_brightness = 0xFF,
	 .end_brightness = 0xFF},
};

const app_led_sequence_step_t app_led_chase_sequence[] = {
	{.fnc = &app_led_chase,
	 .color = RGBHEX(Red),
	 .time_in_10ms = 40,
	 .start_brightness = 0xFF,
	 .end_brightness = 0xFF},
	{.fnc = &app_led_chase,
	 .color = RGBHEX(Black),
	 .time_in_10ms = 0xFF,
	 .start_brightness = 0xFF,
	 .end_brightness = 0xFF},
};

const app_led_sequence_step_t app_led_half_blink_sequence[] = {
	{.fnc = &app_led_half_blink,
	 .color = RGBHEX(Red),
	 .time_in_10ms = 40,
	 .start_brightness = 0xFF,
	 .end_brightness = 0xFF},
	{.fnc = &app_led_half_blink,
	 .color = RGBHEX(Black),
	 .time_in_10ms = 0xFF,
	 .start_brightness = 0xFF,
	 .end_brightness = 0xFF},
};

const app_led_sequence_step_t app_led_fade_blink_sequence[] = {
	{.fnc = &app_led_fade_blink,
	 .color = RGBHEX(Red),
	 .time_in_10ms = 60,
	 .start_brightness = 0xFF,
	 .end_brightness = 0xFF},
	{.fnc = &app_led_fade_blink,
	 .color = RGBHEX(Black),
	 .time_in_10ms = 0xFF,
	 .start_brightness = 0xFF,
	 .end_brightness = 0xFF},
};

const app_led_sequence_step_t app_led_sine_sequence[] = {
	{.fnc = &app_led_sine,
	 .color = RGBHEX(Red),
	 .time_in_10ms = 40,
	 .start_brightness = 0xFF,
	 .end_brightness = 0xFF},
	{.fnc = &app_led_sine,
	 .color = RGBHEX(Black),
	 .time_in_10ms = 0xFF,
	 .start_brightness = 0xFF,
	 .end_brightness = 0xFF},
};

// table of pointers to sequences
const app_led_sequence_step_t *app_led_sequences[] = {
	app_led_test_sequence,	     app_led_error_sequence, app_led_blank_sequence,
	app_led_charging_sequence,
	app_led_fade_sequence, // on
	app_led_fade_sequence, // off
	app_led_bike_blink_sequence, app_led_chase_sequence, app_led_fade_blink_sequence,
	app_led_half_blink_sequence, app_led_sine_sequence,  app_led_breathe_sequence,
};

// table of sequence lengths
const uint8_t app_led_sequence_lengths[] = {
	ARRAY_SIZE(app_led_test_sequence),	 ARRAY_SIZE(app_led_error_sequence),
	ARRAY_SIZE(app_led_blank_sequence),	 ARRAY_SIZE(app_led_charging_sequence),
	ARRAY_SIZE(app_led_fade_sequence),	 ARRAY_SIZE(app_led_fade_sequence),
	ARRAY_SIZE(app_led_bike_blink_sequence), ARRAY_SIZE(app_led_chase_sequence),
	ARRAY_SIZE(app_led_fade_blink_sequence), ARRAY_SIZE(app_led_half_blink_sequence),
	ARRAY_SIZE(app_led_sine_sequence),	 ARRAY_SIZE(app_led_breathe_sequence),
};

#define APP_LED_NUM_SEQUENCES ARRAY_SIZE(app_led_sequences)
