#include <pebble.h>

#define SETTINGS_KEY 1

typedef struct {
	GColor bg_color;
	GColor faint_color;
	GColor bright_color;
	bool faint_bold;
	bool bright_bold;
} ClaySettings;

static ClaySettings settings;

static void default_settings() {
	settings.bg_color	  = GColorBlack;
	settings.faint_color  = GColorDarkGray;
	settings.bright_color = GColorInchworm;

	settings.faint_bold	 = false;
	settings.bright_bold = true;
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

	save_settings();
	settings_changed();
}
