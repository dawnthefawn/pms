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

bool set_mode(enum modes mode)
{
	s_mode = mode;
	return true;
}
