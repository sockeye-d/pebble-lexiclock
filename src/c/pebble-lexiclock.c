#include <pebble.h>

static Window *window;
static Layer *main_layer;

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

typedef struct {
	MinuteOffset minute_offset;
	OffsetDirection offset_direction;
	int hour;
} WordTime;

static WordTime last_word_time;
static WordTime word_time;

static char strings[8][12] = {
	"it-is-twenty",
	"ten--five--a",
	"quarterhalf-",
	"past-to-one-",
	"twothreefour",
	"fivesixseven",
	"eightnineten",
	"eleventwelve",
};

static GBitmap *base_font_atlas[26];
static GBitmap *base_bold_font_atlas[26];
static GBitmap *faint_font_atlas[26];
static GBitmap *font_atlas[26];

static int32_t animation_ticks = INT32_MIN;
static int animation_y		   = -1;

#define PlatformTypeAplite 0
#define PlatformTypeBasalt 1
#define PlatformTypeChalk 2
#define PlatformTypeDiorite 3
#define PlatformTypeEmery 4

#if PBL_PLATFORM_TYPE_CURRENT == PlatformTypeEmery
#define BITMAP_WIDTH 14
#define BITMAP_HEIGHT 30
#define CHAR_X_OFFSET 16
#define CHAR_Y_OFFSET 10
#define CHAR_X_STEP 14
#define CHAR_Y_STEP 26
#elif PBL_PLATFORM_TYPE_CURRENT == PlatformTypeBasalt
#define BITMAP_WIDTH 10
#define BITMAP_HEIGHT 20
#define CHAR_X_OFFSET 6
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
#define BITMAP_SIZE (BITMAP_WIDTH * BITMAP_HEIGHT)

static uint8_t bitmaps[2][26][BITMAP_SIZE];

enum AnimationState {
	ANIMATION_JUST_ENDED = -2,
	ANIMATION_ENDED		 = -1,
	ANIMATION_START		 = 0,
};

static void animation_callback(void *data) {
	animation_y += 1;
	layer_mark_dirty(main_layer);
	if (animation_y >= 10) {
		animation_y = ANIMATION_JUST_ENDED;
		return;
	}
	app_timer_register(50, animation_callback, NULL);
}

static void load_font_atlases(GBitmap *base_atlas[26], GBitmap *atlas[26], uint8_t bitmap_storage[26][BITMAP_SIZE]) {
	for (int i = 0; i < 26; i++) {
		const GBitmap *bitmap	  = base_atlas[i];
		const uint8_t *first_data = gbitmap_get_data(bitmap);
		memcpy(bitmap_storage[i], first_data, BITMAP_SIZE);
		GBitmap *ptr = gbitmap_create_blank(GSize(BITMAP_WIDTH, BITMAP_HEIGHT), GBitmapFormat8Bit);
		gbitmap_set_data(ptr, bitmap_storage[i], GBitmapFormat8Bit, BITMAP_WIDTH, false);
		atlas[i] = ptr;
	}
}

static void modulate_font_atlas(GBitmap *target[26], GColor background, GColor foreground) {
	for (int i = 0; i < 26; i++) {
		GColor8 *bitmap = (GColor8 *)gbitmap_get_data(target[i]);
		for (int y = 0; y < BITMAP_HEIGHT; y++) {
			for (int x = 0; x < BITMAP_WIDTH; x++) {
				GColor8 pixel	   = (GColor8)(bitmap[y * BITMAP_WIDTH + x]);
				uint8_t brightness = pixel.r;
				bitmap[y * BITMAP_WIDTH + x] =
						GColorFromRGBA(foreground.r << 6, foreground.g << 6,
								foreground.b << 6, brightness << 6);
			}
		}
	}
}

uint32_t hash(uint32_t x) {
	x = ((x >> 16) ^ x) * 0x45d9f3bu;
	x = ((x >> 16) ^ x) * 0x45d9f3bu;
	x = (x >> 16) ^ x;
	return x;
}

static char pick_random_char(uint32_t i, uint32_t j, uint32_t k) {
	return 'a' + (hash(i) ^ hash(j) ^ hash(k)) % ('z' - 'a');
}

#define POSMOD(x, n) (((x) % (n) + (n)) % (n))
#define IS_MINUTE(minute) (word_time.minute_offset == minute)
#define IS_HOUR(__hour) (word_time.hour == __hour)
#define X(coord, condition) \
	case coord:             \
		lit = condition;    \
		break;

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
	word_time.hour = POSMOD(hour_24 - 1, 12) + 1;
	word_time.offset_direction =
			minute > 34 ? OFFSET_DIRECTION_TO : OFFSET_DIRECTION_PAST;
	word_time.minute_offset = get_minute_offset(
			word_time.offset_direction == OFFSET_DIRECTION_TO ? 64 - minute : minute);
	if (word_time.offset_direction == OFFSET_DIRECTION_TO) {
		word_time.hour = (word_time.hour + 1) % 12;
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

static bool last_state[8][12];

static void main_layer_draw(Layer *layer, GContext *ctx) {
	int y	 = CHAR_Y_OFFSET;
	bool lit = true;
	graphics_context_set_compositing_mode(ctx, GCompOpSet);
	for (size_t i = 0; i < 8; i++) {
		int x = CHAR_X_OFFSET;
		for (size_t j = 0; j < 12; j++) {
			char ch = strings[i][j];
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
						X(5, IS_MINUTE(FIVE) || IS_MINUTE(TWENTY_FIVE))
						X(11, IS_MINUTE(QUARTER))
					}
					break;
				case 2:
					switch (j) { X(7, IS_MINUTE(HALF)); }
					break;
				case 3:
					switch (j) {
						X(0, word_time.offset_direction == OFFSET_DIRECTION_PAST)
						X(5, word_time.offset_direction == OFFSET_DIRECTION_TO)
						X(8, IS_HOUR(1))
						X(12, false)
					}
					break;
				case 4:
					switch (j) {
						X(0, IS_HOUR(2))
						X(3, IS_HOUR(3))
						X(8, IS_HOUR(4))
					}
					break;
				case 5:
					switch (j) {
						X(0, IS_HOUR(5))
						X(4, IS_HOUR(6))
						X(7, IS_HOUR(7))
					}
					break;
				case 6:
					switch (j) {
						X(0, IS_HOUR(8))
						X(5, IS_HOUR(9))
						X(9, IS_HOUR(10))
					}
					break;
				case 7:
					switch (j) {
						X(0, IS_HOUR(11))
						X(6, IS_HOUR(12))
					}
					break;
			}
			if (animation_y == -2) {
				last_state[i][j] = lit;
			}
			bool really_lit		   = lit;
			bool is_placeholder	   = ch == '-';
			int distance		   = (i - 4) * (i - 4) * 3 + (j - 6) * (j - 6);
			bool in_animation	   = distance > animation_y * animation_y;
			int distance_fac	   = distance - animation_y * animation_y;
			bool in_animation_edge = distance_fac <= 0 && distance_fac > -32;
			if (is_placeholder) {
				int seed = animation_ticks;
				if (animation_y > ANIMATION_ENDED && in_animation) {
					seed--;
				}
				ch		   = pick_random_char(i, j, seed);
				really_lit = false;
			} else if (in_animation_edge && !lit && animation_y > ANIMATION_ENDED) {
				ch		   = pick_random_char(i, j, animation_ticks);
				really_lit = true;
			} else if (animation_y > ANIMATION_ENDED) {
				if (in_animation) {
					really_lit = last_state[i][j];
				}
			}
			graphics_draw_bitmap_in_rect(
					ctx, (really_lit ? font_atlas : faint_font_atlas)[ch - 'a'],
					GRect(x, y, BITMAP_WIDTH, BITMAP_HEIGHT));
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

	window_set_background_color(window, GColorBlack);

	for (int id = RESOURCE_ID_IOSEVKA_ATLAS_0; id < RESOURCE_ID_IOSEVKA_ATLAS_25 + 1; id++) {
		base_font_atlas[id - RESOURCE_ID_IOSEVKA_ATLAS_0] = gbitmap_create_with_resource(id);
	}

	for (int id = RESOURCE_ID_IOSEVKA_ATLAS_BOLD_0; id < RESOURCE_ID_IOSEVKA_ATLAS_BOLD_25 + 1; id++) {
		base_bold_font_atlas[id - RESOURCE_ID_IOSEVKA_ATLAS_BOLD_0] = gbitmap_create_with_resource(id);
	}

	load_font_atlases(base_font_atlas, faint_font_atlas, bitmaps[0]);
	load_font_atlases(base_bold_font_atlas, font_atlas, bitmaps[1]);
	modulate_font_atlas(faint_font_atlas, GColorBlack, GColorDarkGray);
	modulate_font_atlas(font_atlas, GColorBlack, GColorInchworm);

	main_layer = layer_create(bounds);
	layer_add_child(window_layer, main_layer);
	layer_set_update_proc(main_layer, main_layer_draw);

	tick_timer_service_subscribe(SECOND_UNIT, on_time_changed);
	time_t now		 = time(NULL);
	tm *current_time = localtime(&now);
	on_time_changed(current_time, 0);
}

static void on_window_unload(Window *window) {
	layer_destroy(main_layer);
}

static void init(void) {
	window = window_create();
	window_set_window_handlers(window, (WindowHandlers){
											   .load   = on_window_load,
											   .unload = on_window_unload,
									   });
	window_stack_push(window, false);
}

static void deinit(void) {
	window_destroy(window);
	// for (int i = 0; i < 26; i++) {
	// gbitmap_destroy(base_font_atlas[i]);
	//
	// free(gbitmap_get_data(font_atlas[i]));
	// gbitmap_destroy(font_atlas[i]);
	//
	// free(gbitmap_get_data(faint_font_atlas[i]));
	// gbitmap_destroy(faint_font_atlas[i]);
	// }
}

int main(void) {
	init();
	app_event_loop();
	deinit();
}
