#pragma once

#include <lignite_music.h>

typedef struct {
    Layer *layer;
    int16_t min;
    int16_t max;
    int16_t value;
    GColor bar_colour;
    GColor background_colour;
    GColor frame_colour;
} ProgressBarLayer;

typedef struct {
    ProgressBarLayer *progress_bar;
} ProgressBarData;

/**
 * Creates the ProgressBarLayer with a certain frame, usually the frame of the window.
 * @param frame  The frame to set the ProgressBarLayer onto.
 * @return       The created ProgressBarLayer.
 */
ProgressBarLayer *progress_bar_layer_create(GRect frame);

/**
 * Gets the root layer of the ProgressBarLayer.
 * @param  bar The bar to get the root layer of.
 * @return     The root layer of the bar.
 */
Layer *progress_bar_layer_get_layer(ProgressBarLayer *bar);

/**
 * Destroys a ProgressBarLayer.
 * @param bar The ProgressBarLayer to destroy.
 */
void progress_bar_layer_destroy(ProgressBarLayer *bar);

/**
 * Sets the range of the ProgressBarLayer with a minimum and maximum.
 * @param bar The ProgressBarLayer to modify.
 * @param min The minimum value.
 * @param max The maximum value.
 */
void progress_bar_layer_set_range(ProgressBarLayer* bar, int32_t min, int32_t max);

/**
 * Sets the current value of the ProgressBarLayer, in between the min and max,
 * hopefully.
 * @param bar   The ProgressBarLayer to modify.
 * @param value The new value of the ProgressBarLayer.
 */
void progress_bar_layer_set_value(ProgressBarLayer* bar, int32_t value);