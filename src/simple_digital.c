#include <pebble.h>

// Declare the window and layer for the watchface
Window *window;
Layer *watchface_layer;

// Size settings
int segment_size = LENGTH;  // Default size for segments
GColor segment_color = GColorBlack;  // Default segment color
GColor background_color = GColorWhite;  // Default background color

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

        // Get the segment's position based on the segment type and the base origin
        GPoint segment_origin = origin;

        // Adjust the position for each segment (you can tune these based on your layout)
        switch (segment) {
            case 0:  // 'a' (horizontal)
                segment_origin.y -= 10; // Move up for 'a'
                draw_segment(ctx, is_on, segment_origin, &HORIZONTAL_CELL);
                break;
            case 1:  // 'b' (vertical)
                segment_origin.x += 15;
                draw_segment(ctx, is_on, segment_origin, &VERTICAL_CELL);
                break;
            case 2:  // 'c' (vertical)
                segment_origin.x += 15;
                segment_origin.y += 15;
                draw_segment(ctx, is_on, segment_origin, &VERTICAL_CELL);
                break;
            case 3:  // 'd' (horizontal)
                segment_origin.y += 30;
                draw_segment(ctx, is_on, segment_origin, &HORIZONTAL_CELL);
                break;
            case 4:  // 'e' (vertical)
                segment_origin.x += 15;
                segment_origin.y += 30;
                draw_segment(ctx, is_on, segment_origin, &VERTICAL_CELL);
                break;
            case 5:  // 'f' (vertical)
                segment_origin.x += 15;
                segment_origin.y += 15;
                draw_segment(ctx, is_on, segment_origin, &VERTICAL_CELL);
                break;
            case 6:  // 'g' (horizontal)
                segment_origin.y += 20;
                draw_segment(ctx, is_on, segment_origin, &HORIZONTAL_CELL);
                break;
        }
    }
}

void update_proc(Layer *layer, GContext *ctx) {
    time_t now = time(NULL);
    struct tm *tick_time = localtime(&now);

    int hour = tick_time->tm_hour % 12;  // 12-hour format
    int minute = tick_time->tm_min;

    // The origin for each digit (position on screen)
    GPoint positions[4] = {
        {20, 40},  // Position for first digit (tens of hour)
        {50, 40},  // Position for second digit (ones of hour)
        {80, 40},  // Position for first digit (tens of minute)
        {110, 40}, // Position for second digit (ones of minute)
    };

    // Draw each digit
    draw_digit(ctx, positions[0], hour / 10);   // Tens place of hour
    draw_digit(ctx, positions[1], hour % 10);   // Ones place of hour
    draw_digit(ctx, positions[2], minute / 10); // Tens place of minute
    draw_digit(ctx, positions[3], minute % 10); // Ones place of minute
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
    int x = (screen_width - (segment_size * 3)) / 2;
    int y = (screen_height - segment_size) / 2;
    
    // Draw digits (for now, just one for demonstration)
    draw_segment_display(ctx, x, y, hour / 10);  // Tens place of hour
    draw_segment_display(ctx, x + segment_size * 2, y, hour % 10);  // Ones place of hour
    draw_segment_display(ctx, x + segment_size * 4, y, minute / 10);  // Tens place of minute
    draw_segment_display(ctx, x + segment_size * 6, y, minute % 10);  // Ones place of minute
}

// Window load function to initialize the watchface
void window_load(Window *window) {
    watchface_layer = layer_create(GRect(0, 0, 144, 168));  // Full screen size
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
