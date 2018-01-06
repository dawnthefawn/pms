#include <pebble.h>
#include "pms-interface.c"

//**************************************************************************************

static void inbox_received_callback(DictionaryIterator *iter, void *context) {  
	if (!bool_get_js_ready()) {
		Tuple *ready_tuple = dict_find(iter, MESSAGE_KEY_JSReady);
		if(ready_tuple) {
			if (!blnSetJSReady(true))
			{
				APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to set JSReady");
			}

			if (!bool_set_index(0))
			{
				APP_LOG(APP_LOG_LEVEL_ERROR, "bool_set_index() failed.");
			}
			read_stored_values();
			return;
		}
		APP_LOG(APP_LOG_LEVEL_DEBUG, "inbox_callback_received");
	}
	Tuple *pms_error = dict_find(iter, MESSAGE_KEY_PMS_ERROR);
	if (pms_error) {
		if (!pms_error_response_handler(pms_error->value->cstring))
		{
			APP_LOG(APP_LOG_LEVEL_ERROR, "Error Handler failed in inbox_callback_received()");
		}
		return;
	}
	Tuple *pms_success = dict_find(iter, MESSAGE_KEY_PMS_SUCCESS);
	if(pms_success) {
		vibes_double_pulse(); 
		if (!set_mode(NONE))
		{

			APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to set mode in inbox_callback_received()");
		}
		return;
	}
	Tuple *server_response = dict_find(iter, MESSAGE_KEY_PMS_RESPONSE + s_pms_response_index);
	if (server_response) {
		if (!bool_set_response_at_index(int_get_response_index()))
		{
			APP_LOG(APP_LOG_LEVEL_ERROR, "Failed bool_set_response_at_index(%d)", int_get_response_index() + 1);
		}

		if(int_get_response_index() > 8) {
			APP_LOG(APP_LOG_LEVEL_ERROR, "Over expected bounds for response array.");
		}
	}
	Tuple *response_sent = dict_find(iter, MESSAGE_KEY_PMS_RESPONSE_SENT);
	if (response_sent) {
		if (!bool_set_response_items(int_get_response_index()))
		{
			APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to set response items: response_sent event handler.");
		}
		if (!bool_set_index(0))
		{
			APP_LOG(APP_LOG_LEVEL_ERROR, "Failed to set index in response_sent event handler.");
		}
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Received All Items");

		// what the fuck do I do about this?		
		//		initialize_menu();

	}
	//}

Tuple *server_url = dict_find(iter, MESSAGE_KEY_SERVER_URL);
if(server_url) {
	if (!bool_set_base_url(server_url->value->cstring))
	{
		APP_LOG(APP_LOG_LEVEL_ERROR, "bool_set_base_url() failed");
	}
}
Tuple *sonarr_port = dict_find(iter, MESSAGE_KEY_SONARR_PORT);
if(sonarr_port) {
	bool_set_sonarr_port(sonarr_port->value->cstring));
	APP_LOG(APP_LOG_LEVEL_DEBUG, "sonarr port is set to %s", s_pms_sonarr_port);
}

Tuple *sonarr_api = dict_find(iter, MESSAGE_KEY_SONARR_API);
if (sonarr_api) {
	if (!bool_set_sonarr_api(sonarr_api->value->cstring))
	{
		APP_LOG(APP_LOG_LEVEL_ERROR, "bool_set_sonarr_api() failed");
	}
}
Tuple *radarr_api = dict_find(iter, MESSAGE_KEY_RADARR_API);
if (radarr_api) {
	if (!bool_set_radarr_api(radarr_api->value->cstring))
	{
		APP_LOG(APP_LOG_LEVEL_ERROR, "bool_set_radarr_api() failed.");
	}
}
Tuple *radarr_port = dict_find(iter, MESSAGE_KEY_RADARR_PORT);
if (radarr_port) {
	if (!bool_set_radarr_port(radarr_port->value->cstring))
	{
		APP_LOG(APP_LOG_LEVEL_ERROR, "bool_set_radarr_port() failed.");
	}
}
if(!persist_read_bool(MESSAGE_KEY_PMS_IS_CONFIGURED)) {
	pms_verify_setup();
	APP_LOG(APP_LOG_LEVEL_DEBUG, "attempting to verify setup");
	return;
}  
return;  
}

