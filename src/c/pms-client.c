#ifndef PMS_INTERFACE
#define PMS_INTERFACE
#include "pms-interface.h"
#endif
#ifndef PMS_ROUTINES
#define PMS_ROUTINES
#include "pms-routines.h"
#endif
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

//**************************************************************************************

static void inbox_received_callback(DictionaryIterator *iter, void *context) 
{  
	char *function = "inbox_received_callback()";
	if (!bool_get_js_ready()) 
	{
		Tuple *ready_tuple = dict_find(iter, MESSAGE_KEY_JSReady);
		if(ready_tuple) 
		{
			set_js_ready(true);
			if (!bool_set_index(0, true))
			{
				if (!bool_log_error("bool_set_index() failed.", function, 0, false))
				{
					sos_pulse();
				}
			}
			if (!bool_set_response_items(0, true))
			{
				if (!bool_log_error("bool_set_response_items() failed to reset", function, 0, false))
				{
					sos_pulse();
				}
			}
			read_stored_values();
			if (!pms_verify_setup())
			{
				if (!bool_log_error("pms_verify_setup() failed in inbox_received_callback()", function, 0, false))
				{
					sos_pulse();
				}
			 	return;
			}
			pms_cards_reset_text();
		}
		APP_LOG(APP_LOG_LEVEL_DEBUG, "inbox_callback_received");
	}
	
	Tuple *pms_response_index = dict_find(iter, MESSAGE_KEY_PMS_RESPONSE_INDEX);
	if (pms_response_index)
	{
		int response_index = (int)pms_response_index->value->int32;
		if(response_index > 8) 
		{
			if (!bool_log_error("Over expected bounds for response array.", function, response_index, false))
			{
				sos_pulse();
			}
			return;
		}

		if (!bool_set_index(response_index, false))
		{
			if (!bool_log_error("Failed to Increment index", function, 0, false))
			{
				sos_pulse();
			}
		}
	}

	Tuple *pms_response_items = dict_find(iter, MESSAGE_KEY_PMS_RESPONSE_ITEMS);
	if (pms_response_items)
	{
		int response_items = (int)pms_response_items->value->int32;
		if (!bool_set_response_items(response_items, false))
		{
			if (!bool_log_error("bool_set_response_items() failed to set", function, response_items, false))
			{
				sos_pulse();
			}
		}
	}
	Tuple *server_response = dict_find(iter, MESSAGE_KEY_PMS_RESPONSE + int_get_response_index());
	
	if (server_response) 
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Server Response %s!", server_response->value->cstring);
		if (!bool_set_response_at_index(int_get_response_index(), server_response->value->cstring))
		{
			int index = int_get_response_index();
			if (!bool_log_error("Failed bool_set_response_at_index: ", function, index, false))
			{
				sos_pulse();
			}
		}
		return;
	}
	Tuple *sms_success = dict_find(iter, MESSAGE_KEY_PMS_SMS_SUCCESS);
	if (sms_success)
	{
		vibes_double_pulse();
		sms_success_handler(sms_success->value->cstring);
		return;
	}

	Tuple *pms_error = dict_find(iter, MESSAGE_KEY_PMS_ERROR);
	if (pms_error) 
	{
		if (!pms_error_response_handler(pms_error->value->cstring))
		{
			if (!bool_log_error("Error Handler failed in inbox_callback_received()", function, 0, false))
			{
				sos_pulse();
			}
		}
		return;
	}
	Tuple *pms_success = dict_find(iter, MESSAGE_KEY_PMS_SUCCESS);
	if(pms_success) 
	{
		vibes_double_pulse(); 
		set_mode(NONE);
		return;
	}
	Tuple *response_sent = dict_find(iter, MESSAGE_KEY_PMS_RESPONSE_SENT);
	if (response_sent) 
	{
//		if (!bool_set_response_items(int_get_response_index()))
//		{
//			if (!bool_log_error("Failed to set response items: response_sent event handler.", function, 0, false))
		{
			sos_pulse();
		}
//		}
		if (!bool_set_index(0, true))
		{
			if (!bool_log_error("Failed to set index in response_sent event handler.", function, 0, false))
			{
				sos_pulse();
			}
			return;
		}
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Received All Items");

		if (!menu_initializer())
		{
			if (!bool_log_error("menu_initializer() failed after receiving all items.", function, 0, false))
			{
				sos_pulse();
			}
			return;
		}
		return;
	}

	Tuple *server_url = dict_find(iter, MESSAGE_KEY_SERVER_URL);
	if(server_url) 
	{
		if (!bool_set_base_url(server_url->value->cstring))
		{
			if (!bool_log_error("bool_set_base_url() failed", function, 0, false))
			{
				sos_pulse();
			}
		}
	}
	Tuple *sonarr_port = dict_find(iter, MESSAGE_KEY_SONARR_PORT);
	if(sonarr_port) 
	{
		if (!bool_set_sonarr_port(sonarr_port->value->cstring))
		{
			if (!bool_log_error("failed to set sonar_port", function, 0, false))
			{
				sos_pulse();
			}
		}
	}
	
	Tuple *sonarr_api = dict_find(iter, MESSAGE_KEY_SONARR_API);
	if (sonarr_api) 
	{
		if (!bool_set_sonarr_api_key(sonarr_api->value->cstring))
		{
			if (!bool_log_error("bool_set_sonarr_api() failed", function, 0, false))
			{
				sos_pulse();
			}
		}
	}
	Tuple *radarr_api = dict_find(iter, MESSAGE_KEY_RADARR_API);
	if (radarr_api) 
	{
		if (!bool_set_radarr_api_key(radarr_api->value->cstring))
		{
			if (!bool_log_error("bool_set_radarr_api() failed.", function, 0, false))
			{
				sos_pulse();
			}
		}
	}
	Tuple *radarr_port = dict_find(iter, MESSAGE_KEY_RADARR_PORT);
	if (radarr_port) 
	{
		if (!bool_set_radarr_port(radarr_port->value->cstring))
		{
			if (!bool_log_error("bool_set_radarr_port() failed.", function, 0, false))
			{
				sos_pulse();
			}
		}
	}

	Tuple *sms_port = dict_find(iter, MESSAGE_KEY_SMS_PORT);
	if (sms_port)
	{
		if (!bool_set_sms_port(sms_port->value->cstring))
		{
			if (!bool_log_error("bool_set_sms_port() failed", function, 0, false))
			{
				sos_pulse();
			}
		}
	}

	if(!persist_read_bool(MESSAGE_KEY_PMS_IS_CONFIGURED)) 
	{
		pms_verify_setup();
		APP_LOG(APP_LOG_LEVEL_DEBUG, "attempting to verify setup");
		return;
	}  
	return;  
}

bool initialize_client()
{
	char *function = "initialize_client()";
	if (!pms_init())
	{
		if (!bool_log_error("pms_init failed at initialize_client()", function, 0, false))
		{
			sos_pulse();
		}
		return false;
	}
	persist_write_bool(MESSAGE_KEY_PMS_IS_CONFIGURED, false);
	set_js_ready(false);
	app_message_register_inbox_received(inbox_received_callback);
	app_message_register_inbox_dropped(inbox_dropped_callback);
	app_message_register_outbox_failed(outbox_failed_callback);
	app_message_register_outbox_sent(outbox_sent_callback);  
	const int inbox_size = 512;
	const int outbox_size = 512;
	app_message_open(inbox_size, outbox_size);
	set_response_sent(false);
	return true;
}
