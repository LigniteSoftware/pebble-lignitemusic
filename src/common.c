#include <pebble.h>
#include "common.h"

static uint32_t s_sequence_number = 0xFFFFFFFE;

iPodMessage *ipod_message_outbox_get() {
    iPodMessage *message = malloc(sizeof(iPodMessage));

    NSLog("Alloced message %p", message);

    AppMessageResult result = app_message_outbox_begin(&message->iter);
    message->result = result;
    NSLog("Result %d. Iter null? %s", message->result, message->iter == NULL ? "yeah" : "nah");
    if(result != APP_MSG_OK) {
        return message;
    }
    dict_write_int32(message->iter, IPOD_SEQUENCE_NUMBER_KEY, ++s_sequence_number);
    if(s_sequence_number == 0xFFFFFFFF) {
        s_sequence_number = 1;
        NSWarn("Set sequence number to 1.");
    }
    return message;
}

void reset_sequence_number() {
    DictionaryIterator *iter = NULL;
    app_message_outbox_begin(&iter);
    if(!iter) {
        return;
    }
    dict_write_int32(iter, IPOD_SEQUENCE_NUMBER_KEY, 0xFFFFFFFF);
    app_message_outbox_send();
}