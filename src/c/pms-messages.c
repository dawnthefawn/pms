#include <pebble.h>


static bool s_js_ready;
//**************************************************************************************

static void inbox_received_callback(DictionaryIterator *iter, void *context) {  
				if (!s_js_ready) {
								Tuple *ready_tuple = dict_find(iter, MESSAGE_KEY_JSReady);
								if(ready_tuple) {
												s_js_ready = true;
												s_pms_response_index = 0;
												read_stored_values();
												return;
								}
								APP_LOG(APP_LOG_LEVEL_DEBUG, "inbox_callback_received");
				}
				Tuple *pms_error = dict_find(iter, MESSAGE_KEY_PMS_ERROR);
				if (pms_error) {
								pms_error_response_handler(pms_error->value->cstring);
								return;
				}
				Tuple *pms_success = dict_find(iter, MESSAGE_KEY_PMS_SUCCESS);
				if(pms_success) {
								vibes_double_pulse(); 
								mode = NONE;
								return;
				}
				//  if (`mode == MENU) { 
				Tuple *server_response = dict_find(iter, MESSAGE_KEY_PMS_RESPONSE + s_pms_response_index);
				if (server_response) {
								strcpy(s_pms_response[s_pms_response_index], server_response->value->cstring);
								APP_LOG(APP_LOG_LEVEL_DEBUG, s_pms_response[s_pms_response_index]);
								s_pms_response_index ++;
								if(s_pms_response_index > 8) {
								}
				}
				Tuple *response_sent = dict_find(iter, MESSAGE_KEY_PMS_RESPONSE_SENT);
				if (response_sent) {
								s_pms_response_items = s_pms_response_index;
								s_pms_response_index = 0;
								APP_LOG(APP_LOG_LEVEL_DEBUG, "Received All Items");
								initialize_menu();

				}
				//}

				Tuple *server_url = dict_find(iter, MESSAGE_KEY_SERVER_URL);
				if(server_url) {
								strcpy(s_pms_base_url, server_url->value->cstring);
								persist_write_string(MESSAGE_KEY_SERVER_URL, s_pms_base_url);
								APP_LOG(APP_LOG_LEVEL_DEBUG, "server url is set to %s", s_pms_base_url);
				}
				Tuple *sonarr_port = dict_find(iter, MESSAGE_KEY_SONARR_PORT);
				if(sonarr_port) {
								strcpy(s_pms_sonarr_port, sonarr_port->value->cstring);
								persist_write_string(MESSAGE_KEY_SONARR_PORT, s_pms_sonarr_port);
								APP_LOG(APP_LOG_LEVEL_DEBUG, "sonarr port is set to %s", s_pms_sonarr_port);
				}

				Tuple *sonarr_api = dict_find(iter, MESSAGE_KEY_SONARR_API);
				if (sonarr_api) {
								strcpy(s_pms_sonarr_api_key, sonarr_api->value->cstring);
								persist_write_string(MESSAGE_KEY_SONARR_API, s_pms_sonarr_api_key);
								APP_LOG(APP_LOG_LEVEL_DEBUG, "sonarr api key set to %s", s_pms_sonarr_api_key);
				}
				Tuple *radarr_api = dict_find(iter, MESSAGE_KEY_RADARR_API);
				if (radarr_api) {
								strcpy(s_pms_radarr_api_key, radarr_api->value->cstring);
								persist_write_string(MESSAGE_KEY_RADARR_API, s_pms_radarr_api_key);
								APP_LOG(APP_LOG_LEVEL_DEBUG, "radarr api key set to %s", s_pms_radarr_api_key);
				}
				Tuple *radarr_port = dict_find(iter, MESSAGE_KEY_RADARR_PORT);
				if (radarr_port) {
								strcpy(s_pms_radarr_port, radarr_port->value->cstring);
								persist_write_string(MESSAGE_KEY_RADARR_PORT, s_pms_radarr_port);
								APP_LOG(APP_LOG_LEVEL_DEBUG, "radarr port set to %s", s_pms_radarr_port);
				}
				if(!persist_read_bool(MESSAGE_KEY_PMS_IS_CONFIGURED)) {
								pms_verify_setup();
								APP_LOG(APP_LOG_LEVEL_DEBUG, "attempting to verify setup");
								return;
				}  
				return;  
}

