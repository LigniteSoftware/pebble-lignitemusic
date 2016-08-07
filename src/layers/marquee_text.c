#include <lignite_music.h>

static MarqueeTextLayer* head;

void marquee_text_layer_update_proc(Layer* layer, GContext* context);

MarqueeTextLayer *marquee_text_layer_create(GRect frame) {
    MarqueeTextLayer *marquee = malloc(sizeof(MarqueeTextLayer));

    marquee->layer = layer_create_with_data(frame, sizeof(MarqueeTextLayerReference));
    MarqueeTextLayerReference *ref = (MarqueeTextLayerReference*)layer_get_data(marquee->layer);
    ref->marqueeLayer = marquee;

    GRect bounds = layer_get_bounds(marquee->layer);
    layer_set_bounds(marquee->layer, bounds);
    layer_set_update_proc(marquee->layer, marquee_text_layer_update_proc);
    marquee->background_colour = GColorClear;

    marquee->text_colour = GColorWhite;
    marquee->offset = 0;
	marquee->text_width = -1;
    marquee->countdown = 100;
    marquee->font = fonts_get_system_font(FONT_KEY_FONT_FALLBACK);
    marquee_text_layer_mark_dirty(marquee);

	if(head){
		head->next = marquee;
    }
	marquee->previous = head;
	marquee->next = NULL;
	head = marquee;

    return marquee;
}

void marquee_text_layer_destroy(MarqueeTextLayer *marquee) {
	// Remove the marquee from the timer sequence.
	if(marquee == head) {
		head = marquee->previous;
	}
	if(marquee->next) {
		marquee->next->previous = marquee->previous;
	}
	if(marquee->previous) {
		marquee->previous->next = marquee->next;
	}

    layer_destroy(marquee->layer);
    free(marquee);
}

Layer *marquee_text_layer_get_layer(MarqueeTextLayer *marquee){
    return marquee->layer;
}

void marquee_text_layer_set_text(MarqueeTextLayer *marquee, const char *text) {
    marquee->text = text;
    marquee_text_layer_mark_dirty(marquee);
}

void marquee_text_layer_set_font(MarqueeTextLayer *marquee, GFont font) {
    marquee->font = font;
    marquee_text_layer_mark_dirty(marquee);
}

void marquee_text_layer_set_text_color(MarqueeTextLayer *marquee, GColor color) {
    marquee->text_colour = color;
    marquee_text_layer_mark_dirty(marquee);
}

void marquee_text_layer_set_background_color(MarqueeTextLayer *marquee, GColor color) {
    marquee->background_colour = color;
    marquee_text_layer_mark_dirty(marquee);
}

void marquee_text_layer_mark_dirty(MarqueeTextLayer *marquee) {
    marquee->text_width = -1;
    marquee->offset = 0;
    marquee->countdown = 100;
    layer_mark_dirty(marquee->layer);
}

void marquee_text_layer_tick() {
	MarqueeTextLayer *marquee = head;
	while(marquee) {
		if(marquee->countdown > 0) {
			--marquee->countdown;
			goto next;
		}
		marquee->offset += 1;
		layer_mark_dirty(marquee->layer);
	next:
		marquee = marquee->previous;
	}
}

void marquee_text_layer_update_proc(Layer* layer, GContext* context) {    
    MarqueeTextLayerReference *layerReference = (MarqueeTextLayerReference*)layer_get_data(layer);

    MarqueeTextLayer *marquee = layerReference->marqueeLayer;
    if(marquee->text[0] == '\0'){
        return;
    }
    if(marquee->text_width == -1) {
        marquee->text_width =
        graphics_text_layout_get_content_size(marquee->text, marquee->font,
            GRect(0, 0, 1000, layer_get_frame(layer).size.h),
            GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft).w;
	}

    graphics_context_set_fill_color(context, marquee->background_colour);
    graphics_context_set_text_color(context, marquee->text_colour);

	if(marquee->text_width < layer_get_frame(marquee->layer).size.w) {
        GRect rect = GRectZero;
        rect.size = layer_get_bounds(marquee->layer).size;
		graphics_draw_text(context, marquee->text, marquee->font, rect,
            GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
		return;
	}
    if(marquee->offset > marquee->text_width + 30) {
        marquee->offset = 0;
		marquee->countdown = 100;
    }
    if(marquee->offset < marquee->text_width) {
        graphics_draw_text(context, marquee->text, marquee->font,
            GRect(-marquee->offset, 0, marquee->text_width, layer_get_frame(layer).size.h),
            GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
    }
    if(marquee->offset > marquee->text_width - layer_get_frame(layer).size.w + 30) {
        graphics_draw_text(context, marquee->text, marquee->font,
            GRect(-marquee->offset + marquee->text_width + 30, 0, marquee->text_width, layer_get_frame(layer).size.h),
            GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
    }
}
