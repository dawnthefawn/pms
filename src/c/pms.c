#include <pebble.h>
#include <stdio.h> 
#include <string.h>
static Window *s_window;
static TextLayer *s_text_layer;
static DictationSession *s_dictation_session; 
static char s_last_text[512];
static char *s_transcription_header;
static bool s_js_ready;
//static char *s_request_url;
//static char *s_sonarr_api_key;
//static const char *s_pms_base_url =  "http://192.168.1.100:";
//static const char *s_pms_sonarr_root = "8989/api/";
//static const char *s_pms_sonarr_request = "series/lookup?term=";
static char *s_request;
enum modes {
	NONE,
	SONARR,
	RADARR,
};

enum modes mode;


//*********************************************************************************************
bool comm_is_js_ready() {
  return s_js_ready;
}

static void inbox_received_callback(DictionaryIterator *iter, void *context) {  
  Tuple *ready_tuple = dict_find(iter, MESSAGE_KEY_JSReady);
  if(ready_tuple) {
    s_js_ready = true;
  }
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
   APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!"); 
}  

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
   APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!"); 
}  

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
   APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static char* transcription_process() {
  int j = 0;
  static char output[512];
 
  for (int i = 0; s_last_text[i] != '\0'; i++) {
    if (s_last_text[i] != ' ') {output[j] = s_last_text[i]; }
    if (s_last_text[i] == ' ') {
    output[j] = '%';
    j++;
    output[j] = '2';
    j++;
    output[j] = '0';
    }
    j++;
  }
  return &output[0];
}

//static void url_builder(const char *request) {
//  strcpy(s_request_url, s_pms_base_url);
//  switch (mode) {
//    case NONE:
//      break;
//    case SONARR:
//      strcat(s_request_url, s_pms_sonarr_root);
//      strcat(s_request_url, s_pms_sonarr_request); 
//      strcat(s_request_url, request);
//      strcat(s_request_url, s_sonarr_api_key);
//      break;
//    case RADARR:
//      break;
//  }
//}

static void dictation_session_callback(DictationSession *session, DictationSessionStatus status, char *transcription, void *context) {
  if(status == DictationSessionStatusSuccess) {
// Display the dictated text
    snprintf(s_last_text, sizeof(s_last_text),s_transcription_header, transcription);
//    char *temp = transcription_process(s_last_text); 
    s_request = transcription_process(s_last_text);
//    url_builder(s_request);
    text_layer_set_text(s_text_layer, s_request);
    
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Processed request as: %p", s_request);
//    layer_set_hidden(s_text_layer, true);
  } else {
// Display the reason for any error
    static char s_failed_buff[128];
    snprintf(s_failed_buff, sizeof(s_failed_buff), "Transcription failed.\n\nError ID:\n%d", (int)status);
//    text_layer_set_text(s_text_layer, s_failed_buff);

    text_layer_set_text(s_text_layer, "\n\nPress Up to \nAdd a Show\n\n\n\nPress Down to\nAdd a Movie");
  }
}



static void pms_select_click_handler(ClickRecognizerRef recognizer, void *context) {
  switch (mode) {
    case NONE:
      break;
    case SONARR:
      s_transcription_header = "\n\nSearching Show:\n\n%s";
      dictation_session_start(s_dictation_session);
      break;
    case RADARR:
      s_transcription_header = "\n\nSearching Movie:\n\n%s";
      break;      
  }
}

static void pms_up_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_text_layer, "\n\n\n\n\nShow:\nPress Select to Dictate");
//  s_request_url = PMS_BASE_URL + PMS_SONARR_ROOT + PMS_SONARR_REQUEST;
  mode = SONARR;
}

static void pms_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_text_layer, "\n\n\n\n\nMovie:\nPress Select to Dictate");
  mode = RADARR;
}

static void pms_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, pms_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, pms_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, pms_down_click_handler);
}

static void pms_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_text_layer = text_layer_create(bounds);
  text_layer_set_overflow_mode(s_text_layer, GTextOverflowModeWordWrap);
  text_layer_set_background_color(s_text_layer, GColorBlack);
  text_layer_set_text_color(s_text_layer, GColorGreen);
//  font_get_system_font(FONT_KEY_GOTHIC_24_BOLD);
  text_layer_set_text(s_text_layer, "\n\nPress Up to \nAdd a Show\n\n\n\nPress Down to\nAdd a Movie");
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
  s_dictation_session = dictation_session_create(sizeof(s_last_text), dictation_session_callback, NULL);
  mode = NONE;
}

static void pms_window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
  dictation_session_destroy(s_dictation_session);
}

static void pms_init(void) {
  s_window = window_create();
  window_set_click_config_provider(s_window, pms_click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = pms_window_load,
    .unload = pms_window_unload,
  });
  const bool animated = true;
  window_stack_push(s_window, animated);
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);  
  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);
}

static void pms_deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  pms_init();
//  s_sonarr_api_key = "&apikey=d8b01e33441bb82e0d9b0083b453";
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  pms_deinit();
}
