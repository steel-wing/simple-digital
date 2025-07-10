#pragma once
#include <pebble.h>

#define SETTINGS_KEY 1

// A structure containing our settings
typedef struct ClaySettings {
  GColor BackgroundColor;
  GColor ForegroundColor;
} __attribute__((__packed__)) ClaySettings;

// static void config_default();
// static void config_load();
// static void config_save();
// static void config_update_display();
// static void prv_inbox_received_handler(DictionaryIterator *iter, void *context);
// static void window_load(Window *window);
// static void window_unload(Window *window);
// static void init(void);
// static void deinit(void);