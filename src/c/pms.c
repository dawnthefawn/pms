
#ifndef PMS_CLIENT
#define PMS_CLIENT
#include "pms-client.h"
#endif
#ifndef PMS_INTERFACE
#define PMS_INTERFACE
#include "pms-interface.h"
#endif
#ifndef PMS_ROUTINES
#define PMS_ROUTINES
#include "pms-routines.h"
#endif
#ifndef PMS_DATA
#define PMS_DATA
#include "pms-data.h"
#endif
#ifndef CORE_LIBRARIES_INCLUDED
#define CORE_LIBRARIES_INCLUDED
#include <stdio.h>
#include <string.h>
#endif




//*********************************************************************************************

int main(void) 
{
	
	if (!initialize_client())
	{
		return -1;
	}
	app_event_loop();
	pms_deinit();
	return 1;
}
