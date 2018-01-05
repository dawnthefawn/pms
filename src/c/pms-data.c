#include <pebble.h>


static int s_pms_response_index;
static char s_pms_response[8][20];
static int s_pms_response_items;
static MenuLayer *s_menu_layer;
static bool *s_response_sent;

static char s_pms_sonarr_api_key[PERSIST_DATA_MAX_LENGTH];
static char s_pms_base_url[PERSIST_DATA_MAX_LENGTH]; 
static char s_pms_sonarr_port[PERSIST_DATA_MAX_LENGTH];
static char s_pms_radarr_api_key[PERSIST_DATA_MAX_LENGTH];
static char s_pms_radarr_port[PERSIST_DATA_MAX_LENGTH];



static enum modes s_mode;


enum modes 
{
	NONE,
	SONARR,
	RADARR,
	DICTATION,
	MENU,
	PROCESS,
};
//****************************************************************************************


int get_pms_response_items() 
{
	return s_pms_response_items;
}

int get_mode() 
{
	return s_mode;
}

bool set_mode(int mode)
{
	s_mode = mode;
	return true;
}

char strSonarrAPIKey()
{
	return s_pms_sonarr_api_key;
}

char strRadarrAPIKey()
{
	return s_pms_radarr_api_key;
}

char strSonarrPort() 
{
	return s_pms_sonarr_port;
}

char strRadarrPort()
{
	return s_pms_radarr_port;
}

char strBaseURL()
{
	return s_pms_base_url;
}

bool blnSonarrAPIKey_Set(char *sonarr_api)
{
	s_pms_sonarr_api_key = sonarr_api;
	return true;
}

bool blnRadarrAPIKey_Set(char *radarr_api)
{
	s_pms_radarr_api_key = radarr_api;
	return true;
}

bool blnSonarrPort_Set(char *sonarr_port)
{
	s_pms_sonarr_port = sonarr_port;
	return true;
}

bool blnRadarrPort_Set(char *radarr_port)
{
	s_pms_radarr_port = radarr_port;
	return true;
}

bool blnBaseURL_Set(char *base_url)
{
	s_pms_base_url = base_url;
	return true;
}


