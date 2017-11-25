#include <pebble.h>
#define PMS_BASE_URL = "http://192.168.1.100:"
#define PMS_SONARR_ROOT = "8989/api/"
static Window *s_window;
static TextLayer *s_text_layer;
static MenuLayer *s_main_menu;
static DictationSession *s_dictation_session;
static char s_last_text[512];

//static MenuIndex *s_cell_index;

enum pmsmode{
	PMS_MENU,
	PMS_DICTATE,
	PMS_REQUEST,
	PMS_SUCCESS,
	PMS_FAILURE,
};

enum pmsmode current_mode;

}
enum menu_items {
	PMS_SONARR,
	PMS_RADARR,
};

//static void sonarr_dictation_deinit() {
//     dictation_session_destroy(s_dictation_session);
//}

static void dictation_session_callback(DictationSession *session, DictationSessionStatus status, char *transcription, void *context) {
     APP_LOG(APP_LOG_LEVEL_INFO, "Dictation Status: %d", (int)status);
}






static void menu_draw_row_callback(GContext* ctx, const Layer *cell_layer, MenuIndex *cell_index, void *data) {

     switch (cell_index->row) {
          case 0:
                menu_cell_title_draw(ctx, cell_layer, "Add a Show");
                break;
          case 1:
                menu_cell_title_draw(ctx, cell_layer, "Add a Movie - N/A");
                break;
     }

}



static void pms_voice_command(DictationSession *dictation) {
  dictation_session_start(dictation);
  
  if(status == DictationSessionStatusSuccess) {
        snprintf(s_last_text, sizeof(s_last_text), "Add show:\n\n%s", transcription);
        text_layer_set_text(s_text_layer, s_last_text);
        dictation_session_stop(dictation);
} else {
    static char s_failed_buff[128];
    snprintf(s_failed_buff, sizeof(s_failed_buff), "Transcription Failed!\n\nReason:\n%d", (int)status);
    text_layer_set_text(s_text_layer, s_failed_buff);
    dictation_session_stop(dictation);
	}
}




static void pms_select_click_handler(ClickRecognizerRef recognizer, void *context, int *mode) {
  switch (current_mode) {
      case PMS_MENU:
	  pms_voice_command(s_dictation_session);
	  break;
      case PMS_DICTATE:
          dictation_session_stop(s_dictation_session);
	  break;
	}
}

static void pms_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  switch (current_mode) {
    case PMS_MENU:
	menu_layer_set_selected_next(s_main_menu, true, MenuRowAlignCenter, true);
        break;

	}
}

static void pms_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  switch (current_mode) {
      case PMS_MENU:  
	menu_layer_set_selected_next(s_main_menu, false, MenuRowAlignCenter, true);
        break;
	}
}


static void pms_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, pms_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, pms_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, pms_down_click_handler);
}

static void pms_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  s_main_menu = menu_layer_create(bounds);
  s_text_layer = text_layer_create(GRect(0, bounds.size.h, 0, bounds.size.w));
//  text_layer_set_text(s_text_layer, "Press a button");
 // text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, menu_layer_get_layer(s_main_menu));
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
}

static void pms_window_unload(Window *window) {
     menu_layer_destroy(s_main_menu);
     text_layer_destroy(s_text_layer);
}

static void pms_init(void) {
  current_mode = PMS_MENU;
  s_window = window_create();
  window_set_click_config_provider(s_window, pms_click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = pms_window_load,
    .unload = pms_window_unload,
  });
  const bool animated = true;
  window_stack_push(s_window, animated);
  menu_layer_set_callbacks(s_main_menu, NULL, (MenuLayerCallbacks) {
	.draw_row = menu_draw_row_callback,
//	.select_click = pms_select_click_handler,

	});
    

  s_dictation_session = dictation_session_create(sizeof(s_last_text), dictation_session_callback, NULL);
}


static void pms_deinit(void) {
  window_destroy(s_window);
  
  dictation_session_destroy(s_dictation_session);
}

int main(void) {
  pms_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  pms_deinit();
}
