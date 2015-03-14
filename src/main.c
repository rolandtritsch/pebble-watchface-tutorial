#include <pebble.h>

static Window* s_main_window;
static TextLayer* s_time_layer;
static TextLayer* s_header_layer;
static TextLayer* s_footer_layer;
static GFont s_time_font;
static BitmapLayer* s_background_layer;
static GBitmap* s_background_bitmap;

static char* s_hellotext_buffer[5+1];

enum {
  KEY_HELLOTEXT = 0
};

static void inbox_received_callback(DictionaryIterator* iterator, void* context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Inbox message reveived!");

  Tuple* t = dict_read_first(iterator);
  assert(t != null);
  assert(t->key == KEY_HELLOTEXT);

  snprintf(s_hellotext_buffer, sizeof(s_hellotext_buffer), "%s", t->value->cstring);
}

static void inbox_dropped_callback(AppMessageResult reason, void* context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Inbox message dropped!");
}

static void outbox_failed_callback(DictionaryIterator* iterator, AppMessageResult reason, void* context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator* iterator, void* context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}

static void update_time() {
  time_t t = time(NULL);
  struct tm* tick_time = localtime(&t);
  static char time_buffer[] = "00:00";

  if(clock_is_24h_style() == true) {
    strftime(time_buffer, sizeof(time_buffer), "%H:%M", tick_time);
  } else {
    strftime(time_buffer, sizeof(time_buffer), "%I:%M", tick_time);
  }

  text_layer_set_text(s_time_layer, time_buffer);
  text_layer_set_text(s_footer_layer, s_hellotext_buffer);
}

static void tick_handler(struct tm* time, TimeUnits units_change) {
  update_time();
}

static void main_window_load(Window* w) {
  s_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_SOURCECODEPRO_REGULAR_42));

  s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_MYBACKGROUND);
  s_background_layer = bitmap_layer_create(GRect(0, 0, 144, 168));
  bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);

  layer_add_child(window_get_root_layer(w), bitmap_layer_get_layer(s_background_layer));

  s_time_layer = text_layer_create(GRect(0, 55, 144, 50));
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_font(s_time_layer, s_time_font);
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  layer_add_child(window_get_root_layer(w), text_layer_get_layer(s_time_layer));

  s_header_layer = text_layer_create(GRect(0, 0, 144, 50));
  text_layer_set_background_color(s_header_layer, GColorClear);
  text_layer_set_text_color(s_header_layer, GColorWhite);
  text_layer_set_font(s_header_layer, s_time_font);
  text_layer_set_text_alignment(s_header_layer, GTextAlignmentCenter);
  text_layer_set_text(s_header_layer, "Hello");

  layer_add_child(window_get_root_layer(w), text_layer_get_layer(s_header_layer));

  s_footer_layer = text_layer_create(GRect(0, 110, 144, 50));
  text_layer_set_background_color(s_footer_layer, GColorClear);
  text_layer_set_text_color(s_footer_layer, GColorWhite);
  text_layer_set_font(s_footer_layer, s_time_font);
  text_layer_set_text_alignment(s_footer_layer, GTextAlignmentCenter);
  text_layer_set_text(s_footer_layer, s_hellotext_buffer);

  layer_add_child(window_get_root_layer(w), text_layer_get_layer(s_footer_layer));
}

static void main_window_unload(Window* w) {
  fonts_unload_custom_font(s_time_font);
  text_layer_destroy(s_time_layer);
  bitmap_layer_destroy(s_background_layer);
  gbitmap_destroy(s_background_bitmap);
}

static void init() {
  snprintf(s_hellotext_buffer, sizeof(s_hellotext_buffer), "%s", "World!");

  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
      .load = main_window_load,
      .unload = main_window_unload
	});
  window_stack_push(s_main_window, true);

  update_time();
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
