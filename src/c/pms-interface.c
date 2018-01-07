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

static Window *s_window;
static TextLayer *s_text_layer;
static DictationSession *s_dictation_session; 
static char s_last_text[512];
static char *s_transcription_header;
static GRect s_bounds;
static MenuLayer *s_menu_layer;

//*********************************************************************************************
static bool deinitialize_menu();
static bool pms_init_cards();
static bool pms_deinit_cards()
{
	text_layer_destroy(s_text_layer);
	dictation_session_destroy(s_dictation_session);
	return true;
}

bool pms_error_response_handler(char *error_message) 
{
	vibes_long_pulse();
	switch (int_get_mode()) 
	{
		case NONE:
			text_layer_set_text(s_text_layer, error_message);
			return true;
			break;
		case SONARR: 
			text_layer_set_text(s_text_layer, error_message);
			return true;
			break;
		case RADARR:
			text_layer_set_text(s_text_layer, error_message);
			return true;
			break;
		case DICTATION:
			return false;
			break;
		case MENU:
			set_mode(NONE);
			deinitialize_menu();
			return true;
			break;
		case PROCESS:
			set_mode(NONE);
			deinitialize_menu();
			return true;
			break;
	}
	APP_LOG(APP_LOG_LEVEL_ERROR, "Mode not found pms_error_response_handler()");
	return false;
}


static void select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) 
{

	APP_LOG(APP_LOG_LEVEL_DEBUG, "SELECTED!");
	int choice = (int)cell_index->row; 
	if (!pms_request_handler(&choice))
	{
    	APP_LOG(APP_LOG_LEVEL_ERROR, "blnSendChoice returned false.");
	}	
	else 
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Successfully sent request index %d", choice);
	}
}

void pms_deinit(void) 
{
	window_destroy(s_window);
	window_stack_pop_all(true);
}

static void dictation_session_callback(DictationSession *session, DictationSessionStatus status, char *transcription, void *context) 
{
	if(status == DictationSessionStatusSuccess) 
	{
		pms_request_handler(NULL);
	} 
	else 
	{
		static char s_failed_buff[128];
		snprintf(s_failed_buff, sizeof(s_failed_buff), "Transcription failed.\n\nError ID:\n%d", (int)status);
		APP_LOG(APP_LOG_LEVEL_ERROR, "Transcription failed: %s", s_failed_buff);
		text_layer_set_text(s_text_layer, "\n\nPress Up to \nAdd a Show\n\n\n\nPress Down to\nAdd a Movie");
	}
}



static bool initialize_menu() 
{
	Layer *window_layer = window_get_root_layer(s_window); 
	layer_remove_from_parent(text_layer_get_layer(s_text_layer));
	text_layer_destroy(s_text_layer);
	s_menu_layer = menu_layer_create(s_bounds);
	menu_layer_set_click_config_onto_window(s_menu_layer, s_window);
	menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) 
			{
			.get_num_rows = get_num_rows_callback,
			.draw_row = draw_row_callback,
			.get_cell_height = get_cell_height_callback,
			.select_click = select_callback,
			});
	menu_layer_set_click_config_onto_window(s_menu_layer, s_window);
	layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
	set_mode(MENU);
	return true;
}

static void pms_select_click_handler(ClickRecognizerRef recognizer, void *context) 
{
	switch (int_get_mode()) 
	{
		case NONE:
			return;
			break;
		case SONARR:
			s_transcription_header = "\n\nSearching Show:\n\n%s";
			dictation_session_start(s_dictation_session);
			break;
		case RADARR:
			s_transcription_header = "\n\nSearching Movie:\n\n%s";
			dictation_session_start(s_dictation_session);
			break;      
		case DICTATION:
			return;
			break;
		case MENU:
			APP_LOG(APP_LOG_LEVEL_ERROR, "Click Handler Error");
			return;
			break;
		case PROCESS:
			return;
			break;
	}
}

static void pms_up_click_handler(ClickRecognizerRef recognizer, void *context) 
{
	switch (int_get_mode()) 
	{
		case NONE:
			text_layer_set_text(s_text_layer, "\n\n\n\n\nShow:\nPress Select to Dictate");
			set_mode(SONARR);
			pms_request_handler(NULL);
			return;
			break;
		case SONARR:
			pms_request_handler(NULL);
			return;
			break;
		case RADARR:
			text_layer_set_text(s_text_layer, "\n\n\n\n\nShow:\nPress Select to Dictate");
			set_mode(SONARR);
			pms_request_handler(NULL);
			return;
			break;
		case DICTATION:
			return;
			break;
		case MENU:
			APP_LOG(APP_LOG_LEVEL_ERROR, "Click Handler Error");
			return;
			break;
		case PROCESS:
			return;
			break;
	}
}

static void pms_down_click_handler(ClickRecognizerRef recognizer, void *context) 
{
	switch (int_get_mode()) 
	{
		case NONE:

			set_mode(RADARR);
			pms_request_handler(NULL);
			return;
			break;
		case SONARR:
			text_layer_set_text(s_text_layer, "\n\n\n\n\nMovie:\nPress Select to Dictate");
			set_mode(RADARR);
			pms_request_handler(NULL);
			return;
			break;
		case RADARR:
			pms_request_handler(NULL);
			return;
			break;
		case DICTATION:
			return;
			break;
		case MENU:
			APP_LOG(APP_LOG_LEVEL_ERROR, "Click Handler Error");
			return;
			break;
		case PROCESS:
			return;
			break;
	}
}

static void pms_back_click_handler(ClickRecognizerRef recognizer, void *context) 
{
	switch (int_get_mode()) 
	{
		case NONE:
			if (!pms_deinit_cards())
			{
				APP_LOG(APP_LOG_LEVEL_ERROR, "pms_deinit_cards failed.");
			}
			pms_deinit();
			break;
		case SONARR:
			if (!pms_deinit_cards())
			{
				APP_LOG(APP_LOG_LEVEL_ERROR, "pms_deinit_cards failed.");
			}
			pms_deinit();
			break;
		case RADARR:
			if (!pms_deinit_cards())
			{
				APP_LOG(APP_LOG_LEVEL_ERROR, "pms_deinit_cards failed.");
			}
			pms_deinit();
		case DICTATION:
			return;
			break;
		case MENU:
			deinitialize_menu();
			set_mode(NONE);
			{
				APP_LOG(APP_LOG_LEVEL_ERROR, "failed to set mode in pms_down_click_handler();.");
			}
			pms_init_cards();
			break;
		case PROCESS:
			break;

	}
} 

static void pms_click_config_provider(void *context) 
{
	window_single_click_subscribe(BUTTON_ID_SELECT, pms_select_click_handler);
	window_single_click_subscribe(BUTTON_ID_UP, pms_up_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, pms_down_click_handler);
}



static void pms_window_load(Window *window) 
{
	if (!pms_init_cards())
	{
		APP_LOG(APP_LOG_LEVEL_ERROR, "pms_init_cards() failed in pms_window_load();");
	}


}

static void pms_window_unload(Window *window) 
{
		text_layer_destroy(s_text_layer);
		dictation_session_destroy(s_dictation_session);
		menu_layer_destroy(s_menu_layer);
}

static bool pms_init_cards() 
{
	Layer *window_layer = window_get_root_layer(s_window);
	s_bounds = layer_get_bounds(window_layer);
	s_text_layer = text_layer_create(s_bounds);
	text_layer_set_overflow_mode(s_text_layer, GTextOverflowModeWordWrap);
	text_layer_set_background_color(s_text_layer, GColorBlack);
	text_layer_set_text_color(s_text_layer, GColorGreen);
	text_layer_set_text(s_text_layer, "\n\nPress Up to \nAdd a Show\n\n\n\nPress Down to\nAdd a Movie");
	text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
	layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
	window_set_click_config_provider(s_window, pms_click_config_provider);
	//move back to pms.c
	 s_dictation_session = dictation_session_create(sizeof(s_last_text), dictation_session_callback, NULL);
	set_mode(NONE); 
	return true;
}

bool pms_init() 
{
	s_window = window_create();
	window_set_click_config_provider(s_window, pms_click_config_provider);
	window_set_window_handlers(s_window, (WindowHandlers) 
			{
			.load = pms_window_load,
			.unload = pms_window_unload,
			});
	const bool animated = true;
	window_stack_push(s_window, animated);
	return true;

}


static bool deinitialize_menu() 
{
	layer_remove_from_parent(menu_layer_get_layer(s_menu_layer));
	menu_layer_destroy(s_menu_layer);
	return pms_init_cards();
		

}

bool menu_initializer()
{
	if (!initialize_menu())
	{
		APP_LOG(APP_LOG_LEVEL_ERROR, "initialize_menu() failed");
		return false;
	}
	return true;
}


