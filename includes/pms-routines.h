/* This file was automatically generated.  Do not edit! */
#ifndef PEBBLE_INCLUDED
#define PEBBLE_INCLUDED
#include <pebble.h>
#endif

bool bool_get_js_ready();
bool pms_request_handler(int *choice, bool is_sms_request);
bool pms_verify_setup();
int16_t get_cell_height_callback(struct MenuLayer *menu_layer,MenuIndex *cell_index,void *context);
void draw_row_callback(GContext *ctx,const Layer *cell_layer,MenuIndex *cell_index,void *context);
int int_get_pms_response_items();
uint16_t get_num_rows_callback(MenuLayer *menu_layer,uint16_t section_index,void *context);
void outbox_sent_callback(DictionaryIterator *iterator,void *context);
void outbox_failed_callback(DictionaryIterator *iterator,AppMessageResult reason,void *context);
void inbox_dropped_callback(AppMessageResult reason,void *context);
void timeout_timer_handler(void *context);
