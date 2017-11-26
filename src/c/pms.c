#include <pebble.h>
#define PMS_BASE_URL = "http://192.168.1.100:" 
#define PMS_SONARR_ROOT = "8989/api/"
static Window *s_window;
static TextLayer *s_text_layer;
static DictationSession *s_dictation_session; 
static char s_last_text[512];
enum modes {
	NONE,
	SONARR,
	RADARR,
};
static enum modes *s_mode;
static void dictation_session_callback(DictationSession *session, DictationSessionStatus status, 
  char *transcription, void *context) {
  if(status == DictationSessionStatusSuccess) {
// Display the dictated text
    snprintf(s_last_text, sizeof(s_last_text), "Transcription:\n\n%s", transcription);
    text_layer_set_text(s_output_layer, s_last_text);
  } else {
// Display the reason for any error
    static char s_failed_buff[128];
    snprintf(s_failed_buff, sizeof(s_failed_buff), "Transcription failed.\n\nError ID:\n%d", (int)status);
    text_layer_set_text(s_output_layer, s_failed_buff);
  }
}

static void prv_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  switch (s_mode) {
    case NONE:
      break;
    case SONARR:
      break;
    case RADARR:
      break;      
}

static void prv_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_text_layer, "Show: Press Select to Dictate");
  s_mode = SONARR;
}

static void prv_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_text_layer, "Movie: Press Select to Dictate");
  s_mode = RADARR;
}

static void prv_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, prv_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, prv_down_click_handler);
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_text_layer = text_layer_create(GRect(0, 72, bounds.size.w, 20));
  text_layer_set_overflow_mode(s_text_layer, GTextOverflowModeWordWrap);
  text_layer_set_text(s_text_layer, "Press Up to Add a Show\n\n\n\nPress Up to Add a Movie");
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
  s_dictation_session = dictation_session_create(sizeof(s_last_text), dictation_session_callback, NULL);
}

static void prv_window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
  dictation_session_destroy(s_dictation_session);
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
