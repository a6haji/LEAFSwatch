#include <pebble.h>
  
#define KEY_PERIOD 0
#define KEY_GAME_TIME 1
#define KEY_HOME_TEAM 2
#define KEY_HOME_SCORE 3
#define KEY_AWAY_TEAM 4
#define KEY_AWAY_SCORE 5
  
static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_game_layer;
static TextLayer *s_home_layer;
static TextLayer *s_away_layer;

static GFont s_time_font;
static GFont s_data_font;

static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Create a long-lived buffer
  static char buffer[] = "00:00";

  // Write the current hours and minutes into the buffer
  if(clock_is_24h_style() == true) {
    //Use 2h hour format
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
  } else {
    //Use 12 hour format
    strftime(buffer, sizeof("00:00"), "%I:%M", tick_time);
  }
  
  if (!clock_is_24h_style() && buffer[0] == '0') {
    memmove(buffer, &buffer[1], sizeof(buffer) - 1);
  }

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, buffer);
}

static void main_window_load(Window *window) {
  //Get info about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  //Create GBitmap, then set to created BitmapLayer
  s_background_bitmap = gbitmap_create_with_resource(PBL_IF_ROUND_ELSE(RESOURCE_ID_IMAGE_BACKGROUND_TWO, RESOURCE_ID_IMAGE_BACKGROUND_ONE));
  s_background_layer = bitmap_layer_create(bounds);
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
  layer_add_child(window_get_root_layer(window), bitmap_layer_get_layer(s_background_layer));
  
  // Create time TextLayer
  s_time_layer = text_layer_create(GRect (PBL_IF_ROUND_ELSE(22, 5), PBL_IF_ROUND_ELSE(8, 0), 139, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorWhite);
  text_layer_set_text(s_time_layer, "00:00");
  
  //Create GFont
  s_time_font = fonts_load_custom_font(resource_get_handle(PBL_IF_ROUND_ELSE(RESOURCE_ID_FONT_SCRATCH_30, RESOURCE_ID_FONT_SCRATCH_40)));

  //Apply to TextLayer
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_time_layer));
  
  // Create Game Layer
  s_game_layer = text_layer_create(GRect(PBL_IF_ROUND_ELSE(15, 0), PBL_IF_ROUND_ELSE(127, 115), 144, 25));
  text_layer_set_background_color(s_game_layer, GColorClear);
  text_layer_set_text_color(s_game_layer, GColorWhite);
  text_layer_set_text_alignment(s_game_layer, GTextAlignmentCenter);
  text_layer_set_text(s_game_layer, "Loading...");
  
  // Create second custom font, apply it and add to Window
  s_data_font = fonts_load_custom_font(resource_get_handle(PBL_IF_ROUND_ELSE(RESOURCE_ID_FONT_SCRATCH_12, RESOURCE_ID_FONT_SCRATCH_14)));
  text_layer_set_font(s_game_layer, s_data_font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_game_layer));
  
  // Create Home Team Layer
  s_home_layer = text_layer_create(GRect(PBL_IF_ROUND_ELSE(15, 5), PBL_IF_ROUND_ELSE(142, 130), 144, 25));
  text_layer_set_background_color(s_home_layer, GColorClear);
  text_layer_set_text_color(s_home_layer, GColorWhite);
  text_layer_set_text_alignment(s_home_layer, GTextAlignmentCenter);
  text_layer_set_text(s_home_layer, "");
  text_layer_set_font(s_home_layer, s_data_font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_home_layer));
  
  // Create Away Team Layer
  s_away_layer = text_layer_create(GRect(PBL_IF_ROUND_ELSE(15, 5), PBL_IF_ROUND_ELSE(157, 145), 144, 25));
  text_layer_set_background_color(s_away_layer, GColorClear);
  text_layer_set_text_color(s_away_layer, GColorWhite);
  text_layer_set_text_alignment(s_away_layer, GTextAlignmentCenter);
  text_layer_set_text(s_away_layer, "");
  text_layer_set_font(s_away_layer, s_data_font);
  layer_add_child(window_get_root_layer(window), text_layer_get_layer(s_away_layer));
  
  
  // Make sure the time is displayed from the start
  update_time();
}

static void main_window_unload(Window *window) {
  //Unload GFont
  fonts_unload_custom_font(s_time_font);
  fonts_unload_custom_font(s_data_font);
  
  //Destroy GBitmap
  gbitmap_destroy(s_background_bitmap);

  //Destroy BitmapLayer
  bitmap_layer_destroy(s_background_layer);
  
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
  text_layer_destroy(s_game_layer);
  text_layer_destroy(s_home_layer);
  
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  
  // Get score update every 5 minutes
  if(tick_time->tm_min % 5 == 0) {
    // Begin dictionary
    DictionaryIterator *iter;
    app_message_outbox_begin(&iter);

    // Add a key-value pair
    dict_write_uint8(iter, 0, 0);

    // Send the message!
    app_message_outbox_send();
  }
}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  // Store incoming information
  static char period_buffer[32];
  static char game_time_buffer[32];
  static char game_layer_buffer[32];
  
  static char home_team_buffer[32];
  static char home_score_buffer[32];
  static char home_layer_buffer[32];
  
  static char away_team_buffer[32];
  static char away_score_buffer[32];
  static char away_layer_buffer[32];
  
  // Read first item
  Tuple *t = dict_read_first(iterator);

  // For all items
  while(t != NULL) {
    // Which key was received?
    switch(t->key) {
    case KEY_PERIOD:
      snprintf(period_buffer, sizeof(period_buffer), "%s", t->value->cstring);
      break;
    case KEY_GAME_TIME:
      snprintf(game_time_buffer, sizeof(game_time_buffer), "%s", t->value->cstring);
      break;
    case KEY_HOME_TEAM:
      snprintf(home_team_buffer, sizeof(home_team_buffer), "%s", t->value->cstring);
      break;
    case KEY_HOME_SCORE:
      snprintf(home_score_buffer, sizeof(home_score_buffer), "%s", t->value->cstring);
      break;
    case KEY_AWAY_TEAM:
      snprintf(away_team_buffer, sizeof(away_team_buffer), "%s", t->value->cstring);
      break;
    case KEY_AWAY_SCORE:
      snprintf(away_score_buffer, sizeof(away_score_buffer), "%s", t->value->cstring);
      break;
    default:
      APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
      break;
    }

    // Look for next item
    t = dict_read_next(iterator);
  }
  
  // Assemble full string and display
  snprintf(game_layer_buffer, sizeof(game_layer_buffer), "%s %s", period_buffer, game_time_buffer);
  text_layer_set_text(s_game_layer, game_layer_buffer);
  
  snprintf(home_layer_buffer, sizeof(home_layer_buffer), "%s %s", home_team_buffer, home_score_buffer);
  text_layer_set_text(s_home_layer, home_layer_buffer);
  
  snprintf(away_layer_buffer, sizeof(away_layer_buffer), "%s %s", away_team_buffer, away_score_buffer);
  text_layer_set_text(s_away_layer, away_layer_buffer);
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
  
static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  
  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  // Open AppMessage
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
