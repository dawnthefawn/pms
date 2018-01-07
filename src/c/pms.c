#ifndef PMS_CLIENT
#define PMS_CLIENT
#include <pms-client.h>
#endif
#ifndef PMS_ROUTINES
#define PMS_ROUTINES
#include <pms-routines.h>
#endif
#ifndef PMS_DATA
#define PMS_DATA
#include <pms-data.h>
#endif
#ifndef CORE_LIBRARIES_INCLUDED
#define CORE_LIBRARIES_INCLUDED
#include <pebble.h>
#include <stdio.h>
#include <string.h>
#endif




//*********************************************************************************************

int main(void) 
{
	
	if (!pms_init())
	{
		return -1;
	}
	register_app_message_callbacks();
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);
	app_event_loop();
	pms_deinit();
	return 1;
}
