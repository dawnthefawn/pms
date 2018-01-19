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
		set_mode(NONE);
		return true;
	}
	return false;
}

void timeout_timer_handler(void *context) 
{
	char *function = "timeout_timter_handler()";
	if (!bool_cancel_timer())
	{
		if (!bool_log_error("bool_cancel_timer() failed", function, 0, false))
		{
			sos_pulse();
		}
	}
}

void inbox_dropped_callback(AppMessageResult reason, void *context) 
{
	char *function = "inbox_dropped_callback()";
	if (!bool_log_error("Message Dropped", function, 0, false))
	{
		sos_pulse();
	}
}  

void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) 
{
	char *function = "outbox_failed_callback()";
	if (!bool_log_error("Outbox failed", function, 0, false))
	{
		sos_pulse();
	}
	if (!bool_cancel_timer())
	{
		if (!bool_log_error("bool_cancel_timer() failed", function, 0, false))
		{
			sos_pulse();
		}
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
	snprintf(s_buff, sizeof(s_buff), "%s", str_response_at_index(cell_index));
	menu_cell_basic_draw(ctx, cell_layer, s_buff, NULL, NULL);
}

int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) 
{
	const int16_t cell_height = 44;
	return cell_height;
}


bool pms_verify_setup() 
{
	char *function = "pms_verify_setup()";

	
	DictionaryIterator *out_iter;
	AppMessageResult result = app_message_outbox_begin(&out_iter);
	if (result == APP_MSG_OK) 
	{
		dict_write_cstring(out_iter, MESSAGE_KEY_SERVER_URL, str_base_url());
		dict_write_cstring(out_iter, MESSAGE_KEY_SONARR_PORT, str_sonarr_port());
		dict_write_cstring(out_iter, MESSAGE_KEY_SONARR_API, str_sonarr_api_key());
		dict_write_cstring(out_iter, MESSAGE_KEY_RADARR_PORT, str_radarr_port());
		dict_write_cstring(out_iter, MESSAGE_KEY_RADARR_API, str_radarr_api_key());
		dict_write_cstring(out_iter, MESSAGE_KEY_SMS_PORT, str_get_sms_port());
		result = app_message_outbox_send();
		if(result != APP_MSG_OK) 
		{
			if (!bool_log_error("pms_verify_setup(): Error sending outbox message", function, 0, false))
			{
				sos_pulse();
			}
			return false;
		}
	} 
	else 
	{
		if (!bool_log_error("pms_verify_setup(); outbox unreachable", function, 0, false))
		{
			sos_pulse();
		}
		return false;
	}
	persist_write_bool(MESSAGE_KEY_PMS_IS_CONFIGURED, true);
	return true;
}


bool pms_request_handler(int *choice, bool is_sms_request) 
{   
	char *function = "pms_request_handler(int *choice, bool is_sms_request)";

	if (bool_get_js_ready())
	{
		DictionaryIterator *out_iter;
		AppMessageResult result = app_message_outbox_begin(&out_iter);
		if (result == APP_MSG_OK) 
		{
			int yes = 1;
			int no = 0;
			switch (int_get_mode()) 
			{
				case NONE:
					return false;
					break;
				case SONARR:
					dict_write_int(out_iter, MESSAGE_KEY_PMS_SERVICE_SONARR, &yes, sizeof(int), true);
					dict_write_int(out_iter, MESSAGE_KEY_PMS_SERVICE_RADARR, &no, sizeof(int), true);
					if (is_sms_request)
					{
						dict_write_int(out_iter, MESSAGE_KEY_PMS_SERVICE_REQUEST_IS_SMS, &yes, sizeof(int), true);
					}
					else
					{
						dict_write_int(out_iter, MESSAGE_KEY_PMS_SERVICE_REQUEST_IS_SMS, &no, sizeof(int), true);
					}
					break;
				case RADARR:
					dict_write_int(out_iter, MESSAGE_KEY_PMS_SERVICE_RADARR, &yes, sizeof(int), true);
					dict_write_int(out_iter, MESSAGE_KEY_PMS_SERVICE_SONARR, &no, sizeof(int), true);
					if (is_sms_request)
					{
						dict_write_int(out_iter, MESSAGE_KEY_PMS_SERVICE_REQUEST_IS_SMS, &yes, sizeof(int), true);
					}
					else
					{
						dict_write_int(out_iter, MESSAGE_KEY_PMS_SERVICE_REQUEST_IS_SMS, &no, sizeof(int), true);
					}
					break;
				case DICTATION:
					dict_write_cstring(out_iter, MESSAGE_KEY_PMS_REQUEST, str_get_last_text());
					APP_LOG(APP_LOG_LEVEL_DEBUG, "wrote request to dictionary: %s", str_get_last_text());	

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
			const int interval_ms = 10000;
			s_timeout_timer = app_timer_register(interval_ms, timeout_timer_handler, NULL);
			if(result != APP_MSG_OK) 
			{
				if (!bool_log_error("Error sending outbox message", function, 0, false))
				{
					sos_pulse();
				}
				return false;
			} else 
			{
				switch (int_get_mode()) 
				{
					case NONE:
						return true;
						break;
					case SONARR: 
						return true;
						break;
					case RADARR:
						return true;
						break;
					case DICTATION:
						set_mode(NONE);
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
			if (!bool_log_error("outbox unreachable", function, 0, false))
			{
				sos_pulse();
			}
			return false;
		}
		return false;
	}
	else
	{
		if (!bool_log_error("bool_get_js_ready() returned false in pms_request_handler()", function, 0, false))
		{
			sos_pulse();
		}
		return false;
	}

}

