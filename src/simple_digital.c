#include <pebble.h>

#define LENGTH 12   // length of a cell, tip to tip
#define WIDTH 3     // has to be odd in order to look good, unfortunately
#define GAP 4       // gap between numbers
#define SPACING 1   // gap between cells (taxicab)
#define S 1         // the tail on the 6
#define N 1         // the tail on the 9
#define BACKGROUND GColorBlack
#define FOREGROUND GColorWhite

// this may be bad practice, but I just wanted the big statics at the bottom, okay?
static const bool ILLUMINATION_TABLE[10][7];
static const GPathInfo COLON_CELL;
static const GPathInfo VERTICAL_CELL;
static const GPathInfo HORIZONTAL_CELL;

// important variables for below
Window *window;
Layer *watchface_layer;
GRect window_get_unobstructed_area(Window *win);

// draws a single cell
static void draw_cell(GContext *ctx, bool active, GPoint origin, const GPathInfo *path_info) {
    // only draw this if we should
    if (active) {
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

// draws a single digit. GPoint is top left corner of box
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
                draw_cell(ctx, is_on, origin, &HORIZONTAL_CELL);
                break;
            case 1:  // 'b' (vertical)
                origin.x += (LENGTH - 1) + 2 * SPACING;
                origin.y += WIDTH / 2 + SPACING;
                draw_cell(ctx, is_on, origin, &VERTICAL_CELL);
                break;
            case 2:  // 'c' (vertical)
                origin.x += (LENGTH - 1) + 2 * SPACING;
                origin.y += (LENGTH - 1) + 3 * SPACING + WIDTH / 2;
                draw_cell(ctx, is_on, origin, &VERTICAL_CELL);
                break;
            case 3:  // 'd' (horizontal)
                origin.x += WIDTH / 2 + SPACING;
                origin.y += 2 * (LENGTH - 1) + 4 * SPACING;
                draw_cell(ctx, is_on, origin, &HORIZONTAL_CELL);
                break;
            case 4:  // 'e' (vertical)
                origin.y += (LENGTH - 1) + 3 * SPACING + WIDTH / 2;
                draw_cell(ctx, is_on, origin, &VERTICAL_CELL);
                break;
            case 5:  // 'f' (vertical)
                origin.y += WIDTH / 2 + SPACING;
                draw_cell(ctx, is_on, origin, &VERTICAL_CELL);
                break;
            case 6:  // 'g' (horizontal)
                origin.x += WIDTH / 2 + SPACING;
                origin.y += (LENGTH - 1) + 2 * SPACING;
                draw_cell(ctx, is_on, origin, &HORIZONTAL_CELL);
                break;
        }
    }
}

// draws the colon. GPoint is top left corner, if it were number height
void draw_colon(GContext *ctx, GPoint colon_origin) {
    bool illuminate = true;

    // make a copy so we don't ruin anything
    GPoint origin = colon_origin;

    // little bit of math to center the dots the way I want
    origin.y += WIDTH / 2 + SPACING + LENGTH / 2 - 1;

    draw_cell(ctx, illuminate, origin, &COLON_CELL);
    origin.y += WIDTH + LENGTH - 2 * SPACING - 1;

    draw_cell(ctx, illuminate, origin, &COLON_CELL);
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
void watchface_update(Layer *layer, GContext *ctx) {
    // get current time
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    bool military = clock_is_24h_style();

    // get the right hour digits
    if (!military) {
        // 13:00 - 23:00 -> 1:00 - 11:00
        if (t->tm_hour > 12) {
            t->tm_hour -= 12;
        // 0:00 -> 12:00
        } else if (t->tm_hour == 0) {
            t->tm_hour += 12;
        }
    }
    
    // break it down
    int h1 = t->tm_hour / 10;
    int h2 = t->tm_hour % 10;
    int m1 = t->tm_min / 10;
    int m2 = t->tm_min % 10;
    
    // get screen dimensions (make this reactive to alerts)
    GRect bounds = layer_get_unobstructed_bounds(layer);
    int width = bounds.size.w;
    int height = bounds.size.h;

    // set background color
    graphics_context_set_fill_color(ctx, BACKGROUND);
    graphics_fill_rect(ctx, bounds, 0, GCornerNone);

    // figure out initial spacing
    int num_w = number_width(LENGTH, WIDTH, SPACING, 0);
    int num_h = number_height(LENGTH, WIDTH, SPACING, 0);
    int first_w;

    if (!military && h1 == 0) {
        first_w = number_width(LENGTH, WIDTH, SPACING, h2);
    } else {
        first_w = number_width(LENGTH, WIDTH, SPACING, h1);
    }

    // hh:mm assumption
    int exact_w = first_w + 3 * num_w + WIDTH + 4 * GAP;
    int correction = num_w - first_w;

    int x = (width - exact_w) / 2 - correction + 1;
    int y = height / 2 - num_h / 2;

    // set start point for four digits
    GPoint drawpoint = GPoint(x, y);

    // handle two digit hours
    if (military || h1 != 0) {
        draw_digit(ctx, drawpoint, h1);
        drawpoint.x += num_w + GAP;
        
    // handle single-digit hours
    } else {
        drawpoint.x += (num_w + GAP) / 2;
    }

    // write the rest of the time
    draw_digit(ctx, drawpoint, h2);
    drawpoint.x += num_w + GAP;

    draw_colon(ctx, drawpoint);
    drawpoint.x += WIDTH + GAP;

    draw_digit(ctx, drawpoint, m1);
    drawpoint.x += num_w + GAP;

    draw_digit(ctx, drawpoint, m2);
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
    layer_set_update_proc(watchface_layer, watchface_update);
    layer_add_child(window_get_root_layer(window), watchface_layer);
}

// Window unload function to clean up
void window_unload(Window *window) {
    layer_destroy(watchface_layer);
}

// init() to handle everything that has to get done at the startt
static void init() {
    // construct window and get it into position
    window = window_create();
    window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
    });
    window_stack_push(window, true);

    // subscribe us to the minute service
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

// path for single colon cell
static const GPathInfo COLON_CELL = {
    4, (GPoint []){
        {((WIDTH - 1) / 2), 0 - 1},
        {(WIDTH), ((WIDTH - 1) / 2)},
        {((WIDTH - 1) / 2), WIDTH},
        {0 - 1, ((WIDTH - 1) / 2)}
    }
  };

// path for a horizontal cell
static const GPathInfo HORIZONTAL_CELL = {
    // arcane math to counter pebble's awful line drawing script
    6, (GPoint []){
        {((WIDTH - 1) / 2) - 1, 0},
        {(LENGTH - (WIDTH - 1) / 2), 0},
        {LENGTH, ((WIDTH - 1) / 2)},
        {(LENGTH - (WIDTH - 1) / 2), (WIDTH - 1)},
        {((WIDTH - 1) / 2) - 1, (WIDTH - 1)},
        {0 - 1, ((WIDTH - 1) / 2)}
    }
  };

// path for a vertical cell
static const GPathInfo VERTICAL_CELL = {
    // arcane math to counter pebble's awful line drawing script
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
// yes I know these are the wrong datatypes but this looks better

//   a, b, c, d, e, f, g 
    {1, 1, 1, 1, 1, 1, 0},   // 0
    {0, 1, 1, 0, 0, 0, 0},   // 1          aaaa
    {1, 1, 0, 1, 1, 0, 1},   // 2         f    b
    {1, 1, 1, 1, 0, 0, 1},   // 3         f    b
    {0, 1, 1, 0, 0, 1, 1},   // 4          gggg
    {1, 0, 1, 1, 0, 1, 1},   // 5         e    c
    {S, 0, 1, 1, 1, 1, 1},   // 6         e    c
    {1, 1, 1, 0, 0, 0, 0},   // 7          dddd 
    {1, 1, 1, 1, 1, 1, 1},   // 8
    {1, 1, 1, N, 0, 1, 1},   // 9
};