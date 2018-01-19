#ifndef CORE_LIBRARIES_INCLUDED
#define CORE_LIBRARIES_INCLUDED
#include <stdio.h>
#include <string.h>
#endif
#ifndef PEBBLE_INCLUDED
#define PEBBLE_INCLUDED
#include <pebble.h>
#endif


static int s_pms_response_index;
static char s_pms_response[8][20];
static int s_pms_response_items;
static bool *s_response_sent;
static bool s_js_ready;
static char s_last_text[512];
static uint8_t *s_img_data;
static int s_img_size;

static char s_pms_sonarr_api_key[PERSIST_DATA_MAX_LENGTH];
static char s_pms_base_url[PERSIST_DATA_MAX_LENGTH]; 
static char s_pms_sonarr_port[PERSIST_DATA_MAX_LENGTH];
static char s_pms_radarr_api_key[PERSIST_DATA_MAX_LENGTH];
static char s_pms_radarr_port[PERSIST_DATA_MAX_LENGTH];
static char s_pms_sms_port[PERSIST_DATA_MAX_LENGTH];

//static bool s_make_request_to_sms;

enum modes 
{
	NONE,
	SONARR,
	RADARR,
	DICTATION,
	MENU,
	PROCESS,
};

static enum modes s_mode;
//**************************************************************************************************************
void sos_pulse()
{
	vibes_cancel();
	static const uint32_t segments[] = {100, 100, 100, 100, 100, 100, 250, 100, 250, 100, 100, 100, 100, 100, 100};
	VibePattern pms_sos = {
		.durations = segments,
		.num_segments = ARRAY_LENGTH(segments),
	};
	vibes_enqueue_custom_pattern(pms_sos);
}

bool bool_log_error(char *text, char *function, int *value, bool zerovalue)
{
	if (!text)
	{
		APP_LOG(APP_LOG_LEVEL_ERROR, "No text provided to bool_log_error from %s", function);
		return false;
	}
	if (!function)
	{
		APP_LOG(APP_LOG_LEVEL_ERROR, "Context was not provided to bool_log_error()");
		return false;
	}	

	if (value)
	{
		APP_LOG(APP_LOG_LEVEL_ERROR, "%s: %d from %s", text, value, function);
		return true;
	}
	else if (zerovalue)
	{
		APP_LOG(APP_LOG_LEVEL_ERROR, "%s: %d from %s", text, 0, function);
		return true;
	}
	else
	{
		APP_LOG(APP_LOG_LEVEL_ERROR, "%s, from %s", text, function);
		return true;
	}
}


void read_stored_values() 
{
	persist_read_string(MESSAGE_KEY_SERVER_URL, s_pms_base_url, 256);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "stored url: %s", s_pms_base_url);
	persist_read_string(MESSAGE_KEY_SONARR_PORT, s_pms_sonarr_port, 8);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "stored sonarr port: %s", s_pms_sonarr_port);
	persist_read_string(MESSAGE_KEY_SONARR_API, s_pms_sonarr_api_key, 33);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "stored sonarr api: %s", s_pms_sonarr_api_key);
	persist_read_string(MESSAGE_KEY_RADARR_PORT, s_pms_radarr_port, 8);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "stored radarr port: %s", s_pms_radarr_port);
	persist_read_string(MESSAGE_KEY_RADARR_API, s_pms_radarr_api_key, 33);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "stored radarr api: %s", s_pms_radarr_api_key);
	persist_read_string(MESSAGE_KEY_SMS_PORT, s_pms_sms_port, 8);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "stored sms port: %s", s_pms_sms_port);
}

bool bool_set_img_size(int *img_size, bool reset)
{
	char *function = "bool_set_img_size(int *img_size, bool reset)";

	if (reset)
	{
		img_size = 0;
		return true;
	}
	if (!img_size)
	{
		if (!bool_log_error("Error in bool_set_img_size(): no img_size provided", function, NULL, false))
		{
			sos_pulse();
		}
		return false;
	}
	s_img_size = (int)img_size;
	s_img_data = (uint8_t*)malloc(s_img_size * sizeof(uint8_t));
	return true;
}

int int_get_img_size()
{
	return s_img_size;
}

bool bool_set_chunk_data(uint8_t *chunk_data, int chunk_size, int index)
{
	char *function = "bool_set_chunk_data(uint8_t *chunk_data, int chunk_size, int index)";

	if (!chunk_data)
	{
		if (!bool_log_error("Error in fnctmplt(): no chunk_data provided", function, NULL, false))
		{
			sos_pulse();
		}
		return false;
	}
	if (!chunk_size)
	{

		if (!bool_log_error("Error in fnctmplt(): no chunk_size provided", function, NULL, false))
		{
			sos_pulse();
		}
		return false;
	}
	if (!index)
	{
		if (!bool_log_error("Error in fnctmplt(): no template provided", function, NULL, false))
		{
			sos_pulse();
		}
		return false;
	}
	memcpy(&s_img_data[index], chunk_data, chunk_size);
	return true;
}

bool bool_set_sms_port(char *sms_port)
{
	char *function = "bool_set_sms_port(char *sms_port)";

	if (!sms_port)
	{
		if (!bool_log_error("error in bool_set_sms_port(): No sms_port provided to function", function, NULL, false))
		{
			sos_pulse();
		}
		return false;
	}	
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Setting sms port to: %s", sms_port);
	strcpy(s_pms_sms_port, sms_port);
	persist_write_string(MESSAGE_KEY_SMS_PORT, sms_port);
	return true;
}

char * str_get_sms_port()
{
	return s_pms_sms_port;
}

bool * bool_get_response_sent() 
{
	return s_response_sent;
}

void set_response_sent(bool *response_sent)
{
	s_response_sent = response_sent;
}
	

bool bool_set_last_text(char *transcription)
{
	char *function = "bool_set_last_text(char *transcription)";

	if (transcription)
	{
		snprintf(s_last_text, sizeof(s_last_text), transcription);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Processed request as: %p", s_last_text);
		return true;
	}
	else
	{
		if (!bool_log_error("no transcription provided to bool_set_last_text()", function, NULL, false))
		{
			sos_pulse();
		}
		return false;
	}
}

char * str_get_last_text()
{
	return s_last_text;

}


void set_js_ready(bool ready)
{
	s_js_ready = ready;
}

bool bool_get_js_ready()
{
	 return s_js_ready;
}

char * str_response_at_index(MenuIndex *cell_index)
{
	char *function = "str_response_at_index(MenuIndex *cell_index)";

	char *response; 
	if (cell_index)
	{
		response = s_pms_response[(int)cell_index->row];
	}
	else
	{

		if (!bool_log_error("str_response_at_index(): cell_index not provided, unable to process", function, NULL, false))
		{
			sos_pulse();
		}
		response = "ERROR";
	}
	return response;
}

bool bool_set_index(int index, bool reset)
{
	if (reset)
	{
		s_pms_response_index = 0;
		return true;
	}
	else
	{
		if (index)
		{
			s_pms_response_index = index - 1;
			return true;
		}

	}
	return false;
}


int int_get_response_index() 
{
	 return s_pms_response_index;
}

bool bool_set_response_at_index(int index, char *response)
{
	char *function = "bool_set_response_at_index(int index, char *response)";

	if (response)
	{
		if (index > 8)
		{
			if (!bool_log_error("Index over expexted bounds: ", function, &index, false))
			{
				sos_pulse();
			}
			return false;
		}
		
		if (!(s_pms_response_index == index))
		{
			if (!bool_log_error("Error, provided index was not expected index", function, NULL, false))
			{
				sos_pulse();
			}
			char *error = "INDEX ERROR";
			strcpy(s_pms_response[s_pms_response_index], error);
			if (s_pms_response_index < index)
			{
				if (!bool_set_index(s_pms_response_index + 1, false))
				{
					if (!bool_log_error("failed to increment response index", function, NULL, false))
					{
						sos_pulse();
					}
					return false;
				}
				return bool_set_response_at_index(int_get_response_index(), response);
			}
			else
			{
				if (!bool_log_error("stored index larger than index received in call, fatal error", function, NULL, false))
				{
					sos_pulse();
				}
				return false;
			}
		}
		else
		{	
		 	APP_LOG(APP_LOG_LEVEL_DEBUG, "Writing response: %s", response);
			strcpy(s_pms_response[s_pms_response_index],  response);
			APP_LOG(APP_LOG_LEVEL_DEBUG, s_pms_response[s_pms_response_index]);
			return true;
		}
	}
		if (!bool_log_error("no response included in bool_set_response_at_index", function, NULL, false))
		{
			sos_pulse();
		}
		return false;
}


bool bool_set_response_items(int items, bool reset)
{
	char *function = "bool_set_response_items(int items, bool reset)";

	if (reset)
	{
		s_pms_response_items = 0;
		return true;
	}
	if (items)
	{
		s_pms_response_items = items - 1;
		return true;
	}
	if (!bool_log_error("bool_set_response_items() received neither items nor reset", function, NULL, false))
	{
		sos_pulse();
	}
	return false;
}

int int_get_pms_response_items() 
{
	 return s_pms_response_items;
}

int int_get_mode() 
{
	 return s_mode;
}

void set_mode(enum modes mode)
{
	s_mode = mode;
}

char * str_sonarr_api_key()
{
	return s_pms_sonarr_api_key;
}

char * str_radarr_api_key()
{
	return s_pms_radarr_api_key;
}

char * str_sonarr_port() 
{
	return s_pms_sonarr_port;
}

char * str_radarr_port()
{
	return s_pms_radarr_port;
}

char * str_base_url()
{
	return s_pms_base_url;
}

bool bool_set_sonarr_api_key(char *sonarr_api)
{
	char *function = "bool_set_sonarr_api_key(char *sonarr_api)";

	if (!sonarr_api)
	{
		if (!bool_log_error("No sonarr api key provided", function, NULL, false))
		{
			sos_pulse();
		}
		return false;
	}
	int len = (int)strlen(sonarr_api);
	if (!(len == 32))	
	{
		if (!bool_log_error("invalid sonarr api key provided, len: ", function, &len, false))
		{
			sos_pulse();
		}
		return false;
	}

	strcpy(s_pms_sonarr_api_key, sonarr_api);
	persist_write_string(MESSAGE_KEY_SONARR_API, s_pms_sonarr_api_key);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "sonarr api key set to %s", s_pms_sonarr_api_key);
	return true;
}

bool bool_set_radarr_api_key(char *radarr_api)
{
	char *function = "bool_set_radarr_api_key(char *radarr_api)";

	if (!radarr_api)
	{
		if (!bool_log_error("No radarr api provided to bool_set_radarr_api_key()", function, NULL, false))
		{
			sos_pulse();
		}
		return false;
	}
	int len = (int)strlen(radarr_api);
	if (!(len == 32))
	{
		if (!bool_log_error("invalid radarr API key length.", function, NULL, false))
		{
			sos_pulse();
		}
		return false;
	}

	strcpy(s_pms_radarr_api_key, radarr_api);
	persist_write_string(MESSAGE_KEY_RADARR_API, s_pms_radarr_api_key);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "radarr api key set to %s", s_pms_radarr_api_key);
	return true;
}

bool bool_set_sonarr_port(char *sonarr_port)
{
	char *function = "bool_set_sonarr_port(char *sonarr_port)";

	if (!sonarr_port)
	{
		if (!bool_log_error("No sonarr port provided", function, NULL, false))
		{
			sos_pulse();
		}
		return false;
	}
	strcpy(s_pms_sonarr_port, sonarr_port);
	persist_write_string(MESSAGE_KEY_SONARR_PORT, s_pms_sonarr_port);
	return true;
}

bool bool_set_radarr_port(char *radarr_port)
{
	if (!radarr_port)
	{
		if (!bool_log_error("no radarr port provided", function, NULL, false))
		{
			sos_pulse();
		}
		return false;
	}

	strcpy(s_pms_radarr_port, radarr_port);
	persist_write_string(MESSAGE_KEY_RADARR_PORT, s_pms_radarr_port);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "radarr port set to %s", s_pms_radarr_port);
	return true;
}

bool bool_set_base_url(char *base_url)
{
	if (!base_url)
		if (!bool_log_error("bool_set_base_url(): no base_url provided", function, NULL, false))
				{
			sos_pulse()return;
				}
		return false;
	}
	strcpy(s_pms_base_url, base_url);
	persist_write_string(MESSAGE_KEY_SERVER_URL, s_pms_base_url);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "server url is set to %s", s_pms_base_url);
	return true;
}


