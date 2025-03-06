#include <pebble.h>

#define LENGTH 12
#define WIDTH 3
#define TAIL true

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
      {1, 0},
      {2, -1},
      {2, -11},
      {1, -12},
      {0, -11},
      {0, -1}
    }
  };
  
  static const bool ILLUMINATION_TABLE[10][7] = {
    //   a,    b,   c,     d,    e,    f,    g 
    { true, true, true, true, true, true,false},   // 0
    {false, true, true,false,false,false,false},   // 1          aaaa
    { true, true,false, true, true,false, true},   // 2         f    b
    { true, true, true, true,false,false, true},   // 3         f    b
    {false, true, true,false,false, true, true},   // 4          gggg
    { true,false, true, true,false, true, true},   // 5         e    c
    { TAIL,false, true, true, true, true, true},   // 6         e    c
    { true, true, true,false,false,false,false},   // 7          dddd 
    { true, true, true, true, true, true, true},   // 8
    { true, true, true, TAIL,false, true, true},   // 9
  };

// Declare the window and layer for the watchface
Window *window;
Layer *watchface_layer;

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
void draw_digit(GContext *ctx, GPoint origin, int digit) {
    // Iterate through the 7 segments (a, b, c, d, e, f, g)
    for (int segment = 0; segment < 7; segment++) {
        bool is_on = ILLUMINATION_TABLE[digit][segment];

        // Adjust the position for each segment (you can tune these based on your layout)
        switch (segment) {
            case 0:  // 'a' (horizontal)
                origin.y -= 10; // Move up for 'a'
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
                origin.x += 15;
                origin.y += 30;
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
    
    // Calculate the positions for drawing the segments (centered)
    int x = (width - (LENGTH * 3)) / 2;
    int y = (height - LENGTH) / 2;
    
    // Draw digits (for now, just one for demonstration)
    draw_segment(ctx, GPoint(x, y), hour / 10);  // Tens place of hour
    draw_segment(ctx, GPoint(x + 15, y), hour % 10);  // Ones place of hour
    draw_segment(ctx, GPoint(x + 30, y), minute / 10);  // Tens place of minute
    draw_segment(ctx, GPoint(x + 45, y), minute % 10);  // Ones place of minute
}

// Window load function to initialize the watchface
void window_load(Window *window) {
    GRect bounds = layer_get_bounds(layer);
    watchface_layer = layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));  // Full screen size
    layer_set_update_proc(watchface_layer, watchface_update_proc);
    layer_add_child(window_get_root_layer(window), watchface_layer);
}

// Window unload function to clean up
void window_unload(Window *window) {
    layer_destroy(watchface_layer);
}

int main(void) {
    window = window_create();
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
    window_stack_push(window, true);

    app_event_loop();
    window_destroy(window);
}