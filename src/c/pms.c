#include <pebble.h>

static Window *s_window;
static TextLayer *s_text_layer;
static MenuLayer *s_main_menu;
static DictationSession *s_dictation_session;
static char s_last_text[512];

static void dictation_session_callback(DictationSession *session, DictationSessionStatus status, char *transcription, void *context) {
     APP_LOG(APP_LOG_LEVEL_INFO, "Dictation Status: %d", (int)status);
}

static void sonarr_dictation_deinit() {
     dictation_session_destroy(s_dictation_session);
}

static void sonarr_dictation_init() {
    s_dictation_session = dictation_session_create(sizeof(s_last_text), dictation_session_callback, NULL);
    dictation_session_start(s_dictation_session);
    if(status == DictationSessionStatusSuccess) {
        snprintf(s_last_text, sizeof(s_last_text), "Add show:\n\n%s", transcription);
        text_layer_set_text(s_text_layer, s_last_text);
        sonarr_dictation_deinit();
} else {
    static char s_failed_buff[128];
    snprintf(s_failed_buff, sizeof(s_failed_buff), "Transcription Failed!\n\nReason:\n%d", (int)status);
    text_layer_set_text(s_text_layer, s_failed_buff);
    sonarr_dictation_deinit();
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

static void menu_select_callback(MenuLayer *menu_layer, MenuIndex *cell_index, void *data) {
     switch (cell_index->row) {
          case 0:
                sonarr_dictation_init();
                break;
          case 1: 
                break;
	}
}

static void prv_select_click_handler(ClickRecognizerRef recognizer, void *context) {
	menu_select_callback(s_main_menu, menu_layer_get_selected_index(s_main_menu), NULL);
}

static void prv_up_click_handler(ClickRecognizerRef recognizer, void *context) {
	menu_layer_set_selected_next(s_main_menu, true, MenuRowAlignCenter, true);
}

static void prv_down_click_handler(ClickRecognizerRef recognizer, void *context) {
	menu_layer_set_selected_next(s_main_menu, false, MenuRowAlignCenter, true);
}

static void prv_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, prv_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, prv_down_click_handler);
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  s_main_menu = menu_layer_create(bounds)
  s_text_layer = text_layer_create(GRect(0, bounds.size.h, 0, bounds.size.w));
//  text_layer_set_text(s_text_layer, "Press a button");
 // text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer);
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
}

static void prv_window_unload(Window *window) {
     menu_layer_destroy(s_menu_layer);
//  text_layer_destroy(s_text_layer);
}

static void prv_init(void) {
  s_window = window_create();
  window_set_click_config_provider(s_window, prv_click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  const bool animated = true;
  window_stack_push(s_window, animated);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
	.draw_row = draw_row_callback,
	.select_click = prv_select_click_handler,
	.up_click = prv_up_click_handler,
	.down_click = prv_down_click_handler,
	});
}

static void prv_deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  prv_init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  prv_deinit();
}
