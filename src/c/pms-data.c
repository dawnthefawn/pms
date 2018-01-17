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

//bool bool_get_make_request_to_sms()
//{
//	return s_make_request_to_sms;
//}
//
//bool bool_set_make_request_to_sms(bool state, bool reset)
//{
//	if (reset)
//	{
//		s_make_request_to_sms = false;
//		return true;
//	}
//	if (state)
//	{
//		s_make_request_to_sms = state;
//		return true;
//	}
//	APP_LOG(APP_LOG_LEVEL_ERROR, "error in bool_set_make_request_to_sms(): neither state nor reset were provided.");
//	return false;
//}



bool bool_set_sms_port(char *sms_port)
{
	if (!sms_port)
	{
		APP_LOG(APP_LOG_LEVEL_ERROR, "error in bool_set_sms_port(): No sms_port provided to function");
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
	if (transcription)
	{
		snprintf(s_last_text, sizeof(s_last_text), transcription);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Processed request as: %p", s_last_text);
		return true;
	}
	else
	{
		APP_LOG(APP_LOG_LEVEL_ERROR, "no transcription provided to bool_set_last_text()");
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
	char *response; 
	if (cell_index)
	{
		response = s_pms_response[(int)cell_index->row];
	}
	else
	{

		APP_LOG(APP_LOG_LEVEL_ERROR, "str_response_at_index(): cell_index not provided, unable to process");
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
	if (response)
	{
		if (index > 8)
		{
			APP_LOG(APP_LOG_LEVEL_ERROR, "Index over expexted bounds: %d", index);
			return false;
		}
		
		if (!(s_pms_response_index == index))
		{
			APP_LOG(APP_LOG_LEVEL_ERROR, "Error, provided index was not expected index");
			char *error = "INDEX ERROR";
			strcpy(s_pms_response[s_pms_response_index], error);
			if (s_pms_response_index < index)
			{
				if (!bool_set_index(s_pms_response_index + 1, false))
				{
					APP_LOG(APP_LOG_LEVEL_ERROR, "failed to increment response index");
					return false;
				}
				return bool_set_response_at_index(int_get_response_index(), response);
			}
			else
			{
				APP_LOG(APP_LOG_LEVEL_ERROR, "stored index larger than index received in call, fatal error");
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
		APP_LOG(APP_LOG_LEVEL_ERROR, "no response included in bool_set_response_at_index");
		return false;
}


bool bool_set_response_items(int items, bool reset)
{
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
	APP_LOG(APP_LOG_LEVEL_ERROR, "bool_set_response_items() received neither items nor reset");
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
	if (!sonarr_api)
	{
		APP_LOG(APP_LOG_LEVEL_ERROR, "No sonarr api key provided");
		return false;
	}
	int len = (int)strlen(sonarr_api);
	if (!(len == 32))	
	{
		APP_LOG(APP_LOG_LEVEL_ERROR, "invalid sonarr api key provided, len: %d", len);
		return false;
	}

	strcpy(s_pms_sonarr_api_key, sonarr_api);
	persist_write_string(MESSAGE_KEY_SONARR_API, s_pms_sonarr_api_key);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "sonarr api key set to %s", s_pms_sonarr_api_key);
	return true;
}

bool bool_set_radarr_api_key(char *radarr_api)
{
	if (!radarr_api)
	{
		APP_LOG(APP_LOG_LEVEL_ERROR, "No radarr api provided to bool_set_radarr_api_key()");
		return false;
	}
	int len = (int)strlen(radarr_api);
	if (!(len == 32))
	{
		APP_LOG(APP_LOG_LEVEL_ERROR, "invalid radarr API key length.");
		return false;
	}

	strcpy(s_pms_radarr_api_key, radarr_api);
	persist_write_string(MESSAGE_KEY_RADARR_API, s_pms_radarr_api_key);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "radarr api key set to %s", s_pms_radarr_api_key);
	return true;
}

bool bool_set_sonarr_port(char *sonarr_port)
{
	if (!sonarr_port)
	{
		APP_LOG(APP_LOG_LEVEL_ERROR, "No sonarr port provided");
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
		APP_LOG(APP_LOG_LEVEL_ERROR, "no radarr port provided");
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
	{
		APP_LOG(APP_LOG_LEVEL_ERROR, "bool_set_base_url(): no base_url provided");
		return false;
	}
	strcpy(s_pms_base_url, base_url);
	persist_write_string(MESSAGE_KEY_SERVER_URL, s_pms_base_url);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "server url is set to %s", s_pms_base_url);
	return true;
}


