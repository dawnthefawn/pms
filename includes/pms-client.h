/* This file was automatically generated.  Do not edit! */
#ifndef PEBBLE_INCLUDED
#define PEBBLE INCLUDED
#include <pebble.h>
#endif
bool initialize_client();
void set_response_sent(bool *response_sent);
void outbox_sent_callback(DictionaryIterator *iterator,void *context);
void outbox_failed_callback(DictionaryIterator *iterator,AppMessageResult reason,void *context);
void inbox_dropped_callback(AppMessageResult reason,void *context);
bool pms_verify_setup();
