#include <lignite_music.h>

MessageWindow *to_destroy = NULL;

void message_window_update_proc(Layer *layer, GContext *ctx){
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}

void message_window_update_frames(MessageWindow *window){
    GSize icon_size = gbitmap_get_bounds(bitmap_layer_get_bitmap(window->icon_layer)).size;

    GSize text_size = graphics_text_layout_get_content_size_with_attributes(window->message[0],
        GFon, const GRect box, const GTextOverflowMode overflow_mode, const GTextAlignment alignment, GTextAttributes * text_attributes)

    Layer *icon_layer_root_layer = bitmap_layer_get_layer(window->icon_layer);
    GRect icon_layer_frame = GRect(0, (168/2)-icon_size.h, 144, icon_size.h);
    layer_set_frame(icon_layer_root_layer, icon_layer_frame);

    Layer *message_layer_root_layer = text_layer_get_layer(window->message_layer);
    GRect message_layer_frame = GRect(0, icon_layer_frame.origin.y+icon_layer_frame.size.h, 144, 168);
    layer_set_frame(message_layer_root_layer, message_layer_frame);
}

MessageWindow *message_window_create(){
    MessageWindow *new_message_window = malloc(sizeof(MessageWindow));

    NSLog("Size is %d and reference %d", sizeof(MessageWindow), sizeof(MessageWindowReference));

    new_message_window->root_layer = layer_create_with_data(GRect(0, -168, 144, 168), sizeof(MessageWindowReference));
    MessageWindowReference *reference = (MessageWindowReference*)layer_get_data(new_message_window->root_layer);
    reference->window = new_message_window;
    layer_set_update_proc(new_message_window->root_layer, message_window_update_proc);

    new_message_window->message_layer = text_layer_create(GRect(0, 0, 144, 168));
    text_layer_set_text_alignment(new_message_window->message_layer, GTextAlignmentCenter);
    text_layer_set_text_color(new_message_window->message_layer, GColorWhite);
    text_layer_set_background_color(new_message_window->message_layer, GColorBlack);
    text_layer_set_font(new_message_window->message_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
    layer_add_child(new_message_window->root_layer, text_layer_get_layer(new_message_window->message_layer));

    new_message_window->icon_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
    layer_add_child(new_message_window->root_layer, bitmap_layer_get_layer(new_message_window->icon_layer));

    return new_message_window;
}

void message_window_destroy(MessageWindow *window){
    layer_destroy(window->root_layer);
    text_layer_destroy(window->message_layer);
    if(window->destroy_bitmap){
        bitmap_layer_set_bitmap(window->icon_layer, NULL); //Prevents the app from crashing
        gbitmap_destroy(window->icon);
    }
    bitmap_layer_destroy(window->icon_layer);

    free(window);
    window = NULL;
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
    layer_add_child(window_get_root_layer(root_window), message_window->root_layer);

    if(animated){
        animate_layer(message_window->root_layer, &GRect(0, -168, 144, 168), &GRect(0, 0, 144, 168), 200, 0);
    }
    else{
        layer_set_frame(message_window->root_layer, GRect(0, 0, 144, 168));
    }

    message_window->is_pushed = true;
}

void message_window_pop_off_window(MessageWindow *message_window, bool animated, bool auto_destroy){
    if(!message_window->is_pushed){
        return;
    }

    if(animated){
        animate_layer(message_window->root_layer, &GRect(0, 0, 144, 168), &GRect(0, -168, 144, 168), 200, 0);
    }
    else{
        layer_set_frame(message_window->root_layer, GRect(0, -168, 144, 168));
    }

    message_window->is_pushed = false;

    if(auto_destroy && !animated){
        message_window_destroy(message_window);
    }
    else if(auto_destroy && animated){
        app_timer_register(500, message_window_auto_destroy, message_window);
    }
}

void message_window_set_text(MessageWindow *window, char *text){
    strncpy(window->message[0], text, sizeof(window->message[0]));
    text_layer_set_text(window->message_layer, window->message[0]);

    message_window_update_frames(window);
}

void message_window_set_icon(MessageWindow *window, GBitmap *icon, bool auto_destroy){
    window->destroy_bitmap = auto_destroy;
    window->icon = icon;

    bitmap_layer_set_bitmap(window->icon_layer, icon);

    message_window_update_frames(window);
}