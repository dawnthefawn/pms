#include <pebble.h>
#include <stdio.h> 
#include <string.h>
#include <pms-messages.c>
#include <pms-routines.c>


static Window *s_window;
static TextLayer *s_text_layer;
static DictationSession *s_dictation_session; 
static char s_last_text[512];
static char *s_transcription_header;
static GRect s_bounds;

enum modes 
{
	NONE,
	SONARR,
	RADARR,
	DICTATION,
	MENU,
	PROCESS,
};



//*********************************************************************************************
void cancel_timer();
bool blnRunRoutines();
int get_pms_response_items();
int get_mode();
bool set_mode();
bool blnSendChoice();

static uint16_t get_num_rows_callback(MenuLayer *menu_layer, uint16_t section_index, void *context) 
{
	const uint16_t numrows = get_pms_response_items();
	return numrows;
}

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) 
{
	static char s_buff[16];
	snprintf(s_buff, sizeof(s_buff), "%s", s_pms_response[(int)cell_index->row]);
	menu_cell_basic_draw(ctx, cell_layer, s_buff, NULL, NULL);
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) 
{
	const int16_t cell_height = 44;
	return cell_height;
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) 
{
	APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped! "); 
}  

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) 
{
	APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!"); 
	cancel_timer();
}  


static void outbox_sent_callback(DictionaryIterator *iterator, void *context) 
{
	APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}
static void pms_deinit(void) 
{
	window_destroy(s_window);
	window_stack_pop_all(true);
}

static void dictation_session_callback(DictationSession *session, DictationSessionStatus status, char *transcription, void *context) 
{
	if(status == DictationSessionStatusSuccess) 
	{
		snprintf(s_last_text, sizeof(s_last_text), transcription);
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Processed request as: %p", s_last_text);
		pms_handle_request();
	} else 
	{
		static char s_failed_buff[128];
		snprintf(s_failed_buff, sizeof(s_failed_buff), "Transcription failed.\n\nError ID:\n%d", (int)status);
		APP_LOG(APP_LOG_LEVEL_ERROR, "Transcription failed: %s", s_failed_buff);
		text_layer_set_text(s_text_layer, "\n\nPress Up to \nAdd a Show\n\n\n\nPress Down to\nAdd a Movie");
	}
}

static void pms_init_cards() 
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
	//  s_dictation_session = dictation_session_create(sizeof(s_last_text), dictation_session_callback, NULL);
	mode = NONE;
}

static void deinitialize_menu() 
{
	layer_remove_from_parent(menu_layer_get_layer(s_menu_layer));
	menu_layer_destroy(s_menu_layer);
	pms_init_cards();
}

static void initialize_menu() 
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
	mode= MENU;
}

static void pms_select_click_handler(ClickRecognizerRef recognizer, void *context) 
{
	switch (mode) 
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
	switch (mode) 
	{
		case NONE:
			text_layer_set_text(s_text_layer, "\n\n\n\n\nShow:\nPress Select to Dictate");
			mode = SONARR;
			pms_initialize_request();
			return;
			break;
		case SONARR:
			pms_initialize_request();
			return;
			break;
		case RADARR:
			text_layer_set_text(s_text_layer, "\n\n\n\n\nShow:\nPress Select to Dictate");
			mode = SONARR;
			pms_initialize_request();
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
	switch (mode) 
	{
		case NONE:

			//pms_initialize_request();
			return;
			break;
		case SONARR:
			text_layer_set_text(s_text_layer, "\n\n\n\n\nMovie:\nPress Select to Dictate");
			mode = RADARR;
			blnInitializeRequestHandler(NULL, mode);
			return;
			break;
		case RADARR:
			pms_initialize_request();
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
	switch (mode) 
	{
		case NONE:
			pms_deinit_cards();
			pms_deinit();
			break;
		case SONARR:
			pms_deinit_cards();
			pms_deinit();
			break;
		case RADARR:
			pms_deinit_cards();
			pms_deinit();
		case DICTATION:
			return;
			break;
		case MENU:
			deinitialize_menu();
			mode = NONE;
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
	pms_init_cards();
}

static void pms_window_unload(Window *window) 
{
	text_layer_destroy(s_text_layer);
	dictation_session_destroy(s_dictation_session);
	menu_layer_destroy(s_menu_layer);
}

static void pms_init(void) 
{
	persist_write_bool(MESSAGE_KEY_PMS_IS_CONFIGURED, false);
	s_js_ready = false;
	s_window = window_create();
	window_set_click_config_provider(s_window, pms_click_config_provider);
	window_set_window_handlers(s_window, (WindowHandlers) 
			{
			.load = pms_window_load,
			.unload = pms_window_unload,
			});
	const bool animated = true;
	window_stack_push(s_window, animated);
	app_message_register_inbox_received(inbox_received_callback);
	app_message_register_inbox_dropped(inbox_dropped_callback);
	app_message_register_outbox_failed(outbox_failed_callback);
	app_message_register_outbox_sent(outbox_sent_callback);  
	const int inbox_size = 512;
	const int outbox_size = 512;
	app_message_open(inbox_size, outbox_size);
	s_response_sent = false;
}
static void pms_click_config_provider();


static void pms_deinit_cards() 
{
	text_layer_destroy(s_text_layer);
	dictation_session_destroy(s_dictation_session);
}


static void select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) 
{
	APP_LOG(APP_LOG_LEVEL_DEBUG, "SELECTED!");
	if (blnSendChoice((int)cell_index->row + 1) == false 
	{
    	APP_LOG(APP_LOG_LEVEL_ERROR, "blnSendChoice returned false.");
	}	
	else 
	{
		APP_LOG(APP_LOG_LEVEL_DEBUG, "Successfully sent request index %d", (int)cell_index->row + 1);
	}
}



int main(void) 
{
	pms_init();
	APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);
	app_event_loop();
	pms_deinit();
}
