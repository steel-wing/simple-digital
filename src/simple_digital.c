#include <pebble.h>

#define LENGTH 12 
#define WIDTH 3     // has to be odd in order to look good. Unfortunately
#define SPACING 1
#define T true      // the tail on the 6 and the 9

// Declare the window and layer for the watchface
Window *window;
Layer *watchface_layer;

void draw_digit(GContext *ctx, GPoint number_origin, int digit);
void draw_segment(GContext *ctx, bool is_on, GPoint origin, const GPathInfo *path_info);
int number_width(int length, int width, int spacing, int digit);
int number_height(int length, int width, int spacing, int digit);

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
    graphics_context_set_fill_color(ctx, GColorWhite);  // Set your desired background color here
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);  // Fill the entire screen with the background color

    // dynamic adjustment code here
    int standard_width = number_width(LENGTH, WIDTH, SPACING, 0);
    int standard_height = number_height(LENGTH, WIDTH, SPACING, 0);

    // Calculate the positions for drawing the segments (centered)
    int x = (width - 2 * standard_width + 1) / 2;
    int y = (height - standard_height + 1) / 2;

    // Draw digits (for now, just one for demonstration)
    GPoint drawpoint = GPoint(x, y);

    draw_digit(ctx, drawpoint, hour / 10);  // Tens place of hour
    drawpoint.x += standard_width + width;

    draw_digit(ctx, drawpoint, hour % 10);  // Ones place of hour
    drawpoint.x += standard_width + 3 * width;

    draw_digit(ctx, drawpoint, minute / 10);  // Tens place of minute
    drawpoint.x += standard_width + width;

    draw_digit(ctx, drawpoint, minute % 10);  // Ones place of minute
    drawpoint.x += standard_width + width;
}

// clear out the stuff for time reception? not really sure about this one
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
        {((WIDTH - 1) / 2) - 1, 0},
        {(LENGTH - (WIDTH - 1) / 2), 0},
        {LENGTH, ((WIDTH - 1) / 2)},
        {(LENGTH - (WIDTH - 1) / 2), (WIDTH - 1)},
        {((WIDTH - 1) / 2) - 1, (WIDTH - 1)},
        {0 - 1, ((WIDTH - 1) / 2)}
    }
  };

static const GPathInfo VERTICAL_CELL = {
    6, (GPoint []){
        {((WIDTH - 1) / 2), -1},
        {(WIDTH), ((WIDTH - 1) / 2)},
        {(WIDTH), (LENGTH - (WIDTH - 1) / 2) - 1},
        {((WIDTH - 1) / 2), LENGTH},
        {0 - 1, (LENGTH - (WIDTH - 1) / 2) - 1},
        {0 - 1, ((WIDTH - 1) / 2)}
    }
  };

static const bool ILLUMINATION_TABLE[10][7] = {
//   a, b, c, d, e, f, g 
    {1, 1, 1, 1, 1, 1, 0},   // 0
    {0, 1, 1, 0, 0, 0, 0},   // 1          aaaa
    {1, 1, 0, 1, 1, 0, 1},   // 2         f    b
    {1, 1, 1, 1, 0, 0, 1},   // 3         f    b
    {0, 1, 1, 0, 0, 1, 1},   // 4          gggg
    {1, 0, 1, 1, 0, 1, 1},   // 5         e    c
    {T, 0, 1, 1, 1, 1, 1},   // 6         e    c
    {1, 1, 1, 0, 0, 0, 0},   // 7          dddd 
    {1, 1, 1, 1, 1, 1, 1},   // 8
    {1, 1, 1, T, 0, 1, 1},   // 9
};

void draw_segment(GContext *ctx, bool is_on, GPoint origin, const GPathInfo *path_info) {
    if (is_on) {
        GPath *path = gpath_create(path_info);  // Create the path for the segment
        gpath_move_to(path, origin);  // Position the segment at the correct point
        graphics_context_set_fill_color(ctx, GColorBlack);  // Set the fill color
        gpath_draw_filled(ctx, path);  // Draw the filled path (illuminated)
        gpath_destroy(path);  // Don't forget to destroy the path when done
    }
}

// Function to draw a digit at a given position
void draw_digit(GContext *ctx, GPoint number_origin, int digit) {
    // Iterate through the 7 segments (a, b, c, d, e, f, g)
    for (int segment = 0; segment < 7; segment++) {
        bool is_on = ILLUMINATION_TABLE[digit][segment];

        // make a copy so we don't ruin anything
        GPoint origin = number_origin;

        // Adjust the position for each segment (you can tune these based on your layout)
        switch (segment) {
            case 0:  // 'a' (horizontal)
                origin.x += WIDTH / 2 + SPACING;
                draw_segment(ctx, is_on, origin, &HORIZONTAL_CELL);
                break;
            case 1:  // 'b' (vertical)
                origin.x += (LENGTH - 1) + 2 * SPACING;
                origin.y += WIDTH / 2 + SPACING;
                draw_segment(ctx, is_on, origin, &VERTICAL_CELL);
                break;
            case 2:  // 'c' (vertical)
                origin.x += (LENGTH - 1) + 2 * SPACING;
                origin.y += (LENGTH - 1) + 3 * SPACING + WIDTH / 2;
                draw_segment(ctx, is_on, origin, &VERTICAL_CELL);
                break;
            case 3:  // 'd' (horizontal)
                origin.x += WIDTH / 2 + SPACING;
                origin.y += 2 * (LENGTH - 1) + 4 * SPACING;
                draw_segment(ctx, is_on, origin, &HORIZONTAL_CELL);
                break;
            case 4:  // 'e' (vertical)
                origin.y += (LENGTH - 1) + 3 * SPACING + WIDTH / 2;
                draw_segment(ctx, is_on, origin, &VERTICAL_CELL);
                break;
            case 5:  // 'f' (vertical)
                origin.y += WIDTH / 2 + SPACING;
                draw_segment(ctx, is_on, origin, &VERTICAL_CELL);
                break;
            case 6:  // 'g' (horizontal)
                origin.x += WIDTH / 2 + SPACING;
                origin.y += (LENGTH - 1) + 2 * SPACING;
                draw_segment(ctx, is_on, origin, &HORIZONTAL_CELL);
                break;
        }
    }
}

int number_width(int length, int width, int spacing, int digit) {
    if (digit == 1) {
        return width;
    } else if (digit == 3 || digit == 7) {
        return length + spacing + width / 2;
    } else {
        return length + 2 * (spacing + width / 2);
    }
}

int number_height(int length, int width, int spacing, int digit) {
    return 2 * length + 3 * width + 2 * width + 4 * spacing;
}
