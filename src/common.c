#include <lignite_music.h>

static uint32_t s_sequence_number = 0xFFFFFFFE;

void on_animation_stopped(Animation *anim, bool finished, void *context){
    #ifdef PBL_PLATFORM_APLITE
        property_animation_destroy((PropertyAnimation*) anim);
    #endif
}

void animate_layer(Layer *layer, GRect *start, GRect *finish, int length, int delay){
    PropertyAnimation *anim = property_animation_create_layer_frame(layer, start, finish);

    animation_set_duration(property_animation_get_animation(anim), length);
    animation_set_delay(property_animation_get_animation(anim), delay);

    AnimationHandlers handlers = {
        .stopped = (AnimationStoppedHandler) on_animation_stopped
    };
    animation_set_handlers(property_animation_get_animation(anim), handlers, NULL);

    animation_schedule(property_animation_get_animation(anim));
}

void ipod_message_timer_fire(void *timerContext){
    iPodMessage *message = (iPodMessage*)timerContext;
    // NSDebug("Freeing %p.", message);
    free(message);
    // NSDebug("Done.");
}

void ipod_message_destroy(iPodMessage *message){
    // NSDebug("Registering to destroy %p after %dms.", message, IPOD_MESSAGE_DESTROY_TIME);
    app_timer_register(IPOD_MESSAGE_DESTROY_TIME, ipod_message_timer_fire, message);
}

iPodMessage *ipod_message_outbox_get() {
    iPodMessage *message = malloc(sizeof(iPodMessage));

    ipod_message_destroy(message);

    //NSLog("Alloced message %p", message);

    AppMessageResult result = app_message_outbox_begin(&message->iter);
    message->result = result;
    //NSLog("Result %d. Iter null? %s", message->result, message->iter == NULL ? "yeah" : "nah");
    if(result != APP_MSG_OK) {
        return message;
    }
    dict_write_int32(message->iter, MessageKeySequenceNumber, ++s_sequence_number);
    if(s_sequence_number == 0xFFFFFFFF) {
        s_sequence_number = 1;
        //NSWarn("Set sequence number to 1.");
    }
    return message;
}

void reset_sequence_number() {
    DictionaryIterator *iter = NULL;
    app_message_outbox_begin(&iter);
    if(!iter) {
        return;
    }
    dict_write_int32(iter, MessageKeySequenceNumber, 0xFFFFFFFF);
    app_message_outbox_send();
}