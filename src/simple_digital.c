#include <pebble.h>

#define LENGTH 12
#define WIDTH 3
#define TAIL true

// Declare the window and layer for the watchface
Window *window;
Layer *watchface_layer;

void draw_digit(GContext *ctx, GPoint number_origin, int digit);
void draw_segment(GContext *ctx, bool is_on, GPoint origin, const GPathInfo *path_info);
static const bool ILLUMINATION_TABLE[10][7];
static const GPathInfo VERTICAL_CELL;
static const GPathInfo HORIZONTAL_CELL;

// Function to update the watchface
void watchface_update_proc(Layer *layer, GContext *ctx) {
    // Get the current time
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    
    // Get current hour and minute
    int hour = t->tm_hour;
    int minute = t->tm_min;
    
    // Center the digits on the screen based on the size and width of the watch
    GRect bounds = layer_get_bounds(layer);
    int width = bounds.size.w;  // Width of the layer (screen)
    int height = bounds.size.h; // Height of the layer (screen)

    // Set the background color (e.g., to black)
    graphics_context_set_fill_color(ctx, GColorBlack);  // Set your desired background color here
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);  // Fill the entire screen with the background color
    
    
    // Calculate the positions for drawing the segments (centered)
    int x = (width - (LENGTH * 3)) / 2;
    int y = (height - LENGTH) / 2;
    
    // Draw digits (for now, just one for demonstration)
    draw_digit(ctx, GPoint(x, y), hour / 10);  // Tens place of hour
    draw_digit(ctx, GPoint(x + 15, y), hour % 10);  // Ones place of hour
    draw_digit(ctx, GPoint(x + 30, y), minute / 10);  // Tens place of minute
    draw_digit(ctx, GPoint(x + 45, y), minute % 10);  // Ones place of minute
}

static void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
    layer_mark_dirty(window_get_root_layer(window));
  }

// window load function to initialize the watchface
void window_load(Window *window) {
    // get info for window size
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // construct the layer and set up its update proceedures
    watchface_layer = layer_create(bounds);
    layer_set_update_proc(watchface_layer, watchface_update_proc);
    layer_add_child(window_get_root_layer(window), watchface_layer);
}

// Window unload function to clean up
void window_unload(Window *window) {
    layer_destroy(watchface_layer);
}

static void init() {
    window = window_create();
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
    window_stack_push(window, true);

    tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
} 

static void deinit() {
    window_destroy(window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}

static const GPathInfo HORIZONTAL_CELL = {
    6, (GPoint []){
      {1, 0},
      {11, 0},
      {12, -1},
      {11, -2},
      {1, -2},
      {0, -1}
    }
  };

static const GPathInfo VERTICAL_CELL = {
    6, (GPoint []){
      {2, 0},
      {4, -2},
      {4, -10},
      {2, -12},
      {0, -10},
      {0, -2}
    }
  };

 static const bool ILLUMINATION_TABLE[10][7] = {
   //a, b, c, d, e, f, g 
    {1, 1, 1, 1, 1, 1, 0},   // 0
    {0, 1, 1, 0, 0, 0, 0},   // 1          aaaa
    {1, 1, 0, 1, 1, 0, 1},   // 2         f    b
    {1, 1, 1, 1, 0, 0, 1},   // 3         f    b
    {0, 1, 1, 1, 0, 1, 1},   // 4          gggg
    {1, 0, 1, 1, 0, 1, 1},   // 5         e    c
    {1, 0, 1, 1, 1, 1, 1},   // 6         e    c
    {1, 1, 1, 0, 0, 0, 0},   // 7          dddd 
    {1, 1, 1, 1, 1, 1, 1},   // 8
    {1, 1, 1, 1, 0, 1, 1},   // 9
};

void draw_segment(GContext *ctx, bool is_on, GPoint origin, const GPathInfo *path_info) {
    if (is_on) {
        GPath *path = gpath_create(path_info);  // Create the path for the segment
        gpath_move_to(path, origin);  // Position the segment at the correct point
        graphics_context_set_fill_color(ctx, GColorWhite);  // Set the fill color
        gpath_draw_filled(ctx, path);  // Draw the filled path (illuminated)
        gpath_destroy(path);  // Don't forget to destroy the path when done
    }
}

// Function to draw a digit at a given position
void draw_digit(GContext *ctx, GPoint number_origin, int digit) {
    // Iterate through the 7 segments (a, b, c, d, e, f, g)
    for (int segment = 0; segment < 7; segment++) {
        bool is_on = ILLUMINATION_TABLE[digit][segment];

        GPoint origin = number_origin;

        // Adjust the position for each segment (you can tune these based on your layout)
        switch (segment) {
            case 0:  // 'a' (horizontal)
                draw_segment(ctx, is_on, origin, &HORIZONTAL_CELL);
                break;
            case 1:  // 'b' (vertical)
                origin.x += 15;
                draw_segment(ctx, is_on, origin, &VERTICAL_CELL);
                break;
            case 2:  // 'c' (vertical)
                origin.x += 15;
                origin.y += 15;
                draw_segment(ctx, is_on, origin, &VERTICAL_CELL);
                break;
            case 3:  // 'd' (horizontal)
                origin.y += 30;
                draw_segment(ctx, is_on, origin, &HORIZONTAL_CELL);
                break;
            case 4:  // 'e' (vertical)
                origin.y += 15;
                draw_segment(ctx, is_on, origin, &VERTICAL_CELL);
                break;
            case 5:  // 'f' (vertical)
                origin.x += 15;
                origin.y += 15;
                draw_segment(ctx, is_on, origin, &VERTICAL_CELL);
                break;
            case 6:  // 'g' (horizontal)
                origin.y += 20;
                draw_segment(ctx, is_on, origin, &HORIZONTAL_CELL);
                break;
        }
    }
}