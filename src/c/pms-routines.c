#ifndef PMS_DATA
#define PMS_DATA
#include "pms-data.h"
#endif
#ifndef CORE_LIBRARIES_INCLUDED
#define CORE_LIBRARIES_INCLUDED
#include <stdio.h>
#include <string.h>
#endif
#ifndef PEBBLE_INCLUDED
#define PEBBLE_INCLUDED
#include <pebble.h>
#endif

static AppTimer *s_timeout_timer;


//**************************************************************************************

static bool bool_cancel_timer() 
{
	if (s_timeout_timer) 
	{
		app_timer_cancel(s_timeout_timer); 
		s_timeout_timer = NULL;
		return true;
	}
	return false;
}

void timeout_timer_handler(void *context) 
{
	if (!bool_cancel_timer())
	{
		APP_LOG(APP_LOG_LEVEL_ERROR, "bool_cancel_timer failed");
	}
}

void inbox_dropped_callback(AppMessageResult reason, void *context) 
{
	APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped! "); 
}  

void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) 
{
	APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!"); 
	if (!bool_cancel_timer())
	{
		APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to cancel timer");
	}
}  


void outbox_sent_callback(DictionaryIterator *iterator, void *context) 
{
	APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) 
{
	const uint16_t numrows = int_get_pms_response_items();
	return numrows;
}

void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) 
{
	static char s_buff[16];
	snprintf(s_buff, sizeof(s_buff), "%d", str_response_at_index((int)cell_index->row));
	menu_cell_basic_draw(ctx, cell_layer, s_buff, NULL, NULL);
}

int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) 
{
	const int16_t cell_height = 44;
	return cell_height;
}




bool pms_verify_setup() 
{
	DictionaryIterator *out_iter;
	AppMessageResult result = app_message_outbox_begin(&out_iter);
	if (result == APP_MSG_OK) 
	{
		dict_write_cstring(out_iter, MESSAGE_KEY_SERVER_URL, str_base_url());
		dict_write_cstring(out_iter, MESSAGE_KEY_SONARR_PORT, str_sonarr_port());
		dict_write_cstring(out_iter, MESSAGE_KEY_SONARR_API, str_sonarr_api_key());
		dict_write_cstring(out_iter, MESSAGE_KEY_RADARR_PORT, str_radarr_port());
		dict_write_cstring(out_iter, MESSAGE_KEY_RADARR_API, str_radarr_api_key());
		result = app_message_outbox_send();
		if(result != APP_MSG_OK) 
		{
			persist_write_bool(MESSAGE_KEY_PMS_IS_CONFIGURED, true);
			return true;

		}
		else
		{
			APP_LOG(APP_LOG_LEVEL_ERROR, "pms_verify_setup(): Error sending outbox message");
		}
	} 
	else 
	{
		APP_LOG(APP_LOG_LEVEL_ERROR, "pms_verify_setup(); outbox unreachable");
	}
	return false;
}


bool pms_request_handler(int *choice) 
{   
	if (bool_get_js_ready())
	{
		DictionaryIterator *out_iter;
		AppMessageResult result = app_message_outbox_begin(&out_iter);
		if (result == APP_MSG_OK) 
		{
			int value = 1;
			switch (int_get_mode()) 
			{
				case NONE:
					return false;
					break;
				case SONARR:
					dict_write_int(out_iter, MESSAGE_KEY_PMS_SERVICE_SONARR, &value, sizeof(int), true);
					break;
				case RADARR:
					dict_write_int(out_iter, MESSAGE_KEY_PMS_SERVICE_RADARR, &value, sizeof(int), true);
					break;
				case DICTATION:
					dict_write_cstring(out_iter, MESSAGE_KEY_PMS_REQUEST, str_get_last_text());
					break;
				case MENU:
					dict_write_int(out_iter, MESSAGE_KEY_PMS_CHOICE, choice, sizeof(int), true);
					dict_write_cstring(out_iter, MESSAGE_KEY_PMS_REQUEST, NULL);
					break;
				case PROCESS:
					return false;
					break;
			} 
			result = app_message_outbox_send();
			if(result != APP_MSG_OK) 
			{
				APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending outbox message");
				return false;
			} else 
			{
				switch (int_get_mode()) 
				{
					case NONE:
						return false;
						break;
					case SONARR: 
						return true;
						break;
					case RADARR:
						return true;
						break;
					case DICTATION:
						return true;
						break;
					case MENU:
						return true;
						break;
					case PROCESS:
						return false;
						break;
				} 
			}
		} else 
		{
			APP_LOG(APP_LOG_LEVEL_ERROR, "outbox unreachable");
			return false;
		}
		return false;
	}
	else
	{
		APP_LOG(APP_LOG_LEVEL_ERROR, "bool_get_js_ready() returned false in pms_request_handler()");
		return false;
	}

}

