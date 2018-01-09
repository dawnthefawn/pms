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
	if (!bool_get_js_ready()) 
	{
		Tuple *ready_tuple = dict_find(iter, MESSAGE_KEY_JSReady);
		if(ready_tuple) 
		{
			set_js_ready(true);
			if (!bool_set_index(0, true))
			{
				APP_LOG(APP_LOG_LEVEL_ERROR, "bool_set_index() failed.");
			}
			if (!bool_set_response_items(0, true))
			{
				APP_LOG(APP_LOG_LEVEL_ERROR, "bool_set_response_items() failed to reset");
			}
			read_stored_values();
			if (!pms_verify_setup())
			{
				APP_LOG(APP_LOG_LEVEL_ERROR, "pms_verify_setup() failed in inbox_received_callback()");
			 	return;
			}
		}
		APP_LOG(APP_LOG_LEVEL_DEBUG, "inbox_callback_received");
	}
	
	Tuple *pms_response_index = dict_find(iter, MESSAGE_KEY_PMS_RESPONSE_INDEX);
	if (pms_response_index)
	{
		int response_index = (int)pms_response_index->value->int32;
		if(response_index > 8) 
		{
			APP_LOG(APP_LOG_LEVEL_ERROR, "Over expected bounds for response array. %d", response_index);
			return;
		}

		if (!bool_set_index(response_index, false))
		{
			APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to Increment index");
		}
	}

	Tuple *pms_response_items = dict_find(iter, MESSAGE_KEY_PMS_RESPONSE_ITEMS);
	if (pms_response_items)
	{
		int response_items = (int)pms_response_items->value->int32;
		if (!bool_set_response_items(response_items, false))
		{
			APP_LOG(APP_LOG_LEVEL_ERROR, "bool_set_response_items() failed to set %d", response_items);
		}
	}
	Tuple *server_response = dict_find(iter, MESSAGE_KEY_PMS_RESPONSE + int_get_response_index());
	
	if (server_response) 
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Received Server Response %s!", server_response->value->cstring);
		if (!bool_set_response_at_index(int_get_response_index(), server_response->value->cstring))
		{
			APP_LOG(APP_LOG_LEVEL_ERROR, "Failed bool_set_response_at_index(%d)", int_get_response_index());
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
			APP_LOG(APP_LOG_LEVEL_ERROR, "Error Handler failed in inbox_callback_received()");
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
//			APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to set response items: response_sent event handler.");
//		}
		if (!bool_set_index(0, true))
		{
			APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to set index in response_sent event handler.");
			return;
		}
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Received All Items");

		if (!menu_initializer())
		{
			APP_LOG(APP_LOG_LEVEL_ERROR, "menu_initializer() failed after receiving all items.");
			return;
		}
		return;
	}

	Tuple *server_url = dict_find(iter, MESSAGE_KEY_SERVER_URL);
	if(server_url) 
	{
		if (!bool_set_base_url(server_url->value->cstring))
		{
			APP_LOG(APP_LOG_LEVEL_ERROR, "bool_set_base_url() failed");
		}
	}
	Tuple *sonarr_port = dict_find(iter, MESSAGE_KEY_SONARR_PORT);
	if(sonarr_port) 
	{
		if (!bool_set_sonarr_port(sonarr_port->value->cstring))
		{
			APP_LOG(APP_LOG_LEVEL_ERROR, "failed to set sonar_port");
		}
	}
	
	Tuple *sonarr_api = dict_find(iter, MESSAGE_KEY_SONARR_API);
	if (sonarr_api) 
	{
		if (!bool_set_sonarr_api_key(sonarr_api->value->cstring))
		{
			APP_LOG(APP_LOG_LEVEL_ERROR, "bool_set_sonarr_api() failed");
		}
	}
	Tuple *radarr_api = dict_find(iter, MESSAGE_KEY_RADARR_API);
	if (radarr_api) 
	{
		if (!bool_set_radarr_api_key(radarr_api->value->cstring))
		{
			APP_LOG(APP_LOG_LEVEL_ERROR, "bool_set_radarr_api() failed.");
		}
	}
	Tuple *radarr_port = dict_find(iter, MESSAGE_KEY_RADARR_PORT);
	if (radarr_port) 
	{
		if (!bool_set_radarr_port(radarr_port->value->cstring))
		{
			APP_LOG(APP_LOG_LEVEL_ERROR, "bool_set_radarr_port() failed.");
		}
	}

	Tuple *sms_port = dict_find(iter, MESSAGE_KEY_SMS_PORT);
	if (sms_port)
	{
		if (!bool_set_sms_port(sms_port->value->cstring))
		{
			APP_LOG(APP_LOG_LEVEL_ERROR, "bool_set_sms_port() failed");
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
	if (!pms_init())
	{
		APP_LOG(APP_LOG_LEVEL_ERROR, "pms_init failed at initialize_client()");
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
