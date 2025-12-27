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

static struct WordTime {
	MinuteOffset minute_offset;
	OffsetDirection offset_direction;
	int hour;
} word_time;

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
static GBitmap *faint_font_atlas[26];
static GBitmap *font_atlas[26];

static int32_t animation_ticks = INT32_MIN;
static int animation_y		   = -1;

// 10 bytes * 20 rows + 12 header bytes
#define BITMAP_SIZE (10 * 20 /* + 12 */)

static uint8_t bitmaps[2][26][BITMAP_SIZE];

static void animation_callback(void *data) {
	animation_y++;
	layer_mark_dirty(main_layer);
	if (animation_y >= 8) {
		animation_y = -1;
		return;
	}
	app_timer_register(50, animation_callback, NULL);
}

static void load_font_atlases(GBitmap *atlas[26], uint8_t bitmap_storage[26][BITMAP_SIZE]) {
	for (int i = 0; i < 26; i++) {
		const GBitmap *bitmap	  = base_font_atlas[i];
		const uint8_t *first_data = gbitmap_get_data(bitmap);
		uint8_t *new_data		  = memcpy(bitmap_storage[i], first_data, BITMAP_SIZE);
		GBitmap *ptr			  = gbitmap_create_blank(GSize(10, 20), GBitmapFormat8Bit);
		gbitmap_set_data(ptr, bitmap_storage[i], GBitmapFormat8Bit, 10, false);
		atlas[i] = ptr;
	}
}

static void modulate_font_atlas(GBitmap *target[26], GColor background, GColor foreground) {
	for (int i = 0; i < 26; i++) {
		GColor8 *bitmap = (GColor8 *)gbitmap_get_data(target[i]);
		for (int y = 0; y < 20; y++) {
			for (int x = 0; x < 10; x++) {
				GColor8 pixel	   = (GColor8)(bitmap[y * 10 + x]);
				uint8_t brightness = pixel.r;
				bitmap[y * 10 + x] = GColorFromRGBA(foreground.r << 6, foreground.g << 6, foreground.b << 6, brightness << 6);
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
	word_time.hour			   = POSMOD(hour_24 - 1, 12) + 1;
	word_time.offset_direction = minute > 34 ? OFFSET_DIRECTION_TO : OFFSET_DIRECTION_PAST;
	word_time.minute_offset	   = get_minute_offset(word_time.offset_direction == OFFSET_DIRECTION_TO ? 64 - minute : minute);
	if (word_time.offset_direction == OFFSET_DIRECTION_TO) {
		word_time.hour = (word_time.hour + 1) % 12;
	}
	if (word_time.minute_offset == ZERO) {
		word_time.offset_direction = OFFSET_DIRECTION_NONE;
	}
}

void on_time_changed(struct tm *tick_time, TimeUnits units_changed) {
	parse_word_time(tick_time->tm_hour, tick_time->tm_min);
	app_timer_register(50, animation_callback, NULL);
	animation_y = 0;
	animation_ticks++;
	layer_mark_dirty(main_layer);
}

static void main_layer_draw(Layer *layer, GContext *ctx) {
	int y				   = 6;
	bool lit			   = true;
	GColor last_text_color = GColorWhite;
	graphics_context_set_compositing_mode(ctx, GCompOpSet);
	for (size_t i = 0; i < 8; i++) {
		int x = 6;
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
					switch (j) {
						X(7, IS_MINUTE(HALF));
					}
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
			bool really_lit		= lit;
			bool is_placeholder = ch == '-';
			if (is_placeholder) {
				int seed = animation_ticks;
				if (animation_y != -1 && (int)i > animation_y) {
					seed--;
				}
				ch		   = pick_random_char(i, j, seed);
				really_lit = false;
			}
			graphics_draw_bitmap_in_rect(ctx, (really_lit ? font_atlas : faint_font_atlas)[ch - 'a'], GRect(x, y, 10, 20));
			x += 11;
		}
		y += 20;
	}
}

#define LOAD_ATLAS(num) base_font_atlas[num] = gbitmap_create_with_resource(RESOURCE_ID_IOSEVKA_ATLAS_##num)

static void on_window_load(Window *window) {
	Layer *window_layer = window_get_root_layer(window);
	GRect bounds		= layer_get_bounds(window_layer);

	window_set_background_color(window, GColorBlack);

	LOAD_ATLAS(0);
	LOAD_ATLAS(1);
	LOAD_ATLAS(2);
	LOAD_ATLAS(3);
	LOAD_ATLAS(4);
	LOAD_ATLAS(5);
	LOAD_ATLAS(6);
	LOAD_ATLAS(7);
	LOAD_ATLAS(8);
	LOAD_ATLAS(9);
	LOAD_ATLAS(10);
	LOAD_ATLAS(11);
	LOAD_ATLAS(12);
	LOAD_ATLAS(13);
	LOAD_ATLAS(14);
	LOAD_ATLAS(15);
	LOAD_ATLAS(16);
	LOAD_ATLAS(17);
	LOAD_ATLAS(18);
	LOAD_ATLAS(19);
	LOAD_ATLAS(20);
	LOAD_ATLAS(21);
	LOAD_ATLAS(22);
	LOAD_ATLAS(23);
	LOAD_ATLAS(24);
	LOAD_ATLAS(25);

	load_font_atlases(faint_font_atlas, bitmaps[0]);
	load_font_atlases(font_atlas, bitmaps[1]);
	modulate_font_atlas(faint_font_atlas, GColorBlack, GColorDarkGray);
	modulate_font_atlas(font_atlas, GColorBlack, GColorYellow);

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
