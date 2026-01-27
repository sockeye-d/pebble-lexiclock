#include "settings.h"

// #define PREVIEW_TIME

static Window *window	 = NULL;
static Layer *main_layer = NULL;

typedef enum {
	ZERO		= 0,
	FIVE		= 5,
	TEN			= 10,
	QUARTER		= 15,
	TWENTY		= 20,
	TWENTY_FIVE = 25,
	HALF		= 30,
} MinuteOffset;

static MinuteOffset minute_offsets[] = {
	ZERO,
	FIVE,
	TEN,
	QUARTER,
	TWENTY,
	TWENTY_FIVE,
	HALF,
};

typedef enum {
	OFFSET_DIRECTION_PAST,
	OFFSET_DIRECTION_TO,
	OFFSET_DIRECTION_NONE,
} OffsetDirection;

enum AnimationState {
	ANIMATION_JUST_ENDED = -2,
	ANIMATION_ENDED		 = -1,
	ANIMATION_START		 = 0,
};

typedef struct {
	MinuteOffset minute_offset;
	OffsetDirection offset_direction;
	int hour;
} WordTime;

static WordTime last_word_time;
static WordTime word_time;

static GBitmap *base_faint_font_atlas[37];
static GBitmap *base_bold_font_atlas[37];
static GBitmap *faint_font_atlas[37];
static GBitmap *bright_font_atlas[37];

#if !IS_COLOR
static GBitmap *dither1 = NULL;
static GBitmap *dither2 = NULL;
#endif

static int32_t animation_ticks = INT32_MIN;
static int animation_y		   = ANIMATION_ENDED;

#define PlatformTypeAplite 0
#define PlatformTypeBasalt 1
#define PlatformTypeChalk 2
#define PlatformTypeDiorite 3
#define PlatformTypeEmery 4

#if PBL_PLATFORM_TYPE_CURRENT == PlatformTypeEmery

#define STRING_STYLE_EMERY

#define STRINGS_HEIGHT 7
#define STRINGS_WIDTH 13

static char strings[STRINGS_HEIGHT][STRINGS_WIDTH] = {
	"it-is-twenty-",
	"-ten-halfive-",
	"quarter-pasto",
	"twone-foursix",
	"threeleventen",
	"fiveightwelve",
	"-----sevenine",
};

#define BITMAP_WIDTH 14
#define BITMAP_HEIGHT 30
#define CHAR_X_OFFSET 10
#define CHAR_Y_OFFSET 18
#define CHAR_X_STEP 14
#define CHAR_Y_STEP 28

#else

#define STRING_STYLE_BASALT

#define STRINGS_HEIGHT 8
#define STRINGS_WIDTH 12
#if 1
static char strings[STRINGS_HEIGHT][STRINGS_WIDTH] = {
	"it-is-twenty",
	"ten-halfivea",
	"quarter-past",
	"to-twonefour",
	"eighthreesix",
	"twelveleven-",
	"fivenineten-",
	"-------seven",
};

#else

static char strings[8][12] = {
	"accordingtoa",
	"llknownlawso",
	"faviationthe",
	"reisnowayabe",
	"eshouldbeabl",
	"etoflyitswin",
	"gsaretoosmal",
	"ltogetitsfat",
};
#endif

#if PBL_IF_COLOR_ELSE(true, false)

#define BITMAP_WIDTH 10
#define BITMAP_HEIGHT 20

#else

#define BITMAP_WIDTH 10
#define BITMAP_HEIGHT 20

#endif

#define CHAR_X_OFFSET 7
#define CHAR_Y_OFFSET 6
#define CHAR_X_STEP 11
#define CHAR_Y_STEP 20

#endif

#undef PlatformTypeAplite
#undef PlatformTypeBasalt
#undef PlatformTypeChalk
#undef PlatformTypeDiorite
#undef PlatformTypeEmery

// 1 byte per pixel
#define BITMAP_SIZE PBL_IF_COLOR_ELSE(BITMAP_WIDTH *BITMAP_HEIGHT, BITMAP_WIDTH *BITMAP_HEIGHT / 8)

enum InfoStringState {
	INFO_STRING_NOT_SHOWN = -1,
	INFO_STRING_SHOWN	  = 10,
};

static enum InfoStringState info_string_state			= INFO_STRING_NOT_SHOWN;
static char info_strings[STRINGS_HEIGHT][STRINGS_WIDTH] = {};

static int string_map[] = {
	// Non-printables
	0, // ,
	0, // ,
	0, // ,
	0, // ,
	0, // ,
	0, // ,
	0, // ,
	0, // ,
	0, //,
	0, // ,
	0, // ,
	0, // ,
	0, // ,
	0, // ,
	0, // ,
	0, // ,
	0, // ,
	0, // ,
	0, // ,
	0, // ,
	0, // ,
	0, // ,
	0, // ,
	0, // ,
	0, // ,
	0, // ,
	0, // ,
	0, //
	0, // ,
	0, // ,
	0, // ,
	0, // ,
	0, // ,
	0, // !,
	0, // ",
	0, // #,
	0, // $,
	36, // %,
	0, // &,
	0, // ',
	0, // (,
	0, // ),
	0, // *,
	0, // +,
	0, // ,,
	0, // -,
	0, // .,
	0, // /,
	26, // 0,
	27, // 1,
	28, // 2,
	29, // 3,
	30, // 4,
	31, // 5,
	32, // 6,
	33, // 7,
	34, // 8,
	35, // 9,
	0, // :,
	0, // ;,
	0, // <,
	0, // =,
	0, // >,
	0, // ?,
	0, // @,
	0, // A,
	1, // B,
	2, // C,
	3, // D,
	4, // E,
	5, // F,
	6, // G,
	7, // H,
	8, // I,
	9, // J,
	10, // K,
	11, // L,
	12, // M,
	13, // N,
	14, // O,
	15, // P,
	16, // Q,
	17, // R,
	18, // S,
	19, // T,
	20, // U,
	21, // V,
	22, // W,
	23, // X,
	24, // Y,
	25, // Z,
	0, // [,
	0, // \,
	0, // ],
	0, // ^,
	0, // _,
	0, // `,
	0, // a,
	1, // b,
	2, // c,
	3, // d,
	4, // e,
	5, // f,
	6, // g,
	7, // h,
	8, // i,
	9, // j,
	10, // k,
	11, // l,
	12, // m,
	13, // n,
	14, // o,
	15, // p,
	16, // q,
	17, // r,
	18, // s,
	19, // t,
	20, // u,
	21, // v,
	22, // w,
	23, // x,
	24, // y,
	25, // z,
	0, // {,
	0, // |,
	0, // },
	0, // ~,
	0, // ,
	0, // ,
};

static void reset_info_strings() {
	for (int i = 0; i < STRINGS_HEIGHT; i++) {
		for (int j = 0; j < STRINGS_WIDTH; j++) {
			info_strings[i][j] = '-';
		}
	}
}

#define WRITE_STRING(m_x, m_y, ...)                                                          \
	do {                                                                                     \
		int m_pos = snprintf(&info_strings[m_y][m_x], STRINGS_WIDTH - m_x + 1, __VA_ARGS__); \
		if (m_x + m_pos < STRINGS_WIDTH) {                                                   \
			info_strings[m_y][m_x + m_pos] = '-';                                            \
		}                                                                                    \
	} while (false)

static void setup_info_strings(BatteryChargeState battery, struct tm *time) {
	reset_info_strings();
#define MONTH WRITE_STRING(0, 0, "%d", time->tm_mon + 1);
#define DAY WRITE_STRING(0, 1, "%d", time->tm_mday);
#define YEAR WRITE_STRING(0, 2, "%d", time->tm_year + 1900);
	switch (settings.date_mode) {
		case DATE_MODE_MDY:
			MONTH DAY YEAR break;
		case DATE_MODE_DMY:
			DAY MONTH YEAR break;
		case DATE_MODE_YMD:
			YEAR MONTH DAY break;
	}
#undef MONTH
#undef DAY
#undef YEAR
	WRITE_STRING(0, STRINGS_HEIGHT - 2, "%d%%", battery.charge_percent);
	WRITE_STRING(0, STRINGS_HEIGHT - 3, "FISHY");
	int y = STRINGS_HEIGHT - 1;
	for (int i = 0; i < STRINGS_WIDTH; i++) {
		if (i >= (int)(100 - battery.charge_percent) * STRINGS_WIDTH / 100) {
			info_strings[y][STRINGS_WIDTH - i - 1] = '_';
		}
	}
}

static void animation_callback(void *data) {
	animation_y += 1;
	layer_mark_dirty(main_layer);
	if (animation_y >= 10) {
		animation_y = ANIMATION_JUST_ENDED;
		return;
	}
	app_timer_register(50, animation_callback, NULL);
}

static void tap_animation_callback_backward(void *data) {
	info_string_state--;
	layer_mark_dirty(main_layer);
	if (info_string_state > INFO_STRING_NOT_SHOWN) {
		app_timer_register(50, tap_animation_callback_backward, data);
	}
}

static void tap_animation_callback_forward(void *data) {
	info_string_state++;
	layer_mark_dirty(main_layer);
	if (info_string_state < INFO_STRING_SHOWN + 1) {
		app_timer_register(50, tap_animation_callback_forward, data);
	} else {
		app_timer_register(3000, tap_animation_callback_backward, data);
	}
}

static void on_tap(AccelAxisType axis, int32_t direction) {
	if (info_string_state == INFO_STRING_NOT_SHOWN) {
		info_string_state		   = 0;
		BatteryChargeState battery = battery_state_service_peek();
		time_t now				   = time(NULL);
		tm *current_time		   = localtime(&now);
		setup_info_strings(battery, current_time);
		tap_animation_callback_forward(NULL);
	}
}

static void load_font_atlases(GBitmap *base_atlas[RESOURCE_ID_IOSEVKA_ATLAS_36 - RESOURCE_ID_IOSEVKA_ATLAS_0 + 1], GBitmap *atlas[RESOURCE_ID_IOSEVKA_ATLAS_36 - RESOURCE_ID_IOSEVKA_ATLAS_0 + 1]) {
	for (int i = 0; i < RESOURCE_ID_IOSEVKA_ATLAS_36 - RESOURCE_ID_IOSEVKA_ATLAS_0 + 1; i++) {
		const uint8_t *first_data = gbitmap_get_data(base_atlas[i]);
		GBitmap *atlas_bitmap	  = gbitmap_create_blank(GSize(BITMAP_WIDTH, BITMAP_HEIGHT), PBL_IF_COLOR_ELSE(GBitmapFormat8Bit, GBitmapFormat1Bit));
		memcpy(gbitmap_get_data(atlas_bitmap), first_data, gbitmap_get_bytes_per_row(atlas_bitmap) * BITMAP_HEIGHT);
		if (atlas[i] != NULL) {
			gbitmap_destroy(atlas[i]);
		}
		atlas[i] = atlas_bitmap;
	}
}

#if IS_COLOR
static void modulate_font_atlas(GBitmap *source[RESOURCE_ID_IOSEVKA_ATLAS_36 - RESOURCE_ID_IOSEVKA_ATLAS_0 + 1], GBitmap *dest[RESOURCE_ID_IOSEVKA_ATLAS_36 - RESOURCE_ID_IOSEVKA_ATLAS_0 + 1], GColor foreground) {
	for (int i = 0; i < RESOURCE_ID_IOSEVKA_ATLAS_36 - RESOURCE_ID_IOSEVKA_ATLAS_0 + 1; i++) {
		GColor8 *source_bitmap = (GColor8 *)gbitmap_get_data(source[i]);
		GColor8 *dest_bitmap   = (GColor8 *)gbitmap_get_data(dest[i]);
		for (int y = 0; y < BITMAP_HEIGHT; y++) {
			for (int x = 0; x < BITMAP_WIDTH; x++) {
				GColor8 pixel					  = (GColor8)(source_bitmap[y * BITMAP_WIDTH + x]);
				uint8_t brightness				  = pixel.r;
				dest_bitmap[y * BITMAP_WIDTH + x] = GColorFromRGBA(foreground.r << 6, foreground.g << 6, foreground.b << 6, brightness << 6);
			}
		}
	}
}
#endif

uint32_t hash(uint32_t x) {
	x = ((x >> 16) ^ x) * 0x45d9f3bu;
	x = ((x >> 16) ^ x) * 0x45d9f3bu;
	x = (x >> 16) ^ x;
	return x;
}

static char pick_random_char(uint32_t i, uint32_t j, uint32_t k) {
	return 'a' + (hash(i) ^ hash(j) ^ hash(k)) % ('z' - 'a');
}

static uint32_t pick_random_number(uint32_t i, uint32_t j, uint32_t k) {
	return hash(i) ^ hash(j) ^ hash(k);
}

#define POSMOD(x, n) (((x) % (n) + (n)) % (n))
#define IS_MINUTE(minute) (word_time.minute_offset == minute)
#define IS_HOUR(__hour) (word_time.hour == __hour)
#define IS_OFFSET(offset) (word_time.offset_direction == OFFSET_DIRECTION_##offset)
#define X(coord, condition) \
	case coord:             \
		*r_set = true;      \
		return condition;

static int get_minute_offset(int minute) {
	for (int i = 6; i >= 0; i--) {
		int offset = minute_offsets[i];
		if (offset <= minute) {
			return offset;
		}
	}
	return ZERO;
}

static void parse_word_time(int hour_24, int minute) {
	last_word_time = word_time;
#ifdef PREVIEW_TIME
	hour_24 = 11;
	minute	= 35;
#endif
	word_time.hour = POSMOD(hour_24 - 1, 12) + 1;
	word_time.offset_direction =
			minute > 34 ? OFFSET_DIRECTION_TO : OFFSET_DIRECTION_PAST;
	word_time.minute_offset = get_minute_offset(
			word_time.offset_direction == OFFSET_DIRECTION_TO ? 64 - minute : minute);
	if (word_time.offset_direction == OFFSET_DIRECTION_TO) {
		word_time.hour = word_time.hour % 12 + 1;
	}
	if (word_time.minute_offset == ZERO) {
		word_time.offset_direction = OFFSET_DIRECTION_NONE;
	}
}

static bool word_time_changed() {
	return last_word_time.hour != word_time.hour || last_word_time.minute_offset != word_time.minute_offset || last_word_time.offset_direction != word_time.offset_direction;
}

void on_time_changed(struct tm *tick_time, TimeUnits units_changed) {
	parse_word_time(tick_time->tm_hour, tick_time->tm_min);
	if (word_time_changed()) {
		app_timer_register(50, animation_callback, NULL);
		animation_y = 0;
		animation_ticks++;
	}
}

static bool last_state[STRINGS_HEIGHT][STRINGS_WIDTH];

#ifdef STRING_STYLE_BASALT

static bool get_char_state(size_t i, size_t j, bool *r_set) {
	*r_set = false;
	switch (i) {
		case 0:
			switch (j) {
				X(0, true /* 'it is' is always lit */)
				X(6, IS_MINUTE(TWENTY) || IS_MINUTE(TWENTY_FIVE))
			}
			break;
		case 1:
			switch (j) {
				X(0, IS_MINUTE(TEN))
				X(4, IS_MINUTE(HALF))
				X(7, IS_MINUTE(HALF) || IS_MINUTE(FIVE) || IS_MINUTE(TWENTY_FIVE))
				X(8, IS_MINUTE(FIVE) || IS_MINUTE(TWENTY_FIVE))
				X(11, IS_MINUTE(QUARTER) && !settings.british_mode)
			}
			break;
		case 2:
			switch (j) {
				X(0, IS_MINUTE(QUARTER))
				X(8, word_time.offset_direction == OFFSET_DIRECTION_PAST);
			}
			break;
		case 3:
			switch (j) {
				X(0, word_time.offset_direction == OFFSET_DIRECTION_TO)
				X(3, IS_HOUR(2))
				X(5, IS_HOUR(2) || IS_HOUR(1))
				X(6, IS_HOUR(1))
				X(8, IS_HOUR(4))
			}
			break;
		case 4:
			switch (j) {
				X(0, IS_HOUR(8))
				X(4, IS_HOUR(8) || IS_HOUR(3))
				X(5, IS_HOUR(3))
				X(9, IS_HOUR(6))
			}
			break;
		case 5:
			switch (j) {
				X(0, IS_HOUR(12))
				X(5, IS_HOUR(12) || IS_HOUR(11))
				X(6, IS_HOUR(11))
			}
			break;
		case 6:
			switch (j) {
				X(0, IS_HOUR(5))
				X(4, IS_HOUR(9))
				X(8, IS_HOUR(10))
			}
			break;
		case 7:
			switch (j) {
				X(7, IS_HOUR(7))
			}
			break;
	}
	return false;
}

#else

static bool get_char_state(size_t i, size_t j, bool *r_set) {
	*r_set = false;
	switch (i) {
		case 0:
			switch (j) {
				X(0, true /* 'it is' is always lit */)
				X(6, IS_MINUTE(TWENTY) || IS_MINUTE(TWENTY_FIVE))
			}
			break;
		case 1:
			switch (j) {
				X(1, IS_MINUTE(TEN))
				X(5, IS_MINUTE(HALF))
				X(6, IS_MINUTE(HALF) || (IS_MINUTE(QUARTER) && !settings.british_mode))
				X(7, IS_MINUTE(HALF))
				X(8, IS_MINUTE(HALF) || IS_MINUTE(FIVE) || IS_MINUTE(TWENTY_FIVE))
				X(9, IS_MINUTE(FIVE) || IS_MINUTE(TWENTY_FIVE))
			}
			break;
		case 2:
			switch (j) {
				X(0, IS_MINUTE(QUARTER))
				X(8, IS_OFFSET(PAST))
				X(11, IS_OFFSET(PAST) || IS_OFFSET(TO))
				X(12, IS_OFFSET(TO))
			}
			break;
		case 3:
			switch (j) {
				X(0, IS_HOUR(2))
				X(2, IS_HOUR(2) || IS_HOUR(1))
				X(3, IS_HOUR(1))
				X(6, IS_HOUR(4))
				X(10, IS_HOUR(6))
			}
			break;
		case 4:
			switch (j) {
				X(0, IS_HOUR(3))
				X(4, IS_HOUR(3) || IS_HOUR(11))
				X(5, IS_HOUR(11))
				X(10, IS_HOUR(10))
			}
			break;
		case 5:
			switch (j) {
				X(0, IS_HOUR(5))
				X(3, IS_HOUR(5) || IS_HOUR(8))
				X(4, IS_HOUR(8))
				X(7, IS_HOUR(8) || IS_HOUR(12))
				X(8, IS_HOUR(12))
			}
			break;
		case 6:
			switch (j) {
				X(5, IS_HOUR(7))
				X(9, IS_HOUR(7) || IS_HOUR(9))
				X(10, IS_HOUR(9))
			}
			break;
	}
	return false;
}

#endif

static void main_layer_draw(Layer *layer, GContext *ctx) {
	int y	 = CHAR_Y_OFFSET;
	bool lit = true;
#if IS_COLOR
	graphics_context_set_compositing_mode(ctx, GCompOpSet);
#else
#endif
	bool use_info_strings = false;
	for (size_t i = 0; i < STRINGS_HEIGHT; i++) {
		int x = CHAR_X_OFFSET;
		for (size_t j = 0; j < STRINGS_WIDTH; j++) {
			if (info_string_state > INFO_STRING_NOT_SHOWN) {
				int32_t random_char = POSMOD(((int32_t)pick_random_number(i, j, 100000)) + 1234, INFO_STRING_SHOWN);
				use_info_strings	= random_char <= info_string_state;
			}
			char ch		= (use_info_strings ? info_strings : strings)[i][j];
			bool r_set	= false;
			bool result = use_info_strings || get_char_state(i, j, &r_set);
			if (r_set) {
				lit = result;
			}
			if (animation_y == ANIMATION_JUST_ENDED) {
				last_state[i][j] = lit;
			}
			bool really_lit		   = lit;
			bool is_placeholder	   = ch == '-' || ch == '_' || (ch >= 'A' && ch <= 'Z');
			int distance		   = (i - STRINGS_HEIGHT / 2) * (i - STRINGS_HEIGHT / 2) * 3 + (j - STRINGS_WIDTH / 2) * (j - STRINGS_WIDTH / 2);
			bool in_animation	   = distance > animation_y * animation_y;
			int distance_fac	   = distance - animation_y * animation_y;
			bool in_animation_edge = distance_fac <= 0 && distance_fac > -32;
			if (is_placeholder) {
				int seed = animation_ticks;
				if (animation_y > ANIMATION_ENDED && in_animation) {
					seed--;
				}
				really_lit = ch == '_';
				ch		   = ch >= 'A' && ch <= 'Z' ? ch - 'A' + 'a' : pick_random_char(i, j, seed);
			} else if (in_animation_edge && !lit && animation_y > ANIMATION_ENDED) {
				ch		   = pick_random_char(i, j, animation_ticks);
				really_lit = true;
			} else if (animation_y > ANIMATION_ENDED) {
				if (in_animation) {
					really_lit = last_state[i][j];
				}
			}
			int index = string_map[*(unsigned char *)&ch];
#if IS_COLOR
			graphics_draw_bitmap_in_rect(ctx, (really_lit ? bright_font_atlas : faint_font_atlas)[index], GRect(x, y, BITMAP_WIDTH, BITMAP_HEIGHT));
#else
			if (settings.light_on_dark) {
				graphics_context_set_compositing_mode(ctx, GCompOpAnd);
			} else {
				graphics_context_set_compositing_mode(ctx, GCompOpSet);
			}
			graphics_draw_bitmap_in_rect(ctx, base_faint_font_atlas[index], GRect(x, y, BITMAP_WIDTH, BITMAP_HEIGHT));
			if (really_lit) {
				if (settings.bright_bold) {
					graphics_draw_bitmap_in_rect(ctx, base_faint_font_atlas[index], GRect(x + 1, y, BITMAP_WIDTH, BITMAP_HEIGHT));
				}
			} else {
				if (settings.faint_bold) {
					graphics_draw_bitmap_in_rect(ctx, base_faint_font_atlas[index], GRect(x + 1, y, BITMAP_WIDTH, BITMAP_HEIGHT));
				}
				if (settings.light_on_dark) {
					graphics_context_set_compositing_mode(ctx, GCompOpSet);
				} else {
					graphics_context_set_compositing_mode(ctx, GCompOpAnd);
				}
				graphics_draw_bitmap_in_rect(ctx, settings.use_fainter_dithering ? dither2 : dither1, GRect(x, y, BITMAP_WIDTH, BITMAP_HEIGHT));
			}
#endif
			x += CHAR_X_STEP;
		}
		y += CHAR_Y_STEP;
	}
	if (animation_y == ANIMATION_JUST_ENDED) {
		animation_y = ANIMATION_ENDED;
	}
}

static void on_window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds		= layer_get_bounds(window_layer);

	main_layer = layer_create(bounds);
	layer_add_child(window_layer, main_layer);
	layer_set_update_proc(main_layer, main_layer_draw);

	load_settings();
	settings_changed();

	tick_timer_service_subscribe(MINUTE_UNIT, on_time_changed);
	time_t now		 = time(NULL);
	tm *current_time = localtime(&now);
	on_time_changed(current_time, 0);

	accel_tap_service_subscribe(on_tap);
}

static void on_window_unload(Window *window) {
	layer_destroy(main_layer);
}

static void settings_changed() {
#if IS_COLOR
	load_font_atlases(settings.faint_bold ? base_bold_font_atlas : base_faint_font_atlas, faint_font_atlas);
	load_font_atlases(settings.bright_bold ? base_bold_font_atlas : base_faint_font_atlas, bright_font_atlas);
	modulate_font_atlas(faint_font_atlas, faint_font_atlas, settings.faint_color);
	modulate_font_atlas(bright_font_atlas, bright_font_atlas, settings.bright_color);
#endif

#if IS_COLOR
	window_set_background_color(window, settings.bg_color);
#else
	window_set_background_color(window, settings.light_on_dark ? GColorWhite : GColorBlack);
#endif

	if (main_layer != NULL) {
		layer_mark_dirty(main_layer);
	}
}

static void init() {
	for (int id = RESOURCE_ID_IOSEVKA_ATLAS_0; id < RESOURCE_ID_IOSEVKA_ATLAS_36 + 1; id++) {
		base_faint_font_atlas[id - RESOURCE_ID_IOSEVKA_ATLAS_0] = gbitmap_create_with_resource(id);
	}

#if IS_COLOR
	for (int id = RESOURCE_ID_IOSEVKA_ATLAS_BOLD_0; id < RESOURCE_ID_IOSEVKA_ATLAS_BOLD_36 + 1; id++) {
		base_bold_font_atlas[id - RESOURCE_ID_IOSEVKA_ATLAS_BOLD_0] = gbitmap_create_with_resource(id);
	}
#endif

#if !IS_COLOR
	dither1 = gbitmap_create_with_resource(RESOURCE_ID_DITHER_PATTERN_1);
	dither2 = gbitmap_create_with_resource(RESOURCE_ID_DITHER_PATTERN_2);
#endif

	app_message_open(dict_calc_buffer_size(6), 0);
	app_message_register_inbox_received(inbox_received_handler);

	window = window_create();

	window_set_window_handlers(window, (WindowHandlers){
											   .load   = on_window_load,
											   .unload = on_window_unload,
									   });
	window_stack_push(window, true);
}

static void deinit() {
	window_destroy(window);
	app_message_deregister_callbacks();
	for (size_t i = 0; i < 26; i++) {
		gbitmap_destroy(base_faint_font_atlas[i]);
#if IS_COLOR
		gbitmap_destroy(base_bold_font_atlas[i]);
		gbitmap_destroy(faint_font_atlas[i]);
		gbitmap_destroy(bright_font_atlas[i]);
#endif
	}
}

int main() {
	init();
	app_event_loop();
	deinit();
}
