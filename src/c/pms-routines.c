#include <pebble.h>
#include <stdio.h>
#include <string.h>

static AppTimer *s_timeout_timer;



//**************************************************************************************

void cancel_timer() 
{
	if (s_timeout_timer) 
	{
		app_timer_cancel(s_timeout_timer); 
		s_timeout_timer = NULL;
	}
}

static void timeout_timer_handler(void *context) 
{
	cancel_timer();
}


static void pms_handle_request() 
{
	if (s_js_ready == true) 
	{
		DictionaryIterator *out_iter;
		AppMessageResult result = app_message_outbox_begin(&out_iter);
		if (result == APP_MSG_OK) 
		{
			dict_write_cstring(out_iter, MESSAGE_KEY_PMS_REQUEST, s_last_text);
			app_message_outbox_send();
		}
	}
	const int interval = 5000;
	s_timeout_timer = app_timer_register(interval, timeout_timer_handler, NULL); 
	if (s_timeout_timer) 
	{
		s_timeout_timer = NULL;
	}
}


static void pms_verify_setup() 
{
	DictionaryIterator *out_iter;
	AppMessageResult result = app_message_outbox_begin(&out_iter);
	if (result == APP_MSG_OK) 
	{
		dict_write_cstring(out_iter, MESSAGE_KEY_SERVER_URL, s_pms_base_url);
		dict_write_cstring(out_iter, MESSAGE_KEY_SONARR_PORT, s_pms_sonarr_port);
		dict_write_cstring(out_iter, MESSAGE_KEY_SONARR_API, s_pms_sonarr_api_key);
		dict_write_cstring(out_iter, MESSAGE_KEY_RADARR_PORT, s_pms_radarr_port);
		dict_write_cstring(out_iter, MESSAGE_KEY_RADARR_API, s_pms_radarr_api_key);
		result = app_message_outbox_send();
		if(result != APP_MSG_OK) 
		{
			APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending outbox message");
			return;
		}
	} else 
	{
		APP_LOG(APP_LOG_LEVEL_ERROR, "outbox unreachable");
		return;
	}
	persist_write_bool(MESSAGE_KEY_PMS_IS_CONFIGURED, true);
}

static void read_stored_values() 
{
	persist_read_string(MESSAGE_KEY_SERVER_URL, s_pms_base_url, 256);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "stored url: %s", s_pms_base_url);
	persist_read_string(MESSAGE_KEY_SONARR_PORT, s_pms_sonarr_port, 7);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "stored sonarr port: %s", s_pms_sonarr_port);
	persist_read_string(MESSAGE_KEY_SONARR_API, s_pms_sonarr_api_key, 34);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "stored sonarr api: %s", s_pms_sonarr_api_key);
	persist_read_string(MESSAGE_KEY_RADARR_PORT, s_pms_radarr_port, 7);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "stored radarr port: %s", s_pms_radarr_port);
	persist_read_string(MESSAGE_KEY_RADARR_API, s_pms_radarr_api_key, 34);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "stored radarr api: %s", s_pms_radarr_api_key);

	pms_verify_setup();
}

static void pms_error_response_handler(char *error_message) 
{
	vibes_long_pulse();
	switch (s_mode) 
	{
		case NONE:
			text_layer_set_text(s_text_layer, error_message);
			return;
			break;
		case SONARR: 
			text_layer_set_text(s_text_layer, error_message);
			return;
			break;
		case RADARR:
			text_layer_set_text(s_text_layer, error_message);
			return;
			break;
		case DICTATION:
			return;
			break;

		case DICTATION:
			return;
			break;
		case MENU:
			s_mode = NONE;
			deinitialize_menu();
			return;
			break;
		case PROCESS:
			s_mode = NONE;
			deinitialize_menu();
			return;
			break;
	}
}


//static void pms_initialize_request(int mode) 
//{
//	static enum modes s_request_mode;
//	s_request_mode = mode;
//	if (s_js_ready == true) 
//	{
//		DictionaryIterator *out_iter;
//		AppMessageResult result = app_message_outbox_begin(&out_iter);
//		if (result == APP_MSG_OK) 
//		{
//			app_message_outbox_send();
//		}
//		const int interval = 1000;
//		s_timeout_timer = app_timer_register(interval, timeout_timer_handler, NULL); 
//	}
//	if (s_timeout_timer) 
//	{
//		s_timeout_timer = NULL;
//	}
//}

static bool pms_request_handler(int choice) 
{   
	enum modes mode = get_mode();	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "sending choice: %d", choice);
	DictionaryIterator *out_iter;
	AppMessageResult result = app_message_outbox_begin(&out_iter);
	if (result == APP_MSG_OK) 
	{
			int value = 1;
			switch (mode) 
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
					return false;
					break;
				case MENU:
					dict_write_int(out_iter, MESSAGE_KEY_PMS_CHOICE, &choice, sizeof(int), true);
					dict_write_cstring(out_iter, MESSAGE_KEY_PMS_REQUEST, NULL);
					return false;
					break;
				case PROCESS:
					return false;
					break;
			} 
		APP_LOG(APP_LOG_LEVEL_DEBUG, "%d", choice);
		result = app_message_outbox_send();
		if(result != APP_MSG_OK) 
		{
			APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending outbox message");
			return false;
		} else 
		{
			switch (mode) 
			{
				case NONE:
					return;
					break;
				case SONARR:
					break;
				case RADARR:
					break;
				case DICTATION:
					return;
					break;
				case MENU:
					deinitialize_menu(); 
					return true;
					break;
				case PROCESS:
					return;
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

//bool blnRunRoutines(enum s_modes s_mode) 
//{
//	switch (s_mode) 
//	{
//		case NONE:
//			break;
//		case SONARR:
//			break;
//		case RADARR:
//			break;
//		case DICTATION:
//			break;
//		case MENU:
//			break;
//		case PROCESS:
//			break;
//	}
//}

bool blnSendChoice(int choice) 
{
	return pms_send_choice(choice);
}

bool blnInitializeRequestHandler(int choice)
{
	return pms_request_handler(choice);
}
