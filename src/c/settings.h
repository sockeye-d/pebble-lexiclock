#include <pebble.h>

#define SETTINGS_KEY 1

#define IS_COLOR PBL_IF_COLOR_ELSE(true, false)

typedef enum {
	DATE_MODE_MDY,
	DATE_MODE_DMY,
	DATE_MODE_YMD,
} DateMode;

typedef struct {
	GColor bg_color;
	GColor faint_color;
	GColor bright_color;
	bool faint_bold;
	bool bright_bold;
	bool british_mode;

	DateMode date_mode;

#if !IS_COLOR
	bool light_on_dark;
	bool use_fainter_dithering;
#endif
} ClaySettings;

static ClaySettings settings;

static void default_settings() {
	settings.bg_color	  = GColorBlack;
	settings.faint_color  = GColorDarkGray;
	settings.bright_color = GColorInchworm;

	settings.faint_bold	 = false;
	settings.bright_bold = true;

	settings.date_mode = DATE_MODE_YMD;

	settings.british_mode = false;

#if !IS_COLOR
	settings.light_on_dark		   = false;
	settings.use_fainter_dithering = false;
#endif
}

static void load_settings() {
	default_settings();
	persist_read_data(SETTINGS_KEY, &settings, sizeof(settings));
}

static void save_settings() {
	persist_write_data(SETTINGS_KEY, &settings, sizeof(settings));
}

#define LOAD_INT(name)                                     \
	do {                                                   \
		Tuple *name = dict_find(iter, MESSAGE_KEY_##name); \
		if (name)                                          \
			settings.name = name->value->int32;            \
	} while (false)

#define LOAD_COLOR(name)                                       \
	do {                                                       \
		Tuple *name = dict_find(iter, MESSAGE_KEY_##name);     \
		if (name)                                              \
			settings.name = GColorFromHEX(name->value->int32); \
	} while (false)

#define LOAD_BOOL(name)                                    \
	do {                                                   \
		Tuple *name = dict_find(iter, MESSAGE_KEY_##name); \
		if (name)                                          \
			settings.name = name->value->int32 == 1;       \
	} while (false)

static void settings_changed();

static void inbox_received_handler(DictionaryIterator *iter, void *context) {
	printf("Settings received!");
	LOAD_COLOR(bg_color);
	LOAD_COLOR(faint_color);
	LOAD_COLOR(bright_color);
	LOAD_BOOL(faint_bold);
	LOAD_BOOL(bright_bold);

	LOAD_INT(date_mode);

	LOAD_BOOL(british_mode);

#if !IS_COLOR
	LOAD_BOOL(light_on_dark);
	LOAD_BOOL(use_fainter_dithering);
#endif

	save_settings();
	settings_changed();
}
