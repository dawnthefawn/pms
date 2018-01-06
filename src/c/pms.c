#include "pms-client.c"




//*********************************************************************************************

int main(void) 
{
	pms_init();
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);
	app_event_loop();
	pms_deinit();
}
