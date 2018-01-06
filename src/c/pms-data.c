#include <pebble.h>
#include <stdio.h>
#include <string.h>

static int s_pms_response_index;
static char s_pms_response[8][20];
static int s_pms_response_items;
static bool *s_response_sent;
static bool s_js_ready;
static char s_last_text[512];

char s_pms_sonarr_api_key[PERSIST_DATA_MAX_LENGTH];
char s_pms_base_url[PERSIST_DATA_MAX_LENGTH]; 
char s_pms_sonarr_port[PERSIST_DATA_MAX_LENGTH];
char s_pms_radarr_api_key[PERSIST_DATA_MAX_LENGTH];
char s_pms_radarr_port[PERSIST_DATA_MAX_LENGTH];



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

//****************************************************************************************


void read_stored_values() 
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

//	pms_verify_setup();
}

bool bool_get_response_sent() 
{
	return s_response_sent;
}

bool bool_set_response_sent(bool *response_sent)
{
	if(response_sent || !response_sent)
	{
		s_response_sent = response_sent;
		return true;
	}
	else
	{
		APP_LOG(APP_LOG_LEVEL_ERROR, "error setting s_pms_response_sent");
	}
	return false;
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

char str_response_at_index(int index)
{
	return *s_pms_response[index];
}

bool bool_set_response_at_index(int index, char *response)
{
	if (response)
	{
		strcpy(s_pms_response[index],  response);
		s_pms_response_index = index;
		APP_LOG(APP_LOG_LEVEL_DEBUG, s_pms_response[s_pms_response_index]);
		return true;
	}
	APP_LOG(APP_LOG_LEVEL_ERROR, "no response included in bool_set_response_at_index");
	s_pms_response_index = index;
	return false;
}


bool bool_set_index(int index)
{
	if (index)
	{
		s_pms_response_index = index;
		return true;
	}

	else
	{
		return false;
	}
}

int int_get_response_index() 
{
	 return s_pms_response_index;
}

bool bool_set_response_items(int items)
{
	if (items)
	{
		s_pms_response_items = items + 1;
		return true;
	}

	else 
	{
		return false;
	}
}

int int_get_pms_response_items() 
{
	 return s_pms_response_items;
}

int int_get_mode() 
{
	 return s_mode;
}

bool bool_set_mode(int mode)
{
	s_mode = mode;
	return true;
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
	strcpy(s_pms_sonarr_api_key, sonarr_api);
	persist_write_string(MESSAGE_KEY_SONARR_API, s_pms_sonarr_api_key);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "sonarr api key set to %s", s_pms_sonarr_api_key);
	return true;
}

bool bool_set_radarr_api_key(char *radarr_api)
{
	strcpy(s_pms_radarr_api_key, radarr_api);
	persist_write_string(MESSAGE_KEY_RADARR_API, s_pms_radarr_api_key);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "radarr api key set to %s", s_pms_radarr_api_key);
	return true;
}

bool bool_set_sonarr_port(char *sonarr_port)
{
	strcpy(s_pms_sonarr_port, sonarr_port);
	persist_write_string(MESSAGE_KEY_SONARR_PORT, s_pms_sonarr_port);
	return true;
}

bool bool_set_radarr_port(char *radarr_port)
{
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


