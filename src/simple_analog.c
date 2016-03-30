#include <pebble.h>
#include "num2words.h"

#define BUFFER_SIZE 86
#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1
#define KEY_SXX 2
#define KEY_FTSE 3

static Window *s_main_window;
static TextLayer *s_text_layer;
static TextLayer *s_stock_layer;
static TextLayer *s_ftse_layer;
static TextLayer *s_bottom_layer;
static TextLayer *s_temp_layer;
static TextLayer *s_where_is_your_phone_layer;
static Layer *s_battery_layer;
static char s_buffer[BUFFER_SIZE];
static char s_buffer2[BUFFER_SIZE];

static int s_battery_level;

static char sxx_buffer[32];
static char ftse_buffer[32];
static char str1[65];

static GFont s_font;
static GFont s_font_36;
static GFont s_font_40;
static GFont s_font_48;
static GRect bounds;




static void battery_callback(BatteryChargeState state) {
  // Record the new battery level
  s_battery_level = state.charge_percent;

  layer_mark_dirty(s_battery_layer);
}

static void hide_big_screen() {
  printf("Hiding big screen");
  text_layer_set_background_color(s_temp_layer, GColorClear);
  text_layer_set_text(s_temp_layer,"");

  text_layer_set_text(s_where_is_your_phone_layer, "");
  text_layer_set_background_color(s_where_is_your_phone_layer, GColorClear);
}

static void show_where_is_your_phone_screen() {
  text_layer_set_text(s_where_is_your_phone_layer, "Where is your phone?");

  text_layer_set_background_color(s_where_is_your_phone_layer, GColorBlack);
  text_layer_set_text_color(s_where_is_your_phone_layer, GColorWhite);

  app_timer_register(4000, hide_big_screen, NULL );
}

static void bluetooth_callback(bool connected) {
  if(!connected) {
    vibes_double_pulse();
    show_where_is_your_phone_screen();
  }
}

static void accel_tap_handler(AccelAxisType axis, int32_t direction) {

  // Begin dictionary
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  // Add a key-value pair
  dict_write_uint8(iter, 0, 0);

  // Send the message!
  app_message_outbox_send();

}


static void battery_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  // Find the width of the bar
  int width = (int)(float)(((float)s_battery_level / 100.0F) * 143.0F);

  // Draw the background
  graphics_context_set_fill_color(ctx, GColorBlack);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);

  // Draw the bar
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 0, GCornerNone);
}

static void update_time(struct tm *t) {
  fuzzy_time_to_words(t->tm_hour, t->tm_min, s_buffer, BUFFER_SIZE);

  text_layer_set_text(s_text_layer, s_buffer);
  
  text_layer_set_font(s_text_layer, s_font_48);
  
  int a = graphics_text_layout_get_content_size(s_buffer, 
                                                s_font_48, 
                                                GRect(0, 0, text_layer_get_content_size(s_text_layer).w, 500), 
                                        GTextOverflowModeTrailingEllipsis, 
                                        GTextAlignmentLeft).h;                

  printf("The required height is %d", a);
  printf("The available height is %d", bounds.size.h);
  
  if ( a > (bounds.size.h-10) ) {
    printf("too big");
    text_layer_set_font(s_text_layer, s_font_40);

    a = graphics_text_layout_get_content_size(s_buffer, 
                                                s_font_40, 
                                                GRect(0, 0, text_layer_get_content_size(s_text_layer).w, 500), 
                                        GTextOverflowModeTrailingEllipsis, 
                                        GTextAlignmentLeft).h;                

    printf("The required height is %d", a);
    printf("The available height is %d", bounds.size.h);

    if ( a > (bounds.size.h-10) ) {
      printf("still too big");

      text_layer_set_font(s_text_layer, s_font_36);

      a = graphics_text_layout_get_content_size(s_buffer, 
                                                s_font_36, 
                                                GRect(0, 0, text_layer_get_content_size(s_text_layer).w, 500), 
                                        GTextOverflowModeTrailingEllipsis, 
                                        GTextAlignmentLeft).h;                
  
      printf("The required height is %d", a);
      printf("The available height is %d", bounds.size.h);

      if ( a > (bounds.size.h-10) ) {
        printf("still still too big");

        text_layer_set_font(s_text_layer, s_font);
      
      }    
      
    }
      
  }
  
  APP_LOG(APP_LOG_LEVEL_ERROR, text_layer_get_text(s_text_layer));
  //text_layer_set_text(s_stock_layer, "Loading...");

 
  strftime(s_buffer2, 30, "%a %e %b", t);
  text_layer_set_text(s_bottom_layer, s_buffer2);

}

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
  if(tick_time->tm_sec % 15 == 0) {
    update_time(tick_time);
  }
}



static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  bounds = layer_get_frame(window_layer);

  s_text_layer = text_layer_create(GRect(0, 10, bounds.size.w, bounds.size.h - 10));
  text_layer_set_background_color(s_text_layer, GColorBlack);
  text_layer_set_text_color(s_text_layer, GColorWhite);

  s_font = fonts_load_custom_font(
                          resource_get_handle(RESOURCE_ID_FONT_OPENSANS_30));

  s_font_36 = fonts_load_custom_font(
                          resource_get_handle(RESOURCE_ID_FONT_OPENSANS_36));

  s_font_48 = fonts_load_custom_font(
                          resource_get_handle(RESOURCE_ID_FONT_OPENSANS_48));

  s_font_40 = fonts_load_custom_font(
                          resource_get_handle(RESOURCE_ID_FONT_OPENSANS_40));

  text_layer_set_font(s_text_layer, s_font);
  layer_add_child(window_layer, text_layer_get_layer(s_text_layer));

  s_stock_layer = text_layer_create(GRect(0, 0, bounds.size.w, 20));
  text_layer_set_background_color(s_stock_layer, GColorClear);
  text_layer_set_text_color(s_stock_layer, GColorWhite);
  text_layer_set_font(s_stock_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_stock_layer, GTextAlignmentRight);
  text_layer_set_text(s_stock_layer, "...");

  layer_add_child(window_layer, text_layer_get_layer(s_stock_layer));

  s_ftse_layer = text_layer_create(GRect(0, 0, bounds.size.w, 20));
  text_layer_set_background_color(s_ftse_layer, GColorClear);
  text_layer_set_text_color(s_ftse_layer, GColorWhite);
  text_layer_set_font(s_ftse_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_ftse_layer, GTextAlignmentLeft);
  text_layer_set_text(s_ftse_layer, "...");

  layer_add_child(window_layer, text_layer_get_layer(s_ftse_layer));

  s_bottom_layer = text_layer_create(GRect(0, bounds.size.h-18, bounds.size.w, 18));
  text_layer_set_background_color(s_bottom_layer, GColorClear);
  text_layer_set_text_color(s_bottom_layer, GColorWhite);
  text_layer_set_font(s_bottom_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  text_layer_set_text_alignment(s_bottom_layer, GTextAlignmentCenter);
  text_layer_set_text(s_bottom_layer, "...");

  layer_add_child(window_layer, text_layer_get_layer(s_bottom_layer));

  s_temp_layer = text_layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  text_layer_set_background_color(s_temp_layer, GColorClear);
  text_layer_set_text_color(s_temp_layer, GColorWhite);
  text_layer_set_font(s_temp_layer, s_font_40);
  text_layer_set_text_alignment(s_temp_layer, GTextAlignmentLeft);

  layer_add_child(window_layer, text_layer_get_layer(s_temp_layer));
  
  s_where_is_your_phone_layer = text_layer_create(GRect(0, 0, bounds.size.w, bounds.size.h));
  text_layer_set_background_color(s_where_is_your_phone_layer, GColorClear);
  text_layer_set_text_color(s_where_is_your_phone_layer, GColorClear);
  text_layer_set_font(s_where_is_your_phone_layer, s_font_36);
  text_layer_set_text_alignment(s_where_is_your_phone_layer, GTextAlignmentLeft);

  layer_add_child(window_layer, text_layer_get_layer(s_where_is_your_phone_layer));
  
  
  
  time_t now = time(NULL);
  struct tm *t = localtime(&now);
  update_time(t);

  // Create battery meter Layer
  s_battery_layer = layer_create(GRect(0, 0, bounds.size.w, 2));
  layer_set_update_proc(s_battery_layer, battery_update_proc);

  printf("****** The full width %d",  bounds.size.w);
  
  // Add to Window
  layer_add_child(window_get_root_layer(window), s_battery_layer);
  
  tick_timer_service_subscribe(SECOND_UNIT, &handle_second_tick);

  accel_tap_service_subscribe(accel_tap_handler);

  // Register for Bluetooth connection updates
  connection_service_subscribe((ConnectionHandlers) {
    .pebble_app_connection_handler = bluetooth_callback
  });
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_text_layer);
  text_layer_destroy(s_stock_layer);
  text_layer_destroy(s_ftse_layer);
  text_layer_destroy(s_bottom_layer);
  layer_destroy(s_battery_layer);
}


static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  
  // Read tuples for data
Tuple *sxx_tuple = dict_find(iterator, KEY_SXX);
Tuple *ftse_tuple = dict_find(iterator, KEY_FTSE);

// If all data is available, use it
if (sxx_tuple) {
  snprintf(sxx_buffer, sizeof(sxx_buffer), "%s", sxx_tuple->value->cstring);
  
  text_layer_set_text(s_stock_layer, sxx_buffer);

}

  if(ftse_tuple) {
    snprintf(ftse_buffer, sizeof(ftse_buffer), "%s", ftse_tuple->value->cstring);
  
    text_layer_set_text(s_ftse_layer, ftse_buffer);
  }

  printf("****** before SXX %s",  sxx_buffer);
  printf("****** before FTSE %s",  ftse_buffer);
  printf("****** before str1 %s",  str1);
  
  strcpy(str1, sxx_buffer);
  printf("****** after str1 %s",  str1);
  strcat(str1, " ");
  printf("****** after str1 %s",  str1);
  strcat(str1, ftse_buffer);
  printf("****** after str1 %s",  str1);
  
  text_layer_set_background_color(s_temp_layer, GColorBlack);
  text_layer_set_text(s_temp_layer, str1);
  
  app_timer_register(3000, hide_big_screen, NULL );

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
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload,
  });
  window_stack_push(s_main_window, true);
  
  
    // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  // Register for battery level updates
  battery_state_service_subscribe(battery_callback);
  
  // Open AppMessage
  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);

  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  battery_callback(battery_state_service_peek());

  
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}

/*
#include "simple_analog.h"
#include "num2words.h"
#include <math.h>
#include "pebble.h"
#define KEY_TEMPERATURE 0
#define KEY_CONDITIONS 1

static Window *s_window;

static Layer *s_date_layer;
static TextLayer *s_background_layer;
static TextLayer *s_day_label, *s_num_label;

static TextLayer *s_stock_layer;

static TextLayer *s_fuzzy_layer;
static TextLayer *s_minutes_layer;
static TextLayer *s_hour_layer;

// FUZZY
// 12345678901234567890
// It's nearly
// It's about
// It's just gone
// It's just after
// It's just before
// It's coming up to

// MINUTES
// 12345678901234567890
// quarter past
// twenty-five past

// HOUR
// 12345678901234567890
// eleven
static char s_fuzzy_buffer[20], s_minutes_buffer[18], s_hour_buffer[7];

// Store incoming information
static char temperature_buffer[8];
static char conditions_buffer[32];
static char weather_layer_buffer[32];

static char s_day_buffer[12];

static void date_update_proc(Layer *layer, GContext *ctx) {
  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  
//  strftime(s_num_buffer, sizeof(s_num_buffer), "%H:%M", t);
//  text_layer_set_text(s_day_label, s_num_buffer);
  
  strcpy(s_fuzzy_buffer, "Coming up to");
  text_layer_set_text( s_fuzzy_layer, s_fuzzy_buffer);

  strcpy(s_minutes_buffer, "twenty-five");
  text_layer_set_text( s_minutes_layer, s_minutes_buffer);

  strcpy(s_hour_buffer, "past eleven");
  text_layer_set_text(s_hour_layer, s_hour_buffer);

}

static void handle_second_tick(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(window_get_root_layer(s_window));

// Get weather update every 5 minutes
if(tick_time->tm_min % 5 == 0) {
  // Begin dictionary
  DictionaryIterator *iter;
  app_message_outbox_begin(&iter);

  // Add a key-value pair
  dict_write_uint8(iter, 0, 0);

  // Send the message!
//  app_message_outbox_send();
}
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_date_layer = layer_create(bounds);
  s_background_layer = text_layer_create(bounds);

  text_layer_set_background_color(s_background_layer, GColorBlack);

  layer_add_child(window_layer, text_layer_get_layer(s_background_layer));

  layer_set_update_proc(s_date_layer, date_update_proc);
  layer_add_child(window_layer, s_date_layer);
  
  // Create temperature Layer
	s_stock_layer = text_layer_create(GRect(0, 0, bounds.size.w, 30));

  // Style the text  
  text_layer_set_background_color(s_stock_layer, GColorBlack);
  text_layer_set_text_color(s_stock_layer, GColorWhite);
  text_layer_set_font(s_stock_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  text_layer_set_text_alignment(s_stock_layer, GTextAlignmentCenter);
  text_layer_set_text(s_stock_layer, "Loading...");

  layer_add_child(window_layer, text_layer_get_layer(s_stock_layer));
  

  s_fuzzy_layer = text_layer_create( GRect(0,25, bounds.size.w, 35) );
  text_layer_set_background_color(s_fuzzy_layer, GColorBlack);
  text_layer_set_text_color(s_fuzzy_layer, GColorWhite);
  text_layer_set_font(s_fuzzy_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_text_alignment(s_fuzzy_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_fuzzy_layer) );

  
  s_minutes_layer = text_layer_create( GRect(0,75, bounds.size.w, 35) );
  text_layer_set_background_color(s_minutes_layer, GColorBlack);
  text_layer_set_text_color(s_minutes_layer, GColorWhite);
  text_layer_set_font(s_minutes_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_text_alignment(s_minutes_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_minutes_layer) );

  s_hour_layer = text_layer_create( GRect(0,116, bounds.size.w, 35) );
  text_layer_set_background_color(s_hour_layer, GColorBlack);
  text_layer_set_text_color(s_hour_layer, GColorWhite);
  text_layer_set_font(s_hour_layer, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_text_alignment(s_hour_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(s_hour_layer) );

  
 
  //  text_layer_set_text(s_day_label, s_day_buffer);

//   layer_add_child(s_date_layer, text_layer_get_layer(s_day_label));

//   s_num_label = text_layer_create(GRect(0, 125, bounds.size.w, 50));
//   text_layer_set_text(s_num_label, s_num_buffer);
//   text_layer_set_background_color(s_num_label, GColorBlack);
//   text_layer_set_text_color(s_num_label, GColorWhite);
//   text_layer_set_text_alignment(s_day_label, GTextAlignmentCenter);
//   text_layer_set_font(s_num_label, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));

//  layer_add_child(s_date_layer, text_layer_get_layer(s_num_label));
}

static void window_unload(Window *window) {

  text_layer_destroy(s_background_layer);
  layer_destroy(s_date_layer);

  text_layer_destroy(s_stock_layer);

  text_layer_destroy(s_fuzzy_layer);
  text_layer_destroy(s_minutes_layer);
  text_layer_destroy(s_hour_layer);

}

static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  
  // Read tuples for data
Tuple *temp_tuple = dict_find(iterator, KEY_TEMPERATURE);
Tuple *conditions_tuple = dict_find(iterator, KEY_CONDITIONS);

// If all data is available, use it
if(temp_tuple && conditions_tuple) {
  snprintf(temperature_buffer, sizeof(temperature_buffer), "%s", temp_tuple->value->cstring);
  snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", conditions_tuple->value->cstring);
  
  // Assemble full string and display
  snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s, %s", temperature_buffer, conditions_buffer);
  text_layer_set_text(s_stock_layer, weather_layer_buffer);
//   text_layer_set_text(s_stock_layer, conditions_buffer);

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
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);

  s_day_buffer[0] = '\0';

  tick_timer_service_subscribe(SECOND_UNIT, handle_second_tick);

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  
  // Open AppMessage
  const int inbox_size = 128;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);

  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
}


static void deinit() {
  tick_timer_service_unsubscribe();
  window_destroy(s_window);
}

int main() {
  init();
  app_event_loop();
  deinit();
}
*/