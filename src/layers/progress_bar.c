#include <lignite_music.h>

static void progress_bar_update_proc(Layer* layer, GContext *context);

ProgressBarLayer *progress_bar_layer_create(GRect frame) {
    ProgressBarLayer *bar = malloc(sizeof(ProgressBarLayer));

    bar->layer = layer_create_with_data(frame, sizeof(ProgressBarData));

    ProgressBarData *data = layer_get_data(bar->layer);
    data->progress_bar = bar;

    layer_set_update_proc(bar->layer, progress_bar_update_proc);

    bar->min = 0;
    bar->max = 255;
    bar->value = 0;
    bar->bar_colour = PBL_IF_COLOR_ELSE(GColorRed, GColorWhite);

    return bar;
}

void progress_bar_layer_destroy(ProgressBarLayer *bar){
    layer_destroy(bar->layer);
    free(bar);
}

Layer *progress_bar_layer_get_layer(ProgressBarLayer *bar){
    return bar->layer;
}

void progress_bar_layer_set_range(ProgressBarLayer* bar, int32_t min, int32_t max) {
    bar->max = max;
    bar->min = min;
    if(bar->layer){
        layer_mark_dirty(bar->layer);
    }
}

void progress_bar_layer_set_value(ProgressBarLayer* bar, int32_t value) {
    bar->value = value;
    if(bar->layer){
        layer_mark_dirty(bar->layer);
    }
}

static void progress_bar_update_proc(Layer* layer, GContext *ctx) {
    ProgressBarLayer* bar = ((ProgressBarData*)layer_get_data(layer))->progress_bar;

    GRect bounds = layer_get_bounds(layer);
    #ifdef PBL_ROUND
    GRect layer_frame = layer_get_frame(layer);
    int max_width = 60;
	int x_offset = (layer_frame.size.w-max_width)/2;
	int battery_bar_width = (bar->value*max_width)/bar->max;
	int actual_battery_bar_height = 6;
	graphics_context_set_fill_color(ctx, GColorDarkGray);
	graphics_fill_rect(ctx, GRect(x_offset, layer_frame.size.h-(actual_battery_bar_height*3)-4, max_width, actual_battery_bar_height), 2, GCornersAll);
	graphics_context_set_fill_color(ctx, GColorRed);
	graphics_fill_rect(ctx, GRect(x_offset, layer_frame.size.h-(actual_battery_bar_height*3)-4, battery_bar_width, actual_battery_bar_height), 2, GCornersAll);
    #else
    bounds.size.w = ((bar->value - bar->min) * bounds.size.w) / (bar->max - bar->min);
    graphics_context_set_fill_color(ctx, bar->bar_colour);
    graphics_fill_rect(ctx, GRect(bounds.origin.x, 166, bounds.size.w, 3), 0, GCornerNone);
    #endif
}
