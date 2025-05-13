/* RGB color struct with union for easy access to RGB values, hex value, and byte array */
typedef struct {
	union {
		struct {
			uint8_t r; // red
			uint8_t g; // green
			uint8_t b; // blue
		};
		uint8_t bytes[3]; // array of bytes
		uint32_t hex;	  // hex value
	};
} rgb_color_t;

/* * RGB color macros
 * RGB(_r, _g, _b) - create a rgb_color_t from r,g,b values
 * RGBHEX(_code) - create a rgb_color_t from hex code
 * HEXRGB(_c) - convert a rgb_color_t to hex code
 */
#define RGB(_r, _g, _b)                                                                            \
	(rgb_color_t)                                                                              \
	{                                                                                          \
		.r = (_r), .g = (_g), .b = (_b)                                                    \
	}
#define RGBHEX(_code)                                                                              \
	(rgb_color_t){.r = ((_code) >> 16) & 0xFF, .g = ((_code) >> 8) & 0xFF, .b = (_code) & 0xFF}
#define HEXRGB(_c) (((_c).r << 16) | ((_c).g << 8) | (_c).b)

/* Predefined RGB colors - from FastLED */
typedef enum {
	AliceBlue = 0xF0F8FF,		 ///< @htmlcolorblock{F0F8FF}
	Amethyst = 0x9966CC,		 ///< @htmlcolorblock{9966CC}
	AntiqueWhite = 0xFAEBD7,	 ///< @htmlcolorblock{FAEBD7}
	Aqua = 0x00FFFF,		 ///< @htmlcolorblock{00FFFF}
	Aquamarine = 0x7FFFD4,		 ///< @htmlcolorblock{7FFFD4}
	Azure = 0xF0FFFF,		 ///< @htmlcolorblock{F0FFFF}
	Beige = 0xF5F5DC,		 ///< @htmlcolorblock{F5F5DC}
	Bisque = 0xFFE4C4,		 ///< @htmlcolorblock{FFE4C4}
	Black = 0x000000,		 ///< @htmlcolorblock{000000}
	BlanchedAlmond = 0xFFEBCD,	 ///< @htmlcolorblock{FFEBCD}
	Blue = 0x0000FF,		 ///< @htmlcolorblock{0000FF}
	BlueViolet = 0x8A2BE2,		 ///< @htmlcolorblock{8A2BE2}
	Brown = 0xA52A2A,		 ///< @htmlcolorblock{A52A2A}
	BurlyWood = 0xDEB887,		 ///< @htmlcolorblock{DEB887}
	CadetBlue = 0x5F9EA0,		 ///< @htmlcolorblock{5F9EA0}
	Chartreuse = 0x7FFF00,		 ///< @htmlcolorblock{7FFF00}
	Chocolate = 0xD2691E,		 ///< @htmlcolorblock{D2691E}
	Coral = 0xFF7F50,		 ///< @htmlcolorblock{FF7F50}
	CornflowerBlue = 0x6495ED,	 ///< @htmlcolorblock{6495ED}
	Cornsilk = 0xFFF8DC,		 ///< @htmlcolorblock{FFF8DC}
	Crimson = 0xDC143C,		 ///< @htmlcolorblock{DC143C}
	Cyan = 0x00FFFF,		 ///< @htmlcolorblock{00FFFF}
	DarkBlue = 0x00008B,		 ///< @htmlcolorblock{00008B}
	DarkCyan = 0x008B8B,		 ///< @htmlcolorblock{008B8B}
	DarkGoldenrod = 0xB8860B,	 ///< @htmlcolorblock{B8860B}
	DarkGray = 0xA9A9A9,		 ///< @htmlcolorblock{A9A9A9}
	DarkGrey = 0xA9A9A9,		 ///< @htmlcolorblock{A9A9A9}
	DarkGreen = 0x006400,		 ///< @htmlcolorblock{006400}
	DarkKhaki = 0xBDB76B,		 ///< @htmlcolorblock{BDB76B}
	DarkMagenta = 0x8B008B,		 ///< @htmlcolorblock{8B008B}
	DarkOliveGreen = 0x556B2F,	 ///< @htmlcolorblock{556B2F}
	DarkOrange = 0xFF8C00,		 ///< @htmlcolorblock{FF8C00}
	DarkOrchid = 0x9932CC,		 ///< @htmlcolorblock{9932CC}
	DarkRed = 0x8B0000,		 ///< @htmlcolorblock{8B0000}
	DarkSalmon = 0xE9967A,		 ///< @htmlcolorblock{E9967A}
	DarkSeaGreen = 0x8FBC8F,	 ///< @htmlcolorblock{8FBC8F}
	DarkSlateBlue = 0x483D8B,	 ///< @htmlcolorblock{483D8B}
	DarkSlateGray = 0x2F4F4F,	 ///< @htmlcolorblock{2F4F4F}
	DarkSlateGrey = 0x2F4F4F,	 ///< @htmlcolorblock{2F4F4F}
	DarkTurquoise = 0x00CED1,	 ///< @htmlcolorblock{00CED1}
	DarkViolet = 0x9400D3,		 ///< @htmlcolorblock{9400D3}
	DeepPink = 0xFF1493,		 ///< @htmlcolorblock{FF1493}
	DeepSkyBlue = 0x00BFFF,		 ///< @htmlcolorblock{00BFFF}
	DimGray = 0x696969,		 ///< @htmlcolorblock{696969}
	DodgerBlue = 0x1E90FF,		 ///< @htmlcolorblock{1E90FF}
	FireBrick = 0xB22222,		 ///< @htmlcolorblock{B22222}
	FloralWhite = 0xFFFAF0,		 ///< @htmlcolorblock{FFFAF0}
	ForestGreen = 0x228B22,		 ///< @htmlcolorblock{228B22}
	Fuchsia = 0xFF00FF,		 ///< @htmlcolorblock{FF00FF}
	Gainsboro = 0xDCDCDC,		 ///< @htmlcolorblock{DCDCDC}
	GhostWhite = 0xF8F8FF,		 ///< @htmlcolorblock{F8F8FF}
	Gold = 0xFFD700,		 ///< @htmlcolorblock{FFD700}
	Goldenrod = 0xDAA520,		 ///< @htmlcolorblock{DAA520}
	Gray = 0x808080,		 ///< @htmlcolorblock{808080}
	Grey = 0x808080,		 ///< @htmlcolorblock{808080}
	Green = 0x008000,		 ///< @htmlcolorblock{008000}
	GreenYellow = 0xADFF2F,		 ///< @htmlcolorblock{ADFF2F}
	Honeydew = 0xF0FFF0,		 ///< @htmlcolorblock{F0FFF0}
	HotPink = 0xFF69B4,		 ///< @htmlcolorblock{FF69B4}
	IndianRed = 0xCD5C5C,		 ///< @htmlcolorblock{CD5C5C}
	Indigo = 0x4B0082,		 ///< @htmlcolorblock{4B0082}
	Ivory = 0xFFFFF0,		 ///< @htmlcolorblock{FFFFF0}
	Khaki = 0xF0E68C,		 ///< @htmlcolorblock{F0E68C}
	Lavender = 0xE6E6FA,		 ///< @htmlcolorblock{E6E6FA}
	LavenderBlush = 0xFFF0F5,	 ///< @htmlcolorblock{FFF0F5}
	LawnGreen = 0x7CFC00,		 ///< @htmlcolorblock{7CFC00}
	LemonChiffon = 0xFFFACD,	 ///< @htmlcolorblock{FFFACD}
	LightBlue = 0xADD8E6,		 ///< @htmlcolorblock{ADD8E6}
	LightCoral = 0xF08080,		 ///< @htmlcolorblock{F08080}
	LightCyan = 0xE0FFFF,		 ///< @htmlcolorblock{E0FFFF}
	LightGoldenrodYellow = 0xFAFAD2, ///< @htmlcolorblock{FAFAD2}
	LightGreen = 0x90EE90,		 ///< @htmlcolorblock{90EE90}
	LightGrey = 0xD3D3D3,		 ///< @htmlcolorblock{D3D3D3}
	LightPink = 0xFFB6C1,		 ///< @htmlcolorblock{FFB6C1}
	LightSalmon = 0xFFA07A,		 ///< @htmlcolorblock{FFA07A}
	LightSeaGreen = 0x20B2AA,	 ///< @htmlcolorblock{20B2AA}
	LightSkyBlue = 0x87CEFA,	 ///< @htmlcolorblock{87CEFA}
	LightSlateGray = 0x778899,	 ///< @htmlcolorblock{778899}
	LightSlateGrey = 0x778899,	 ///< @htmlcolorblock{778899}
	LightSteelBlue = 0xB0C4DE,	 ///< @htmlcolorblock{B0C4DE}
	LightYellow = 0xFFFFE0,		 ///< @htmlcolorblock{FFFFE0}
	Lime = 0x00FF00,		 ///< @htmlcolorblock{00FF00}
	LimeGreen = 0x32CD32,		 ///< @htmlcolorblock{32CD32}
	Linen = 0xFAF0E6,		 ///< @htmlcolorblock{FAF0E6}
	Magenta = 0xFF00FF,		 ///< @htmlcolorblock{FF00FF}
	Maroon = 0x800000,		 ///< @htmlcolorblock{800000}
	MediumAquamarine = 0x66CDAA,	 ///< @htmlcolorblock{66CDAA}
	MediumBlue = 0x0000CD,		 ///< @htmlcolorblock{0000CD}
	MediumOrchid = 0xBA55D3,	 ///< @htmlcolorblock{BA55D3}
	MediumPurple = 0x9370DB,	 ///< @htmlcolorblock{9370DB}
	MediumSeaGreen = 0x3CB371,	 ///< @htmlcolorblock{3CB371}
	MediumSlateBlue = 0x7B68EE,	 ///< @htmlcolorblock{7B68EE}
	MediumSpringGreen = 0x00FA9A,	 ///< @htmlcolorblock{00FA9A}
	MediumTurquoise = 0x48D1CC,	 ///< @htmlcolorblock{48D1CC}
	MediumVioletRed = 0xC71585,	 ///< @htmlcolorblock{C71585}
	MidnightBlue = 0x191970,	 ///< @htmlcolorblock{191970}
	MintCream = 0xF5FFFA,		 ///< @htmlcolorblock{F5FFFA}
	MistyRose = 0xFFE4E1,		 ///< @htmlcolorblock{FFE4E1}
	Moccasin = 0xFFE4B5,		 ///< @htmlcolorblock{FFE4B5}
	NavajoWhite = 0xFFDEAD,		 ///< @htmlcolorblock{FFDEAD}
	Navy = 0x000080,		 ///< @htmlcolorblock{000080}
	OldLace = 0xFDF5E6,		 ///< @htmlcolorblock{FDF5E6}
	Olive = 0x808000,		 ///< @htmlcolorblock{808000}
	OliveDrab = 0x6B8E23,		 ///< @htmlcolorblock{6B8E23}
	Orange = 0xFFA500,		 ///< @htmlcolorblock{FFA500}
	OrangeRed = 0xFF4500,		 ///< @htmlcolorblock{FF4500}
	Orchid = 0xDA70D6,		 ///< @htmlcolorblock{DA70D6}
	PaleGoldenrod = 0xEEE8AA,	 ///< @htmlcolorblock{EEE8AA}
	PaleGreen = 0x98FB98,		 ///< @htmlcolorblock{98FB98}
	PaleTurquoise = 0xAFEEEE,	 ///< @htmlcolorblock{AFEEEE}
	PaleVioletRed = 0xDB7093,	 ///< @htmlcolorblock{DB7093}
	PapayaWhip = 0xFFEFD5,		 ///< @htmlcolorblock{FFEFD5}
	PeachPuff = 0xFFDAB9,		 ///< @htmlcolorblock{FFDAB9}
	Peru = 0xCD853F,		 ///< @htmlcolorblock{CD853F}
	Pink = 0xFFC0CB,		 ///< @htmlcolorblock{FFC0CB}
	Plaid = 0xCC5533,		 ///< @htmlcolorblock{CC5533}
	Plum = 0xDDA0DD,		 ///< @htmlcolorblock{DDA0DD}
	PowderBlue = 0xB0E0E6,		 ///< @htmlcolorblock{B0E0E6}
	Purple = 0x800080,		 ///< @htmlcolorblock{800080}
	Red = 0xFF0000,			 ///< @htmlcolorblock{FF0000}
	RosyBrown = 0xBC8F8F,		 ///< @htmlcolorblock{BC8F8F}
	RoyalBlue = 0x4169E1,		 ///< @htmlcolorblock{4169E1}
	SaddleBrown = 0x8B4513,		 ///< @htmlcolorblock{8B4513}
	Salmon = 0xFA8072,		 ///< @htmlcolorblock{FA8072}
	SandyBrown = 0xF4A460,		 ///< @htmlcolorblock{F4A460}
	SeaGreen = 0x2E8B57,		 ///< @htmlcolorblock{2E8B57}
	Seashell = 0xFFF5EE,		 ///< @htmlcolorblock{FFF5EE}
	Sienna = 0xA0522D,		 ///< @htmlcolorblock{A0522D}
	Silver = 0xC0C0C0,		 ///< @htmlcolorblock{C0C0C0}
	SkyBlue = 0x87CEEB,		 ///< @htmlcolorblock{87CEEB}
	SlateBlue = 0x6A5ACD,		 ///< @htmlcolorblock{6A5ACD}
	SlateGray = 0x708090,		 ///< @htmlcolorblock{708090}
	SlateGrey = 0x708090,		 ///< @htmlcolorblock{708090}
	Snow = 0xFFFAFA,		 ///< @htmlcolorblock{FFFAFA}
	SpringGreen = 0x00FF7F,		 ///< @htmlcolorblock{00FF7F}
	SteelBlue = 0x4682B4,		 ///< @htmlcolorblock{4682B4}
	Tan = 0xD2B48C,			 ///< @htmlcolorblock{D2B48C}
	Teal = 0x008080,		 ///< @htmlcolorblock{008080}
	Thistle = 0xD8BFD8,		 ///< @htmlcolorblock{D8BFD8}
	Tomato = 0xFF6347,		 ///< @htmlcolorblock{FF6347}
	Turquoise = 0x40E0D0,		 ///< @htmlcolorblock{40E0D0}
	Violet = 0xEE82EE,		 ///< @htmlcolorblock{EE82EE}
	Wheat = 0xF5DEB3,		 ///< @htmlcolorblock{F5DEB3}
	White = 0xFFFFFF,		 ///< @htmlcolorblock{FFFFFF}
	WhiteSmoke = 0xF5F5F5,		 ///< @htmlcolorblock{F5F5F5}
	Yellow = 0xFFFF00,		 ///< @htmlcolorblock{FFFF00}
	YellowGreen = 0x9ACD32,		 ///< @htmlcolorblock{9ACD32}
} CRGB;

/* Modes of the app_led module */
typedef enum {
	Manual,	  // manual code and use set_led etc. state machine will not clear
	Rainbow,  // rainbow cycle
	Blink,	  // use app_led_blink functions, app_led_state to control
	Sequence, // running sequence
	Error,	  // error indicator
	Off,	  // off - no operations will be performed
} LedMode;

/* Used to tag app_led_data_t with the type of LED hardware */
typedef enum {
	APP_LED_TYPE_GPIO,  // GPIO LED
	APP_LED_TYPE_PWM,   // PWM LED
	APP_LED_TYPE_STRIP, // RGB LED strip
} LedType;

struct app_led_pwm_config {
	int num_leds;
	const struct pwm_dt_spec *led;
};

struct led_gpio_config {
	int num_leds;
	const struct gpio_dt_spec *led;
};

/* struct to hold state of each LED */
struct app_led_state {
	rgb_color_t color;	   // desired color
	rgb_color_t _color;	   // actual color set - different for fade/blink check etc
	uint32_t on_time_ms_left;  // time left on for blink
	uint32_t off_time_ms_left; // time left off for blink
};

/* struct to hold sequence data */
typedef struct {
	uint16_t count;	    // counter for sequence
	rgb_color_t color;  // color to set at this step
	uint8_t brightness; // brightness to set at this step
	uint8_t fade_step;  // fade step for inc/dec of brightness
	int64_t last_tick;  // last tick for sequence timing
} app_led_sequence_data_t;

/* sequence function pointer type */
typedef void (*app_led_sequence_func_t)(void *const leds, const void *const step,
					k_timeout_t block);

/* struct to hold sequence step data */
typedef struct {
	rgb_color_t color;	     // color to set at this step
	app_led_sequence_func_t fnc; // optional function to call for this step
	uint8_t time_in_10ms;	     // 0xFF for last step, colour will be set for exit
	uint8_t start_brightness;    // brightness to start sequence
	uint8_t end_brightness;	     // brightness to end sequence
	uint8_t decay_rate;	     // rate to decay brightness 0xFF for no decay
} app_led_sequence_step_t;

typedef int (*app_led_update_hw_func_t)(void *leds);
typedef int (*app_led_set_pixels_func_t)(void *leds, uint16_t start, uint16_t end, rgb_color_t c,
					 uint8_t brightness, k_timeout_t block);
struct app_led_funcs {
	app_led_update_hw_func_t update_hw;   // function to update hardware
	app_led_set_pixels_func_t set_pixels; // function to set pixels
};

/* Main struct for controllable app_led device */
typedef struct {
	LedMode mode;			    // current display mode
	LedMode last_mode;		    // last mode before current to return
	const LedType hw_type;		    // tagged hardware type for any runtime checks
	const bool is_rgb;		    // true if RGB LED strip
	const uint8_t offset;		    // start offset to apply to device tree node phandle
	const uint8_t cell_size;            // number of LEDs in an addressable index
	struct k_mutex mutex;		    // mutex for mutli-thread access
	const struct device *const app_led; // pointer to device tree node
	uint8_t global_brightness;	    // current brightness
	uint8_t hue;			    // global hue for rainbow
	bool rainbow;			    // rainbow mode
	const uint8_t hw_num_leds;	    // number of LEDs in DT prop
	const uint8_t num_leds;		    // number of LEDs in sequence
	rgb_color_t global_color;	    // user colour for manual mode, will revert to this
	rgb_color_t _color;		    // current global color
        bool _toggle;		            // toggle state for blink
	struct app_led_state *const state;  // state of each led
	uint8_t sequence_step;		    // sequence step index
	const app_led_sequence_step_t *sequence; // current sequence frame
	uint32_t time_sequence_next;		 // tick to next
	int8_t sequence_repeat_count;		 // -1 to repeat forever
	app_led_sequence_data_t sequence_data;	 // data for sequence being run
	app_led_set_pixels_func_t set_pixels;	 // function to set pixels
	void *const pixels;			 // pixel buffer for RGB LED strip, NULL if not used
#if IS_ENABLED(CONFIG_APP_LED_USE_WORKQUEUE)
	struct k_work_delayable dwork; // delayed work for state machine update
#endif
} app_led_data_t;

/* Helper to calculate the number of logical LEDs in a chain; if it's not a strip LED and is RGB,
 * divide by 3
 */
#define APP_LED_CALC_NUM_LOGICAL_LEDS(_node_id, _num_leds, _is_rgb)                                \
	COND_CODE_1(DT_NODE_HAS_PROP(_node_id, chain_length), (_num_leds),                         \
		    ((_is_rgb) ? (_num_leds) / 3U : _num_leds))

/**
 * @brief Statically define and initialize an app_led_data instance with type info.
 *
 * @param _name Name of the app_led_data variable.
 * @param _node_id Devicetree node identifier for the underlying device (GPIO, PWM, SPI, etc.).
 * @param _num_hw_leds The total number of physical LEDs/pixels/components.
 * @param _is_rgb A compile-time constant (0 or 1). If non-zero, indicates logical RGB.
 */
#define APP_LED_STATIC_DEFINE(_name, _node_id, _num_hw_leds, _is_rgb)                              \
	/* Auto-detect hardware type based on known compat */                                      \
	static const LedType _name##_auto_hw_type = /* PWM type */                                 \
		COND_CODE_1(                                                                       \
			DT_NODE_HAS_COMPAT(_node_id, pwm_leds), (APP_LED_TYPE_PWM),                \
			(/* GPIO type */                                                           \
			 COND_CODE_1(DT_NODE_HAS_COMPAT(_node_id, gpio_leds), (APP_LED_TYPE_GPIO), \
				     (/* Fallback to strip type */                                 \
				      APP_LED_TYPE_STRIP))));                                      \
	static struct app_led_state _name##_state_array[APP_LED_CALC_NUM_LOGICAL_LEDS(             \
		_node_id, _num_hw_leds, _is_rgb)] = {0};                                           \
	COND_CODE_1(DT_NODE_HAS_PROP(_node_id, chain_length),                                      \
		    (static struct led_rgb _name##_pixel_buffer[(_num_hw_leds)] = {0};),           \
		    (static void *const _name##_pixel_buffer = NULL;))                             \
	app_led_data_t _name = {                                                                   \
		.mode = Manual,                                                                    \
		.last_mode = Manual,                                                               \
		.mutex = Z_MUTEX_INITIALIZER(_name.mutex),                                         \
		.app_led = DEVICE_DT_GET(_node_id),                                                \
		.hw_type = (_name##_auto_hw_type),                                                 \
		.is_rgb = (bool)(_is_rgb),                                                         \
		.offset = 0,                                                                       \
		.cell_size = 1,                                                                    \
		.hw_num_leds = (_num_hw_leds),                                                     \
		.num_leds = APP_LED_CALC_NUM_LOGICAL_LEDS(_node_id, _num_hw_leds, _is_rgb),        \
		.global_brightness = 0xFF,                                                         \
		.global_color = {.r = 0, .g = 0, .b = 0},                                          \
		._color = {.r = 0, .g = 0, .b = 0},                                                \
		._toggle = false,                                                                  \
		.hue = 0,                                                                          \
		.rainbow = false,                                                                  \
		.state = _name##_state_array,                                                      \
		.sequence_step = 0,                                                                \
		.sequence = NULL,                                                                  \
		.time_sequence_next = 0,                                                           \
		.sequence_repeat_count = 0,                                                        \
		.sequence_data = {0},                                                              \
		.pixels = _name##_pixel_buffer,                                                    \
	}

/* Helper to define a static discrete App LED chain of GPIO or PWM LEDs */
#define APP_LED_STATIC_IDV_DEFINE(_name, _node_id)                                                 \
	BUILD_ASSERT(DT_NODE_HAS_COMPAT(_node_id, gpio_leds) ||                                    \
			     DT_NODE_HAS_COMPAT(_node_id, pwm_leds),                               \
		     #_node_id "must be gpio_leds or pwm_leds");                                   \
	APP_LED_STATIC_DEFINE(_name, _node_id, DT_CHILD_NUM(_node_id), 0)
/* Helper to define a static RGB App LED using either GPIO or PWM App LEDs */
#define APP_LED_STATIC_RGB_DEFINE(_name, _node_id)                                                 \
	BUILD_ASSERT(DT_NODE_HAS_COMPAT(_node_id, gpio_leds) ||                                    \
			     DT_NODE_HAS_COMPAT(_node_id, pwm_leds),                               \
		     #_node_id "must be gpio_leds or pwm_leds");                                   \
	APP_LED_STATIC_DEFINE(_name, _node_id, DT_CHILD_NUM(_node_id), 1)
/* Helper to define a static RGB App LED strip using the chain_length property for hw_num_leds */
#define APP_LED_STATIC_STRIP_DEFINE(_name, _node_id)                                               \
	BUILD_ASSERT(IS_ENABLED(CONFIG_LED_STRIP), "CONFIG_LED_STRIP must be enabled");            \
	APP_LED_STATIC_DEFINE(_name, _node_id, DT_PROP(_node_id, chain_length), 1)
/* Helper to define a App LED with a offset for node LED array */
#define APP_LED_STATIC_OFFSET_DEFINE(_name, _node_id, _offset, _num_hw_leds, _is_rgb)              \
	APP_LED_STATIC_DEFINE(_name, _node_id, _num_hw_leds, _is_rgb);                             \
	_name.offset = _offset

/* @brief Run a LED sequence
 *
 * @param leds Pointer to the app_led_data_t structure
 * @param sequence Pointer to the sequence to run
 * @param num_repeat Number of times to repeat the sequence, -1 for infinite
 * @param block Timeout for blocking operation
 */
void app_led_run_sequence(app_led_data_t *leds, const app_led_sequence_step_t *sequence,
			  int8_t num_repeat, k_timeout_t block);
void app_led_sequence_clear(app_led_data_t *leds, k_timeout_t block);

/* @brief Blink an LED at specific index
 *
 * @param leds Pointer to the app_led_data_t structure
 * @param i Index of the LED to blink
 * @param c Color to blink
 * @param on_period_ms Time in milliseconds for the LED to be on
 * @param off_period_ms Time in milliseconds for the LED to be off
 * @param state_override Override the current state of the LED
 * @param block Timeout for blocking operation
 * @return 0 on success, negative error code on failure
 */
int app_led_blink_index(app_led_data_t *leds, uint16_t i, rgb_color_t c, uint32_t on_period_ms,
			uint32_t off_period_ms, bool state_override, k_timeout_t block);
/* @brief Blink all LEDs
 *
 * @param leds Pointer to the app_led_data_t structure
 * @param c Color to blink
 * @param on_period_ms Time in milliseconds for the LED to be on
 * @param off_period_ms Time in milliseconds for the LED to be off
 * @param state_override Override the current state of the LED
 * @param block Timeout for blocking operation
 * @return 0 on success, negative error code on failure
 */
int app_led_blink(app_led_data_t *leds, rgb_color_t c, uint32_t on_period_ms,
		  uint32_t off_period_ms, bool state_override, k_timeout_t block);
int app_led_blink_sync_index(app_led_data_t *leds, uint16_t i, rgb_color_t c, k_timeout_t block);
int app_led_blink_sync(app_led_data_t *leds, rgb_color_t c, k_timeout_t block);

/* @breif Fade to a color over a period of time
 *
 * Maniplates app_led_fade_sequence to set the color and brightness so will not work with multiple
 * App LEDs fading at once
 *
 * @param leds Pointer to the app_led_data_t structure
 * @param c Color to fade to
 * @param end_brightness Brightness at the end of the fade
 * @param fade_time_ms Time in milliseconds for the fade
 * @param block Timeout for blocking operation
 * @return 0 on success, negative error code on failure
 */
int app_led_fade_to(app_led_data_t *leds, rgb_color_t c, uint8_t end_brightness,
		    uint32_t fade_time_ms, k_timeout_t block);
/* @brief Fade on (brightness 255) over a period of time to global_color
 *
 * Desired color to fade to should be set first with app_led_set_global_color (with
 * app_led_set_global_brightness set to zero if fading on)
 *
 * @param leds Pointer to the app_led_data_t structure
 * @param fade_time_ms Time in milliseconds for the fade
 * @param block Timeout for blocking operation
 * @return 0 on success, negative error code on failure
 */
int app_led_fade_on(app_led_data_t *leds, uint32_t fade_time_ms, k_timeout_t block);
/* @brief Fade off (brightness 0) over a period of time from global_color
 *
 * @param leds Pointer to the app_led_data_t structure
 * @param fade_time_ms Time in milliseconds for the fade
 * @param block Timeout for blocking operation
 * @return 0 on success, negative error code on failure
 */
int app_led_fade_off(app_led_data_t *leds, uint32_t fade_time_ms, k_timeout_t block);

/* @brief Wait for LEDs to be inactive
 *
 * @param leds Pointer to the app_led_data_t structure
 * @param wait_ms Timeout to wait for LEDs to be inactive
 */
void app_led_wait_inactive(app_led_data_t *leds, k_timeout_t wait_ms);
/* @brief Wait for LEDs to finish running sequence
 *
 * @param leds Pointer to the app_led_data_t structure
 * @param wait_ms Timeout to wait for LEDs to finish running sequence
 */
void app_led_wait_sequence(app_led_data_t *leds, k_timeout_t wait_ms);
/* @brief Wait for LEDs to finish blinking
 *
 * @param leds Pointer to the app_led_data_t structure
 * @param wait_ms Timeout to wait for LEDs to finish blinking
 */
void app_led_wait_blink(app_led_data_t *leds, k_timeout_t wait_ms);

/* @brief Set the LED mode
 *
 * @param leds Pointer to the app_led_data_t structure
 * @param mode Mode to set
 * @param block Timeout for blocking operation
 */
void app_led_set_mode(app_led_data_t *leds, LedMode mode, k_timeout_t block);

/* @brief Set the color of a specific LED
 *
 * @param leds Pointer to the app_led_data_t structure
 * @param i Index of the LED to set
 * @param c Color to set
 * @param block Timeout for blocking operation
 * @return 0 on success, negative error code on failure
 */
int app_led_set_index(app_led_data_t *leds, uint16_t i, rgb_color_t c, k_timeout_t block);
/* @brief Set the color of all LEDs
 *
 * @param leds Pointer to the app_led_data_t structure
 * @param c Color to set
 * @param block Timeout for blocking operation
 * @return 0 on success, negative error code on failure
 */
int app_led_set_global_color(app_led_data_t *leds, rgb_color_t c, k_timeout_t block);
/* @brief Set the brightness of all LEDs
 *
 * @param leds Pointer to the app_led_data_t structure
 * @param brightness Brightness to set
 * @param block Timeout for blocking operation
 * @return 0 on success, negative error code on failure
 */
int app_led_set_global_brightness(app_led_data_t *leds, uint8_t brightness, k_timeout_t block);
/* @brief Initialize the passed leds
 *
 * Ensures dt node is valid and initialized then schedules work if enabled
 *
 * @param leds Pointer to the app_led_data_t structure
 * @return 0 on success, negative error code on failure
 */
int app_led_init(app_led_data_t *leds);
/* @brief Update state of the LEDs
 *
 * This function is called from the workqueue to update the state of the LEDs
 * and run the state machine. It should be called periodically to ensure the
 * LEDs are updated if CONFIG_APP_LED_USE_WORKQUEUE=n
 *
 * @param leds Pointer to the app_led_data_t structure
 */
void app_led_update(app_led_data_t *leds);

/* @brief Convert hue to RGB color
 *
 * @param hue Hue value to convert
 * @return RGB color value
 */
rgb_color_t app_led_hue_to_rgb(uint8_t hue);
/* @brief Convert HSV to RGB color
 *
 * @param hue Hue value to convert
 * @param sat Saturation value to convert
 * @param value Value value to convert
 * @return RGB color value
 */
rgb_color_t app_led_hsv_to_rgb(uint8_t hue, uint8_t sat, uint8_t value);

/* Helper to indicate Rx/Tx activity for example */
#define app_led_indicate_act(_l, _c) app_led_blink(_l, RGBHEX(_c), 20, 30, false, K_NO_WAIT);
/* Helper to run error sequence */
#define app_led_error_indicate(_l)                                                                 \
	app_led_run_sequence(_l, app_led_error_sequence, 0, 4, K_MSEC(5));

/**
 * Sequences
 *
 * See led_sequences.c for details on each sequence and how to define them.
 */

extern const app_led_sequence_step_t app_led_test_sequence[];
extern const app_led_sequence_step_t app_led_error_sequence[];
extern const app_led_sequence_step_t app_led_blank_sequence[];
extern app_led_sequence_step_t app_led_charging_sequence[];
extern app_led_sequence_step_t app_led_fade_sequence[];
extern const app_led_sequence_step_t app_led_chase_sequence[];
extern const app_led_sequence_step_t app_led_fade_blink_sequence[];
extern const app_led_sequence_step_t app_led_half_blink_sequence[];
extern const app_led_sequence_step_t app_led_sine_sequence[];
extern const app_led_sequence_step_t app_led_breathe_sequence[];
extern const app_led_sequence_step_t *app_led_sequences[];
extern const uint8_t app_led_sequence_lengths[];
void app_led_seq_fnc(void *const leds, const void *const step, k_timeout_t block);
void app_led_chase(void *const leds, const void *const step, k_timeout_t block);
void app_led_half_blink(void *const leds, const void *const step, k_timeout_t block);
void app_led_sine(void *const leds, const void *const step, k_timeout_t block);
void app_led_breathe(void *const l, const void *const s, k_timeout_t block);
void app_led_fade_blink(void *const l, const void *const s, k_timeout_t block);

enum LedSequences {
	LED_TEST_SEQUENCE,
	LED_ERROR_SEQUENCE,
	LED_BLANK_SEQUENCE,
	LED_CHARGING_SEQUENCE,
	LED_FADE_ON_SEQUENCE,
	LED_FADE_OFF_SEQUENCE,
	LED_BIKE_BLINK_SEQUENCE,
	LED_CHASE_SEQUENCE,
	LED_FADE_BLINK_SEQUENCE,
	LED_HALF_BLINK_SEQUENCE,
	LED_SINE_SEQUENCE,
	LED_BREATHE_SEQUENCE,
	LED_NUM_SEQUENCES,
};
