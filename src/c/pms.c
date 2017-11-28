#include <pebble.h>
#include <stdio.h> 
#include <string.h>
static Window *s_window;
static TextLayer *s_text_layer;
static DictationSession *s_dictation_session; 
static char s_last_text[512];
static char *s_transcription_header;
static bool s_js_ready;
static char s_pms_sonarr_api_key[PERSIST_DATA_MAX_LENGTH];
static char s_pms_base_url[PERSIST_DATA_MAX_LENGTH]; 
static char s_pms_sonarr_port[PERSIST_DATA_MAX_LENGTH];
static AppTimer *s_timeout_timer;
static int s_pms_response_index;
static char *s_pms_response[8];
static int s_pms_response_items;
static MenuLayer *s_menu_layer;
static GRect *s_bounds;
enum modes {
	NONE,
	SONARR,
	RADARR,
};

enum modes mode;


//*********************************************************************************************
static uint16_t get_num_rows() {
  const uint16_t numrows = s_pms_response_items;
  return numrows;
}

static void draw_row_callback(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *context) {
  static char s_buff[16];
  snprintf(s_buff, sizeof(s_buff), "Row %d", (int)cell_index->row);
  menu_cell_basic_draw(ctx, cell_layer, s_buff, NULL, NULL);
}

static int16_t get_cell_height_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  const int16_t cell_height = 44;
  return cell_height;
}

static void select_callback(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *context) {
  APP_LOG(APP_LOG_LEVEL_DEFAULT, "SELECTED!");
}


static void initialize_menu() {
  layer_remove_from_parent(s_text_layer);
  text_layer_destroy(s_text_layer);
  s_menu_layer = menu_layer_create(s_bounds);
  menu_layer_set_click_config_onto_window(s_menu_layer, s_window);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = get_num_rows_callback,
    .draw_row = draw_row_callback,
    .get_cell_height = get_cell_height_callback,
    .select_click = select_callback,
  });
  
  layer_add_child(s_window, s_menu_layer);

}

static void pms_verify_setup() {
  DictionaryIterator *out_iter;
  AppMessageResult result = app_message_outbox_begin(&out_iter);
  if (result == APP_MSG_OK) {
    dict_write_cstring(out_iter, MESSAGE_KEY_SERVER_URL, s_pms_base_url);
    
    dict_write_cstring(out_iter, MESSAGE_KEY_SONARR_PORT, s_pms_sonarr_port);
    dict_write_cstring(out_iter, MESSAGE_KEY_SONARR_API, s_pms_sonarr_api_key);
    result = app_message_outbox_send();
    if(result != APP_MSG_OK) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending outbox message");
        return;
    }
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "outbox unreachable");
    return;
  }
  persist_write_bool(MESSAGE_KEY_PMS_IS_CONFIGURED, true);

}

static void read_stored_values() {
  
  persist_read_string(MESSAGE_KEY_SERVER_URL, s_pms_base_url, 256);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "stored url: %s", s_pms_base_url);

  persist_read_string(MESSAGE_KEY_SONARR_PORT, s_pms_sonarr_port, 7);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "stored sonarr port: %s", s_pms_sonarr_port);

  persist_read_string(MESSAGE_KEY_SONARR_API, s_pms_sonarr_api_key, 34);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "stored sonarr api: %s", s_pms_sonarr_api_key);
  pms_verify_setup();
}

static void inbox_received_callback(DictionaryIterator *iter, void *context) {  
  Tuple *ready_tuple = dict_find(iter, MESSAGE_KEY_JSReady);
  if(ready_tuple) {
    s_js_ready = true;
    s_pms_response_index = 0;
    read_stored_values();
    return;
  }
  
  
  Tuple *server_response = dict_find(iter, MESSAGE_KEY_PMS_RESPONSE + s_pms_response_index);
  if (server_response) {
    s_pms_response[s_pms_response_index] = server_response->value->cstring;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "received response: %s", s_pms_response[s_pms_response_index]);
    s_pms_response_index ++;
    if(s_pms_response_index > 8) {
    }
    return;
  }
  Tuple *response_sent = dict_find(iter, MESSAGE_KEY_PMS_RESPONSE);
  if (response_sent) {
    s_pms_response_items = s_pms_response_index-1;
    s_pms_response_index = 0;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Received All Items");
    initialize_menu();
    return;
  }

  Tuple *server_url = dict_find(iter, MESSAGE_KEY_SERVER_URL);
  if(server_url) {
    strcpy(s_pms_base_url, server_url->value->cstring);
    persist_write_string(MESSAGE_KEY_SERVER_URL, s_pms_base_url);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "server url is set to %s", s_pms_base_url);
  }
  Tuple *sonarr_port = dict_find(iter, MESSAGE_KEY_SONARR_PORT);
  if(sonarr_port) {
    strcpy(s_pms_sonarr_port, sonarr_port->value->cstring);
    persist_write_string(MESSAGE_KEY_SONARR_PORT, s_pms_sonarr_port);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "sonarr port is set to %s", s_pms_sonarr_port);
  }

  Tuple *sonarr_api = dict_find(iter, MESSAGE_KEY_SONARR_API);
  if (sonarr_api) {
    strcpy(s_pms_sonarr_api_key, sonarr_api->value->cstring);
    persist_write_string(MESSAGE_KEY_SONARR_API, s_pms_sonarr_api_key);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "sonarr api key set to %s", s_pms_sonarr_api_key);

  }

  if(!persist_read_bool(MESSAGE_KEY_PMS_IS_CONFIGURED)) {
    pms_verify_setup();
    APP_LOG(APP_LOG_LEVEL_DEBUG, "attempting to verify setup");
  }  
  
  
}



static void inbox_dropped_callback(AppMessageResult reason, void *context) {
   APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped! "); 
}  

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
   APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!"); 
}  

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
  if (s_timeout_timer) {
    app_timer_cancel(s_timeout_timer); 
  }
}

//static char* transcription_process() {
//  int j = 0;
//  static char output[512];
//  APP_LOG(APP_LOG_LEVEL_DEBUG, "transcription_process(): s_last_text = %s", s_last_text); 
//  for (int i = 0; s_last_text[i] != '\0'; i++) {
//    if (s_last_text[i] != ' ') {output[j] = s_last_text[i]; }
//    if (s_last_text[i] == ' ') {
//    output[j] = '%';
//    j++;
//    output[j] = '2';
//    j++;
//    output[j] = '0';
//    }
//    j++;
//  }
//  APP_LOG(APP_LOG_LEVEL_DEBUG, "output = %s", output);
//  return output;
//}


static void timeout_timer_handler(void *context) {
  if (s_timeout_timer) {
    app_timer_cancel(s_timeout_timer); 
    s_timeout_timer = NULL;
  }
}


static void pms_handle_request() {
  if (s_js_ready == true) {
    DictionaryIterator *out_iter;
    AppMessageResult result = app_message_outbox_begin(&out_iter);
    if (result == APP_MSG_OK) {
      dict_write_cstring(out_iter, MESSAGE_KEY_PMS_REQUEST, s_last_text);
      app_message_outbox_send();
    }
  }
  const int interval = 5000;
  s_timeout_timer = app_timer_register(interval, timeout_timer_handler, NULL); 
  if (s_timeout_timer) {
    s_timeout_timer = NULL;
  }
}

static void pms_initialize_request() {
  if (s_js_ready == true) {
    DictionaryIterator *out_iter;
    AppMessageResult result = app_message_outbox_begin(&out_iter);
    if (result == APP_MSG_OK) {
      int value = 1;
      switch (mode) {
        case NONE:
          break;
        case SONARR:
          dict_write_int(out_iter, MESSAGE_KEY_PMS_SERVICE_SONARR, &value, sizeof(int), true);
          break;
        case RADARR:
          dict_write_int(out_iter, MESSAGE_KEY_PMS_SERVICE_RADARR, &value, sizeof(int), true);
          break;
      
      } 
      app_message_outbox_send();
    }
    const int interval = 1000;

    s_timeout_timer = app_timer_register(interval, timeout_timer_handler, NULL); 
  }
  if (s_timeout_timer) {
    s_timeout_timer = NULL;
  }
}
static void dictation_session_callback(DictationSession *session, DictationSessionStatus status, char *transcription, void *context) {
  if(status == DictationSessionStatusSuccess) {

    snprintf(s_last_text, sizeof(s_last_text), transcription);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "Processed request as: %p", s_last_text);
    pms_handle_request();

  } else {

    static char s_failed_buff[128];
    snprintf(s_failed_buff, sizeof(s_failed_buff), "Transcription failed.\n\nError ID:\n%d", (int)status);
    APP_LOG(APP_LOG_LEVEL_ERROR, "Transcription failed: %s", s_failed_buff);
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
  mode = SONARR;
  pms_initialize_request();
}

static void pms_down_click_handler(ClickRecognizerRef recognizer, void *context) {
  text_layer_set_text(s_text_layer, "\n\n\n\n\nMovie:\nPress Select to Dictate");
  mode = RADARR;
  pms_initialize_request();
}

static void pms_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, pms_select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, pms_up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, pms_down_click_handler);
}

static void pms_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  s_bounds = layer_get_bounds(window);

  s_text_layer = text_layer_create(s_bounds);
  text_layer_set_overflow_mode(s_text_layer, GTextOverflowModeWordWrap);
  text_layer_set_background_color(s_text_layer, GColorBlack);
  text_layer_set_text_color(s_text_layer, GColorGreen);
  text_layer_set_text(s_text_layer, "\n\nPress Up to \nAdd a Show\n\n\n\nPress Down to\nAdd a Movie");
  text_layer_set_text_alignment(s_text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));
  s_dictation_session = dictation_session_create(sizeof(s_last_text), dictation_session_callback, NULL);
  mode = NONE;
}

static void pms_window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
  dictation_session_destroy(s_dictation_session);
  menu_layer_destroy(s_menu_layer);
}
static void pms_init(void) {
  persist_write_bool(MESSAGE_KEY_PMS_IS_CONFIGURED, false);
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
  const int inbox_size = 512;
  const int outbox_size = 512;
  app_message_open(inbox_size, outbox_size);
}

static void pms_deinit(void) {
  window_destroy(s_window);
}

int main(void) {
  pms_init();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", s_window);

  app_event_loop();
  pms_deinit();
}
