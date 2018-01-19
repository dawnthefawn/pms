/* This file was automatically generated.  Do not edit! */
#ifndef PEBBLE_INCLUDED
#define PEBBLE_INCLUDED
#include <pebble.h>
#endif
bool bool_set_text(char *text, bool reset);
bool menu_initializer();
bool pms_init();
int16_t get_cell_height_callback(struct MenuLayer *menu_layer,MenuIndex *cell_index,void *context);
void draw_row_callback(GContext *ctx,const Layer *cell_layer,MenuIndex *cell_index,void *context);
uint16_t get_num_rows_callback(MenuLayer *menu_layer,uint16_t section_index,void *context);
void pms_deinit(void);
int int_get_mode();
bool pms_error_response_handler(char *error_message);
void sms_success_handler(char *msg);
