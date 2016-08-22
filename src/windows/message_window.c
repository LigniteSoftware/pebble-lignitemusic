#include <lignite_music.h>

MessageWindow *to_destroy = NULL;

void message_window_update_proc(Layer *layer, GContext *ctx){
    MessageWindowReference *reference = (MessageWindowReference*)layer_get_data(layer);
    MessageWindow *message_window = reference->window;

    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);

    graphics_context_set_compositing_mode(ctx, GCompOpSet);
    graphics_draw_bitmap_in_rect(ctx, message_window->icon, message_window->icon_frame);

    graphics_context_set_text_color(ctx, GColorWhite);
    graphics_draw_text(ctx, message_window->message[0], fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
        message_window->text_frame, GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
}

void message_window_update_frames(MessageWindow *window){
    GSize icon_size = GSize(0, 0);
    if(window->icon){
        icon_size = gbitmap_get_bounds(window->icon).size;
    }

    GSize text_size = graphics_text_layout_get_content_size_with_attributes(window->message[0],
        fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD), WINDOW_FRAME,
        GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);

    uint16_t offset_y = (WINDOW_FRAME.size.h-(icon_size.h+text_size.h))/2;

    GRect icon_frame = GRect((WINDOW_FRAME.size.w/2)-(icon_size.w/2), offset_y, icon_size.w, icon_size.h);
    GRect message_frame = GRect(0, icon_frame.origin.y+icon_frame.size.h, WINDOW_FRAME.size.w, WINDOW_FRAME.size.h);

    window->icon_frame = icon_frame;
    window->text_frame = message_frame;

    layer_mark_dirty(window->root_layer);
}

MessageWindow *message_window_create(){
    MessageWindow *new_message_window = malloc(sizeof(MessageWindow));

    new_message_window->icon = NULL;

    new_message_window->root_layer = layer_create_with_data(GRect(WINDOW_FRAME.origin.x,
        -WINDOW_FRAME.size.h, WINDOW_FRAME.size.w, WINDOW_FRAME.size.h), sizeof(MessageWindowReference));

    MessageWindowReference *reference = (MessageWindowReference*)layer_get_data(new_message_window->root_layer);
    reference->window = new_message_window;
    layer_set_update_proc(new_message_window->root_layer, message_window_update_proc);

    return new_message_window;
}

void message_window_destroy(MessageWindow *window){
    layer_destroy(window->root_layer);
    if(window->destroy_bitmap){
        gbitmap_destroy(window->icon);
    }

    free(window);
    window = NULL;

    NSLog("Free: %d", heap_bytes_free());
}

void message_window_auto_destroy(void *window){
    MessageWindow *message_window = (MessageWindow*)window;
    message_window_destroy(message_window);
}

void message_window_push_on_window(MessageWindow *message_window, Window *root_window, bool animated){
    if(message_window->is_pushed){
        return;
    }
    message_window->root_window = root_window;

    GRect old_frame = GRect(0, -WINDOW_FRAME.size.h, WINDOW_FRAME.size.w, WINDOW_FRAME.size.h);
    GRect new_frame = GRect(0, 0, WINDOW_FRAME.size.w, WINDOW_FRAME.size.h);
    if(animated){
        animate_layer(message_window->root_layer, &old_frame, &new_frame, 200, 0);
    }
    else{
        layer_set_frame(message_window->root_layer, new_frame);
    }

    layer_add_child(window_get_root_layer(root_window), message_window->root_layer);

    message_window->is_pushed = true;
}

void message_window_pop_off_window(MessageWindow *message_window, bool animated, int auto_destroy){
    if(!message_window){
        return;
    }
    if(!message_window->is_pushed){
        return;
    }

    GRect old_frame = GRect(0, 0, WINDOW_FRAME.size.w, WINDOW_FRAME.size.h);
    GRect new_frame = GRect(0, -WINDOW_FRAME.size.h, WINDOW_FRAME.size.w, WINDOW_FRAME.size.h);
    if(animated){
        animate_layer(message_window->root_layer, &old_frame, &new_frame, 200, 0);
    }
    else{
        layer_set_frame(message_window->root_layer, new_frame);
    }

    message_window->is_pushed = false;

    app_timer_register(500, message_window_auto_destroy, message_window);
}

void message_window_set_text(MessageWindow *window, char *text){
    strncpy(window->message[0], text, sizeof(window->message[0]));

    message_window_update_frames(window);
}

void message_window_set_icon(MessageWindow *window, GBitmap *icon, bool auto_destroy){
    window->destroy_bitmap = auto_destroy;
    window->icon = icon;

    message_window_update_frames(window);
}