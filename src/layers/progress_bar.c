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
    bar->bar_colour = GColorRed;

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
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 5, 0, DEG_TO_TRIGANGLE(360));

    int percentage = (((bar->value)*100)/bar->max);
    graphics_context_set_fill_color(ctx, bar->bar_colour);
    graphics_fill_radial(ctx, bounds, GOvalScaleModeFitCircle, 5, 0, DEG_TO_TRIGANGLE(percentage*3.6));
    #else
    bounds.size.w = ((bar->value - bar->min) * bounds.size.w) / (bar->max - bar->min);
    graphics_context_set_fill_color(ctx, bar->bar_colour);
    graphics_fill_rect(ctx, GRect(bounds.origin.x, 166, bounds.size.w, 3), 0, GCornerNone);
    #endif
}
