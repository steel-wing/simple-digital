#include <pebble.h>

#define LENGTH 12 
#define WIDTH 3     // has to be odd in order to look good, unfortunately
#define GAP 4
#define SPACING 1
#define T true      // the tail on the 6 and the 9
#define BACKGROUND GColorWhite
#define FOREGROUND GColorBlack

// important variables for below
Window *window;
Layer *watchface_layer;

static const bool ILLUMINATION_TABLE[10][7];
static const GPathInfo VERTICAL_CELL;
static const GPathInfo HORIZONTAL_CELL;


GRect window_get_unobstructed_area(Window *win);

// draws a single segment
void draw_segment(GContext *ctx, bool is_on, GPoint origin, const GPathInfo *path_info) {
    // only draw this if we should
    if (is_on) {
        // generate a path and and move it's origin to the origin point given
        GPath *path = gpath_create(path_info);
        gpath_move_to(path, origin);

        // make the fill color black (we aren't doing a stroke here)
        graphics_context_set_fill_color(ctx, FOREGROUND);

        // actually draw the path with the points provided
        gpath_draw_filled(ctx, path);

        // destroy the path in memory to avoid leaks
        gpath_destroy(path);
    }
}

// draws a single digit
void draw_digit(GContext *ctx, GPoint number_origin, int digit) {
    // iterate across all 7 segments
    for (int segment = 0; segment < 7; segment++) {
        bool is_on = ILLUMINATION_TABLE[digit][segment];

        // make a copy so we don't ruin anything
        GPoint origin = number_origin;

        // go through each segment and draw it in the right location
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

// function for calculating the width of a number based on parameters 
int number_width(int length, int width, int spacing, int digit) {
    if (digit == 1) {
        return width;
    } else if (digit == 3 || digit == 7) {
        return length + spacing + (width - 1) / 2;
    } else {
        return length + 2 * (spacing + (width - 1) / 2);
    }
}

// function for calculating the height of a number based on parameters 
int number_height(int length, int width, int spacing, int digit) {
    return 2 * length + 2 * ((width - 1) / 2) + 4 * spacing;
}

// update the watchface (runs every time-> call)
void watchface_update_proc(Layer *layer, GContext *ctx) {
    // get current time
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
   // bool military = clock_is_24h_style();
    
    // break it down
    int h1 = t->tm_hour / 10;
    int h2 = t->tm_hour % 10;
    int m1 = t->tm_min / 10;
    int m2 = t->tm_min % 10;
    
    // get screen dimensions (make this reactive to alerts)
    GRect bounds = layer_get_unobstructed_bounds(layer);
    int width = bounds.size.w;
    int height = bounds.size.h;

    // set background color)
    graphics_context_set_fill_color(ctx, BACKGROUND);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);

    // get number dimensions
    int num_w = number_width(LENGTH, WIDTH, SPACING, 0);
    int num_h = number_height(LENGTH, WIDTH, SPACING, 0);

    // find point to begin drawing from (currently assuming 00:00)
    int x = width / 2 - (4 * (num_w + GAP) + WIDTH) / 2; // half of: screen minus (4 numbers, 4 gaps, and a colon (WIDTH)
    int y = height / 2 - (num_h) / 2;


    // handle military time and leading digits
    // if (military) {
        GPoint drawpoint = GPoint(x, y);


    draw_digit(ctx, drawpoint, h1); //hour / 10);
    drawpoint.x += num_w + GAP;

    draw_digit(ctx, drawpoint, h2); //hour % 10);
    drawpoint.x += num_w + 2 * GAP + WIDTH;

    draw_digit(ctx, drawpoint, m1); //minute / 10);
    drawpoint.x += num_w + GAP;

    draw_digit(ctx, drawpoint, m2); //minute % 10);
    drawpoint.x += num_w + GAP;
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

// init() to handle everything that has to get done at the startt
static void init() {
    window = window_create();
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
    window_stack_push(window, true);

    // subscribe us to the minute service (could make this seconds later)
    tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
} 

// just destroys the window since we already handled the paths
static void deinit() {
    window_destroy(window);
}

// gotta love best practice
int main(void) {
    init();
    app_event_loop();
    deinit();
}

// path for a horizontal cell (dynamic to the #define's above)
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

// path for a vertical cell (dynamic to the #define's above)
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

// how we decide which cells to illuminate for which digits
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