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
#ifndef PEBBLE_INCLUDED
#define PEBBLE_INCLUDED
#include <pebble.h>
#endif

static Window *s_window;
static TextLayer *s_text_layer;
static DictationSession *s_dictation_session; 
static char s_last_text[512];
static char *s_transcription_header;
static GRect s_bounds;
static MenuLayer *s_menu_layer;
static ClickConfigProvider last_ccp;
//*********************************************************************************************
static bool deinitialize_menu();
static bool pms_init_cards();

bool bool_set_text(char *text, bool reset)
{
	char *function = "bool_set_text()";
	if (reset)
	{
		text_layer_set_text(s_text_layer, "");
		return true;
	}

	if (!text)
	{
		if (!bool_log_error("No text provided to set", function, 0, false))
		{
			sos_pulse();
		}
		return false;
	}
	text_layer_set_text(s_text_layer, text);
	return true;

}
static bool pms_deinit_cards()
{
	if (s_text_layer)
	{
		text_layer_destroy(s_text_layer);
		s_text_layer = NULL;
	}
	if (s_dictation_session)
	{
		dictation_session_destroy(s_dictation_session);
		s_dictation_session = NULL;
	}
	return true;
}

void pms_cards_reset_text()
{
	char *function = "pms_cards_reset_text()";
	APP_LOG(APP_LOG_LEVEL_DEBUG, "resetting text, mode: %d", int_get_mode());
	if (!(int_get_mode() == NONE))
	{
		set_mode(NONE);
		if (!bool_log_error("mode was not reset.", function, 0, false))
		{
			sos_pulse();
		}
	}
	text_layer_set_overflow_mode(s_text_layer, GTextOverflowModeWordWrap);
	text_layer_set_background_color(s_text_layer, GColorBlack);
	text_layer_set_text_color(s_text_layer, GColorGreen);
	if (!bool_get_js_ready())
	{
		if (!bool_set_text("Server not ready, standby", false))
		{
			if (!bool_log_error("Failed to set js not ready text", function, 0, false))
			{
				sos_pulse();
			}
		}
	}
	else
	{
		if (!bool_set_text("\n\nPress Up to \nAdd a Show\n\n\n\nPress Down to\nAdd a Movie", false))
		{

			if (!bool_log_error("Was unable to set text in pms_cards_reset_text()", function, 0, false))
			{
				sos_pulse();
			}
		}
	}
	text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
}


bool pms_error_response_handler(char *error_message) 
{
	char *function = "pms_error_response_handler()";
	vibes_long_pulse();
	switch (int_get_mode()) 
	{
		case NONE:
			if (!bool_set_text(error_message, false))
			{
				if (!bool_log_error("Unable to set error message as text, no error message provided", function, 0, false))
				{
					sos_pulse();
				}
			}
			return true;
			break;
		case SONARR: 
			if (!bool_set_text(error_message, false))
			{
				if (!bool_log_error("Unable to set error message as text, no error message provided", function, 0, false))
				{
					sos_pulse();
				}
			}
			return true;
			break;
		case RADARR:
			if (!bool_set_text(error_message, false))
			{
				if (!bool_log_error("Unable to set error message as text, no error message provided", function, 0, false))
				{
					sos_pulse();
				}
			}
			return true;
			break;
		case DICTATION:
			return false;
			break;
		case MENU:
			set_mode(NONE);
			deinitialize_menu();
			return pms_init_cards();
			break;
		case PROCESS:
			set_mode(NONE);
			deinitialize_menu();
			return pms_init_cards();
			break;
	}
	if (!bool_log_error("Mode not found pms_error_response_handler()", function, 0, false))
	{
		sos_pulse();
	}
	return false;
}


static void select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) 
{
	char *function = "select_callback()";

	APP_LOG(APP_LOG_LEVEL_DEBUG, "SELECTED!");
	int choice = (int)cell_index->row + 1; 
	if (!pms_request_handler(&choice, false))
	{
		if (!bool_set_text("Unable to handle request.", false))
		{

			if (!bool_log_error("pms_request_handler() returned false. ", function, choice, false))
			{
				sos_pulse();
			}
		}
	}	
	else 
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Successfully sent request index %d", choice);
		char buf[48];
		snprintf(buf, sizeof(buf), "Sent Request index %d", choice);
		char *text = buf;
		if (!bool_set_text(text, false))
		{
			if (!bool_log_error("unable to set sent request text", function, 0 , false))
			{
				sos_pulse();
			}
		}
		text = NULL;
		buf[0] = 0;
		set_mode(NONE);
		if (!deinitialize_menu())
		{
			if (!bool_log_error("Failed to Deintialize menu", function, 0, false))
			{
				sos_pulse();
			}
		}	
		if (!pms_init_cards())
		{
			if (!bool_log_error("pms_initialize_cards() failed in select_callback()", function, 0, false))
			{
				sos_pulse();
			}
		}

	}
}

void pms_deinit(void) 
{
	exit_reason_set(APP_EXIT_ACTION_PERFORMED_SUCCESSFULLY);
	if (s_window)
	{
		window_destroy(s_window);
		s_window = NULL;
	}
	window_stack_pop_all(true);
}

static void dictation_session_callback(DictationSession *session, DictationSessionStatus status, char *transcription, void *context) 
{
	char *function = "dictation_session_callback()";
	if(status == DictationSessionStatusSuccess) 
	{

		set_mode(DICTATION);
		bool_set_last_text(transcription);
		if (!pms_request_handler(NULL, false))
		{
			if (!bool_log_error("Failed to handle dictation request ", function, 0, false))
			{
				sos_pulse();
			}
		}
	} 
	else 
	{
		pms_cards_reset_text();
		static char s_failed_buff[128];
		snprintf(s_failed_buff, sizeof(s_failed_buff), "Transcription failed.\n\nError ID:\n%d", (int)status);
		if (!bool_log_error(s_failed_buff, function, 0, false))
		{
			sos_pulse();
		}


	}
}

static void pms_back_click_handler(ClickRecognizerRef recognizer, void *context) 
{
	char *function = "pms_back_click_handler()";
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Selected back! Mode: %d", int_get_mode());
	int m = int_get_mode();
	switch (int_get_mode()) 
	{
		case NONE:
			if (!pms_deinit_cards())
			{
				if (!bool_log_error("pms_deinit_cards failed.", function, 0, false))
				{
					sos_pulse();
				}
			}
			window_stack_pop_all(true);
			break;
		case SONARR:
			pms_cards_reset_text();
			break;
		case RADARR:
			pms_cards_reset_text();
			break;
		case DICTATION:
			pms_cards_reset_text();
			break;
		case MENU:
			if (!deinitialize_menu())
			{
				if (!bool_log_error("deinitialize_menu() failed on back click provider, mode MENU", function, 0, false))
				{
					sos_pulse();
				}
			}
			if (!pms_init_cards())
			{
				if (!bool_log_error("pms_init_cards() failed in back click provider, mode MENU", function, 0, false))
				{
					sos_pulse();
				}
			}
			return;
			break;
		case PROCESS:
			if (!bool_log_error("Click Handler Error, Mode: ", function, m, false))
			{
				sos_pulse();
			}
			set_mode(NONE);
			break;

	}
} 

static void pms_menu_click_config_provider(void *context)
{

	APP_LOG(APP_LOG_LEVEL_DEBUG, "Subscribing to back_click_handler");
	last_ccp(context);
	window_single_click_subscribe(BUTTON_ID_BACK, pms_back_click_handler);
}


static void pms_force_back_button(Window *window, MenuLayer *menu_layer)
{
	last_ccp = window_get_click_config_provider(window);
	window_set_click_config_provider_with_context(window, pms_menu_click_config_provider, menu_layer);
}


static bool initialize_menu() 
{
	char *function = "initialize_menu()";
	Layer *window_layer = window_get_root_layer(s_window); 
	if (!window_layer)
	{
		if (!bool_log_error("failed to return window_layer in initialize_menu()", function, 0, false))
		{
			sos_pulse();
		}
		return false;
	}
	layer_remove_from_parent(text_layer_get_layer(s_text_layer));
	if (s_text_layer)
	{
		text_layer_destroy(s_text_layer);
		s_text_layer = NULL;
	}
	s_menu_layer = menu_layer_create(s_bounds);
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Setting default click config profile");
	menu_layer_set_click_config_onto_window(s_menu_layer, s_window);
	menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) 
			{
			.get_num_rows = get_num_rows_callback,
			.draw_row = draw_row_callback,
			.get_cell_height = get_cell_height_callback,
			.select_click = select_callback,
			});
	layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
	set_mode(MENU);
	pms_force_back_button(s_window, s_menu_layer);
	return true;
}

static void pms_select_click_handler(ClickRecognizerRef recognizer, void *context) 
{
	char *function = "pms_select_click_handler()";
	int m = int_get_mode();
	switch (m)
	{
		case NONE:
			if (!bool_log_error("Click Handler Error, Mode: ", function, m, false))
			{
				sos_pulse();
			}
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
			if (!bool_log_error("Click Handler Error, Mode: ", function, m, false))
			{
				sos_pulse();
			}
			return;
			break;
		case MENU:
			if (!bool_log_error("Click Handler Error, Mode: ", function, m, false))
			{
				sos_pulse();
			}
			return;
			break;
		case PROCESS:
			if (!bool_log_error("Click Handler Error, Mode: ", function, m, false))
			{
				sos_pulse();
			}
			return;
			break;
	}
}

static void pms_up_click_handler(ClickRecognizerRef recognizer, void *context) 
{
	char *function = "pms_up_click_handler()";
	int m = int_get_mode();
	switch (m) 
	{
		case NONE:
			if (!bool_set_text("\n\nUp: Restart Sonarr\n\n\nShow:\nPress Select to Dictate", false))
			{
				if (!bool_log_error("Unable to set sonarr text in pms_up_click_handler()", function, 0, false))
				{
					sos_pulse();
				}
			}
			set_mode(SONARR);
			pms_request_handler(NULL, false);
			return;
			break;
		case SONARR:
			pms_request_handler(NULL, true);
			return;
			break;
		case RADARR:
			if (!bool_set_text("\n\nUp: Restart Sonarr\n\n\nShow:\nPress Select to Dictate", false))
			{
				if (!bool_log_error("Unable to set sonarr text in RADARR mode, pms_up_click_handler()", function, 0, false))
				{
					sos_pulse();
				}
			}
			set_mode(SONARR);
			pms_request_handler(NULL, false);
			return;
			break;
		case DICTATION:
			return;
			break;
		case MENU:
			if (!bool_log_error("Click Handler Error", function, 0, false))
			{
				sos_pulse();
			}
			return;
			break;
		case PROCESS:
			return;
			break;
	}
}

static void pms_down_click_handler(ClickRecognizerRef recognizer, void *context) 
{
	char *function = "pms_down_click_handler()";
	int m = int_get_mode();
	switch (m) 
	{
		case NONE:
			if (!bool_set_text("\n\n\n\n\nMovie:\nPress Select to Dictate\n\nDown: Restart Radarr", false))
			{
				if (!bool_log_error("Unable to set radarr text in mode none at pms_down_click_handler()", function, 0, false))
				{
					sos_pulse();
				}
			}	
			set_mode(RADARR);
			pms_request_handler(NULL,false);
			return;
			break;
		case SONARR:
			if (!bool_set_text("\n\n\n\n\nMovie:\nPress Select to Dictate\n\nDown: Restart Radarr", false))
			{
				if (!bool_log_error("Unable to set radarr text in mode sonarr", function, 0, false))
				{
					sos_pulse();
				}
			}

			set_mode(RADARR);
			pms_request_handler(NULL,false);
			return;
			break;
		case RADARR:
			set_mode(RADARR);
			pms_request_handler(NULL, true);
			return;
			break;
		case DICTATION:
			if (!bool_log_error("Click Handler Error, Mode: ", function, m, false))
			{
				sos_pulse();
			}
			return;
			break;
		case MENU:
			if (!bool_log_error("Click Handler Error, Mode: ", function, m, false))
			{
				sos_pulse();
			}
			return;
			break;
		case PROCESS:
			if (!bool_log_error("Click Handler Error, Mode: ", function, m, false))
			{
				sos_pulse();
			}
			return;
			break;
	}
}




static void pms_cards_click_config_provider(void *context) 
{
	window_single_click_subscribe(BUTTON_ID_SELECT, pms_select_click_handler);
	window_single_click_subscribe(BUTTON_ID_UP, pms_up_click_handler);
	window_single_click_subscribe(BUTTON_ID_DOWN, pms_down_click_handler);
	window_single_click_subscribe(BUTTON_ID_BACK, pms_back_click_handler);
}



static void pms_window_load(Window *window) 
{
	char *function = "pms_window_load()";
	if (!pms_init_cards())
	{
		if (!bool_log_error("pms_init_cards() failed in pms_window_load();", function, 0, false))
		{
			sos_pulse();
		}
	}


}


static void pms_window_unload(Window *window) 
{
	if (s_text_layer)
	{
		text_layer_destroy(s_text_layer);
		s_text_layer = NULL;
	}
	if (s_dictation_session)
	{
		dictation_session_destroy(s_dictation_session);
		s_dictation_session = NULL;
	}
	if (s_menu_layer)
	{
		menu_layer_destroy(s_menu_layer);
		s_menu_layer = NULL;
	}
}


static bool pms_init_cards() 
{
	char *function = "pms_init_cards()";
	Layer *window_layer = window_get_root_layer(s_window);
	if (!window_layer)
	{
		if (!bool_log_error("Error in pms_init_cards(): failed to return window_layer", function, 0, false))
		{
			sos_pulse();
		}
		return false;
	}
	s_bounds = layer_get_bounds(window_layer);
	s_text_layer = text_layer_create(s_bounds);
	if (!s_text_layer)
	{

		if (!bool_log_error("Error in pms_init_cards(): failed to return s_text_layer", function, 0, false))
		{
			sos_pulse();
		}
		return false;
	}
	pms_cards_reset_text();
	layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
	window_set_click_config_provider(s_window, pms_cards_click_config_provider);
	s_dictation_session = dictation_session_create(sizeof(s_last_text), dictation_session_callback, NULL);
	if (!s_dictation_session)
	{

		if (!bool_log_error("Error in pms_init_cards(): failed to return s_dictation_session", function, 0, false))
		{
			sos_pulse();
		}
		return false;
	}
	set_mode(NONE); 
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Finished initializing cards interface, mode: %d", int_get_mode());
	return true;
}

bool pms_init() 
{
	char *function = "pms_init()";
	s_window = window_create();
	window_set_window_handlers(s_window, (WindowHandlers) 
			{
			.load = pms_window_load,
			.unload = pms_window_unload,
			});
	const bool animated = true;
	if (!pms_init_cards())
	{
		if (!bool_log_error("pms_init_cards() failed during pms_init()", function, 0, false))
		{
			sos_pulse();
		}
		return false;
	}
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Pushing Window");
	window_stack_push(s_window, animated);
	return true;
}

bool sms_success_handler(char *msg)
{
	char *error;
	char *function = "sms_success_handler(char *msg)";
	if (!bool_set_text(msg, false))
	{
		char buf[256];
		snprintf(buf, sizeof(buf), "Unable to set error message: %s", msg);
		error = buf;
		if (!bool_log_error(error, function, 0, false))
		{
			sos_pulse();
		}
		vibes_long_pulse();
		error = NULL;
		buf[0] = 0;
		return false;
	}
	vibes_double_pulse();
	return true;
}
static bool deinitialize_menu() 
{
	char *function = "deinitialize_menu()";
	layer_remove_from_parent(menu_layer_get_layer(s_menu_layer));
	if (s_menu_layer)
	{
		menu_layer_destroy(s_menu_layer);
		s_menu_layer = NULL;
	}
	if (!bool_set_response_items(0, true))
	{
		if (!bool_log_error("bool_set_response_items() failed to resetin deinitialize_menu()", function, 0, false))
		{
			sos_pulse();
		}
		return false;
	}
	set_mode(NONE);
	if (!bool_set_index(0, true))
	{

		if (!bool_log_error("bool_set_response_index() failed to resetin deinitialize_menu()", function, 0, false))
		{
			sos_pulse();
		}
		return false;
	}
	return true;
}

bool menu_initializer()
{
	char *function = "menu_initializer()";
	if (!initialize_menu())
	{
		if (!bool_log_error("initialize_menu() failed", function, 0, false))
		{
			sos_pulse();
		}
		return false;
	}
	return true;
}


