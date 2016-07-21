#include "common.h"
#include "progress_bar.h"

static void update_proc(Layer* layer, GContext *context);

void progress_bar_layer_init(ProgressBarLayer* bar, GRect frame) {
    bar->layer = layer_create_with_data(frame, sizeof(ProgressBarData));

    ProgressBarData *data = layer_get_data(bar->layer);
    data->progress_bar = bar;

    layer_set_update_proc(bar->layer, update_proc);

    bar->min = 0;
    bar->max = 255;
    bar->value = 0;
    bar->frame_colour = GColorBlack;
    bar->bar_colour = GColorBlack;
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

static void update_proc(Layer* layer, GContext *context) {
    NSLog("Updating");
    ProgressBarLayer* bar = ((ProgressBarData*)layer_get_data(layer))->progress_bar;
    GRect bounds = layer_get_bounds(layer);
    graphics_draw_round_rect(context, bounds, 3);

    bounds.size.w = ((bar->value - bar->min) * bounds.size.w) / (bar->max - bar->min);
    GCornerMask corners = GCornersLeft;
    if(bounds.size.w >= layer_get_bounds(layer).size.w - 4) {
        corners |= GCornersRight;
    }
    graphics_context_set_fill_color(context, bar->bar_colour);
    graphics_fill_rect(context, bounds, 3, corners);
}
