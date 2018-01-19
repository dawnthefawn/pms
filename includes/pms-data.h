/* This file was automatically generated.  Do not edit! */
#ifndef PEBBLE_INCLUDED
#define PEBBLE_INCLUDED
#include <pebble.h>
#endif
enum modes 
{
	NONE,
	SONARR,
	RADARR,
	DICTATION,
	MENU,
	PROCESS,
};
bool bool_log_error(char *text, char *function, int value, bool zerovalue);
void sos_pulse();
bool bool_set_sms_port(char *sms_port);
char * str_get_sms_port();
bool bool_reset_response_array();
int int_get_pms_response_items();
bool bool_set_response_items(int items, bool reset);
int int_get_response_index();
bool bool_set_index(int index, bool reset);
bool bool_set_response_at_index(int index,char *response);
char * str_response_at_index(MenuIndex *cell_index);
bool bool_get_js_ready();
void set_js_ready(bool ready);
char *str_get_last_text();
bool bool_set_last_text(char *transcription);
void set_response_sent(bool *response_sent);
bool bool_get_response_sent();
void read_stored_values();
char * str_sonarr_api_key();
char * str_radarr_api_key();
char * str_sonarr_port() ;
char * str_radarr_port();
char * str_radarr_port();
char * str_base_url();
bool bool_set_sonarr_api_key(char *sonarr_api);
bool bool_set_radarr_api_key(char *radarr_api);
bool bool_set_sonarr_port(char *sonarr_port);
bool bool_set_radarr_port(char *radarr_port);
bool bool_set_base_url(char *base_url);
void set_mode(enum modes mode);
int int_get_mode();
int int_get_response_items();
