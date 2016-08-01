#pragma once

#include <lignite_music.h>

typedef struct MarqueeTextLayer {
    Layer *layer;
    const char* text;
    GFont font;
    GColor text_colour;
    GColor background_colour;
    int16_t countdown;
    int16_t text_width;
    int16_t offset;
	struct MarqueeTextLayer* previous;
	struct MarqueeTextLayer* next;
} MarqueeTextLayer;

typedef struct {
    MarqueeTextLayer *marqueeLayer;
} MarqueeTextLayerReference;

/**
 * Fires the marquee layer for updating.
 */
void marquee_text_layer_tick();

/**
 * Marks the layer as dirty, not as in my dad's sense of humour, but ready for
 * redrawing.
 * @param marquee The MarqueeTextLayer to update
 */
void marquee_text_layer_mark_dirty(MarqueeTextLayer *marquee);

/**
 * Creates a MarqueeTextLayer on a frame.
 * @param frame   The frame to set the MarqueeTextLayer on.
 * @return        The created MarqueeTextLayer.
 */
MarqueeTextLayer *marquee_text_layer_create(GRect frame);

/**
 * Destroys a MarqueeTextLayer.
 * @param marquee The MarqueeTextLayer pointer to destroy.
 */
void marquee_text_layer_destroy(MarqueeTextLayer *marquee);

/**
 * Gets the root layer of the MarqueeTextLayer, the layer that's actually drawn on.
 * This is more of a convenience function to fit the standards of the Pebble API.
 * @param  marquee The MarqueeTextLayer to get the root layer of.
 * @return         The root layer.
 */
Layer *marquee_text_layer_get_layer(MarqueeTextLayer *marquee);

/**
 * Sets the text of the MarqueeTextLayer.
 * @param marquee The MarqueeTextLayer to set the text onto.
 * @param text    The text that will be set onto the layer.
 */
void marquee_text_layer_set_text(MarqueeTextLayer *marquee, const char *text);

/**
 * Sets the GFont of the MarqueeTextLayer.
 * @param marquee The MarqueeTextLayer to set the GFont onto.
 * @param font    The GFont to apply.
 */
void marquee_text_layer_set_font(MarqueeTextLayer *marquee, GFont font);

/**
 * Sets the text colour of the MarqueeTextLayer.
 * @param marquee The MarqueeTextLayer to apply the colour to.
 * @param colour   The colour to apply.
 */
void marquee_text_layer_set_text_color(MarqueeTextLayer *marquee, GColor colour);

/**
 * Sets the background colour of the MarqueeTextLayer.
 * @param marquee The MarqueeTextLayer to set the colour onto.
 * @param colour   The colour to set on.
 */
void marquee_text_layer_set_background_color(MarqueeTextLayer *marquee, GColor colour);