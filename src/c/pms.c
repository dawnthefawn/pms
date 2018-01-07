#include "pms-client.c"




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
	if (!pms_deinit())
	{
		return -1;
	}
	return 1;
}
