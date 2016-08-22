#include <pebble.h>

// TODO Cleanup naming conventions

// Storage Keys
enum storage {
  S_BACKGROUND_COLOR,
  S_MINUTE_BACKGROUND_COLOR,
  S_MINUTE_COLOR,
  S_HOUR_COLOR,
  S_DATE_COLOR,
  
  S_SHOW_DATE
};

// Defined at init
static Window* window;
static Layer* minute_tail_background_layer;
static Layer* minute_tail_layer;
static TextLayer* date_layer;
static Layer* hour_layer;
static Layer* hour_dot_layer;
static Layer* minute_layer;
static Layer* minute_dot_layer;
static GRect bounds;
static GPoint center;
static uint16_t minute_back_length;
static uint16_t hour_back_length;
static uint16_t minute_width;
static uint16_t hour_width;
static uint16_t minute_dot_radius;
static uint16_t hour_dot_radius;
static uint16_t minute_tail_gap;
static uint32_t minute_tail_thickness;
static uint16_t hour_gap;
static uint16_t minute_tail_radius;
static uint16_t hour_radius;
#ifdef PBL_RECT
static Layer* tail_mask_layer;
static GRect tail_fill_bounds;
#endif

// Set by config
static GColor8 minute_color;
static GColor8 hour_color;
static GColor8 background_color;
static GColor8 tail_background_color;

// Set at each update
static int32_t hour_angle;
static int32_t min_angle;
static char day[3];


static GRect center_square(uint16_t radius) {
  return (GRect){{center.x - radius, center.y - radius},
                 {radius * 2, radius * 2}};
}
  
static void tick_update(struct tm* tick_time, TimeUnits units_changed) {
  if (units_changed & (MINUTE_UNIT | HOUR_UNIT)) {
    min_angle = TRIG_MAX_ANGLE * tick_time->tm_min / 60;
    hour_angle = TRIG_MAX_ANGLE * (tick_time->tm_hour * 60 + tick_time->tm_min) / (12 * 60);
    layer_mark_dirty(minute_tail_layer);
    layer_mark_dirty(minute_layer);
    layer_mark_dirty(hour_layer);
  }
  if (units_changed & DAY_UNIT) {
    snprintf(day, 3, "%02d", tick_time->tm_mday);
    text_layer_set_text(date_layer, day);
  }
}

static void minute_tail_background_layer_update(Layer* layer, GContext* ctx) {
  graphics_context_set_fill_color(ctx, tail_background_color);
  #ifdef PBL_RECT
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
  #else
  graphics_fill_radial(ctx, layer_get_bounds(layer), GOvalScaleModeFillCircle, minute_tail_thickness, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(360));
  #endif
}

static void minute_tail_layer_update(Layer* layer, GContext* ctx) {
  graphics_context_set_fill_color(ctx, minute_color);
  #ifdef PBL_RECT
  graphics_fill_radial(ctx, tail_fill_bounds, GOvalScaleModeFillCircle, tail_fill_bounds.size.h / 2, DEG_TO_TRIGANGLE(0), min_angle);
  #else
  graphics_fill_radial(ctx, layer_get_bounds(layer), GOvalScaleModeFillCircle, minute_tail_thickness, DEG_TO_TRIGANGLE(0), min_angle);
  #endif
}

#ifdef PBL_RECT
static void minute_tail_mask_layer_update(Layer* layer, GContext* ctx) {
  graphics_context_set_fill_color(ctx, background_color);
  graphics_fill_rect(ctx, layer_get_bounds(layer), 0, GCornerNone);
}
#endif

static void hour_layer_update(Layer* layer, GContext* ctx) {
  graphics_context_set_stroke_color(ctx, hour_color);
  graphics_context_set_stroke_width(ctx, hour_width);
  GPoint tip = {(sin_lookup(hour_angle) * hour_radius / TRIG_MAX_RATIO) + hour_radius,
                (-cos_lookup(hour_angle) * hour_radius / TRIG_MAX_RATIO) + hour_radius};
  GPoint back = {(-sin_lookup(hour_angle) * hour_back_length / TRIG_MAX_RATIO) + hour_radius,
                 (cos_lookup(hour_angle) * hour_back_length / TRIG_MAX_RATIO) + hour_radius};
  graphics_draw_line(ctx, back, tip);
}

static void hour_dot_layer_update(Layer* layer, GContext* ctx) {
  graphics_context_set_fill_color(ctx, hour_color);
  graphics_fill_radial(ctx, layer_get_bounds(layer), GOvalScaleModeFillCircle, hour_dot_radius, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(360));
}

static void minute_layer_update(Layer* layer, GContext* ctx) {
  graphics_context_set_stroke_color(ctx, minute_color);
  graphics_context_set_stroke_width(ctx, minute_width);
  int32_t tip_length = (bounds.size.w + bounds.size.h) / 2;
  GPoint tip = {(sin_lookup(min_angle) * tip_length / TRIG_MAX_RATIO) + center.x,
                (-cos_lookup(min_angle) * tip_length / TRIG_MAX_RATIO) + center.y};
  GPoint back = {(-sin_lookup(min_angle) * minute_back_length / TRIG_MAX_RATIO) + center.x,
                 (cos_lookup(min_angle) * minute_back_length / TRIG_MAX_RATIO) + center.y};
  graphics_draw_line(ctx, back, tip);
}

static void minute_dot_layer_update(Layer* layer, GContext* ctx) {
  graphics_context_set_fill_color(ctx, minute_color);
  graphics_fill_radial(ctx, layer_get_bounds(layer), GOvalScaleModeFillCircle, minute_dot_radius, DEG_TO_TRIGANGLE(0), DEG_TO_TRIGANGLE(360));
}

void repaint(void) {
  background_color = GColorFromHEX(persist_read_int(S_BACKGROUND_COLOR));
  minute_color = GColorFromHEX(persist_read_int(S_MINUTE_COLOR));
  tail_background_color = GColorFromHEX(persist_read_int(S_MINUTE_BACKGROUND_COLOR));
  hour_color = GColorFromHEX(persist_read_int(S_HOUR_COLOR));
  text_layer_set_text_color(date_layer, GColorFromHEX(persist_read_int(S_DATE_COLOR)));
  layer_set_hidden((Layer*) date_layer, !persist_read_bool(S_SHOW_DATE));
  
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  window_set_background_color(window, background_color);
  tick_update(tick_time, HOUR_UNIT | MINUTE_UNIT | DAY_UNIT);
}

static void config_received(DictionaryIterator *iter, void *context) {
  Tuple* tup;
  
  // Colors
  tup = dict_find(iter, MESSAGE_KEY_BACKGROUND_COLOR);
  if (tup) {
    persist_write_int(S_BACKGROUND_COLOR, tup->value->int32);
  }
  
  tup = dict_find(iter, MESSAGE_KEY_MINUTE_COLOR);
  if (tup) {
    persist_write_int(S_MINUTE_COLOR, tup->value->int32);
  }
  
  tup = dict_find(iter, MESSAGE_KEY_MINUTE_BACKGROUND_COLOR);
  if (tup) {
    persist_write_int(S_MINUTE_BACKGROUND_COLOR, tup->value->int32);
  }
  
  tup = dict_find(iter, MESSAGE_KEY_HOUR_COLOR);
  if (tup) {
    persist_write_int(S_HOUR_COLOR, tup->value->int32);
  }
  
  tup = dict_find(iter, MESSAGE_KEY_DATE_COLOR);
  if (tup) {
    persist_write_int(S_DATE_COLOR, tup->value->int32);
  }
  
  // Misc things
  tup = dict_find(iter, MESSAGE_KEY_SHOW_DATE);
  if (tup) {
    persist_write_bool(S_SHOW_DATE, tup->value->uint8);
  }
  
  repaint();
}

void init(void) {
  // Create window and define sizes accordingly
  // This is an attempt to make measurements roughly indepedent of screen size
  window = window_create();
  Layer *window_layer = window_get_root_layer(window);
  bounds = layer_get_bounds(window_layer);
  center = grect_center_point(&bounds);
  minute_tail_gap = bounds.size.w / 9;
  minute_tail_thickness = minute_tail_gap / 2;
  hour_gap = minute_tail_thickness;
  hour_back_length = minute_tail_gap * 3 / 4;
  minute_back_length = hour_back_length * 3 / 2;
  minute_width = 2;
  hour_width = 4;
  minute_dot_radius = hour_width;
  hour_dot_radius = minute_dot_radius + 1;
  minute_tail_radius = bounds.size.w / 2 - minute_tail_gap;
  hour_radius = minute_tail_radius - minute_tail_thickness - hour_gap;
  GRect tail_bounds = {{minute_tail_gap, minute_tail_gap}, {bounds.size.w - 2 * minute_tail_gap, bounds.size.h - 2 * minute_tail_gap}};
  GRect date_bounds = {{center.x - 15, center.y + (bounds.size.h / 2 - minute_tail_gap - minute_tail_thickness) / 2 - 10}, {30, 20}};
  #ifdef PBL_RECT
  int16_t fill_size = tail_bounds.size.w + tail_bounds.size.h;
  tail_fill_bounds = (GRect){{(tail_bounds.size.w - fill_size) / 2, (tail_bounds.size.h - fill_size) / 2},
                             {fill_size, fill_size}};
  #endif
  
  // Set default storage values
  if (!persist_exists(S_BACKGROUND_COLOR)) {
    persist_write_int(S_BACKGROUND_COLOR, 0x00ffffff);
  }
  if (!persist_exists(S_MINUTE_BACKGROUND_COLOR)) {
    persist_write_int(S_MINUTE_BACKGROUND_COLOR, PBL_IF_COLOR_ELSE(0x00aaaaaa, 0x00ffffff));
  }
  if (!persist_exists(S_MINUTE_COLOR)) {
    persist_write_int(S_MINUTE_COLOR, 0x00000000);
  }
  if (!persist_exists(S_HOUR_COLOR)) {
    persist_write_int(S_HOUR_COLOR, 0x00000000);
  }
  if (!persist_exists(S_DATE_COLOR)) {
    persist_write_int(S_DATE_COLOR, 0x00000000);
  }
  if (!persist_exists(S_SHOW_DATE)) {
    persist_write_bool(S_SHOW_DATE, 1);
  }
  
  // Listen for config changes
  app_message_register_inbox_received(config_received);
  uint32_t dict_size = dict_calc_buffer_size(6, 4, 4, 4, 4, 4, 1);
  app_message_open(dict_size, dict_size);
  
  // Define all layers
  minute_tail_background_layer = layer_create(tail_bounds);
  layer_set_update_proc(minute_tail_background_layer, minute_tail_background_layer_update);
  layer_add_child(window_layer, minute_tail_background_layer);
  layer_mark_dirty(minute_tail_background_layer);
  
  minute_tail_layer = layer_create(tail_bounds);
  layer_set_update_proc(minute_tail_layer, minute_tail_layer_update);
  layer_add_child(window_layer, minute_tail_layer);
  
  #ifdef PBL_RECT
  tail_mask_layer = layer_create((GRect){{tail_bounds.origin.x + minute_tail_thickness, tail_bounds.origin.y + minute_tail_thickness},
                                                      {tail_bounds.size.w - 2 * minute_tail_thickness, tail_bounds.size.h - 2 * minute_tail_thickness}});
  layer_set_update_proc(tail_mask_layer, minute_tail_mask_layer_update);
  layer_add_child(window_layer, tail_mask_layer);
  layer_mark_dirty(tail_mask_layer);
  #endif
  
  date_layer = text_layer_create(date_bounds);
  text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_LECO_20_BOLD_NUMBERS));
  text_layer_set_text_alignment(date_layer, GTextAlignmentCenter);
  text_layer_set_background_color(date_layer, GColorClear);
  layer_add_child(window_layer, (Layer*) date_layer);
  
  hour_layer = layer_create(center_square(hour_radius));
  layer_set_update_proc(hour_layer, hour_layer_update);
  layer_add_child(window_layer, hour_layer);
  
  hour_dot_layer = layer_create(center_square(hour_dot_radius));
  layer_set_update_proc(hour_dot_layer, hour_dot_layer_update);
  layer_add_child(window_layer, hour_dot_layer);
  layer_mark_dirty(hour_dot_layer);
  
  minute_layer = layer_create(bounds);
  layer_set_update_proc(minute_layer, minute_layer_update);
  layer_add_child(window_layer, minute_layer);
  
  minute_dot_layer = layer_create(center_square(minute_dot_radius));
  layer_set_update_proc(minute_dot_layer, minute_dot_layer_update);
  layer_add_child(window_layer, minute_dot_layer);
  layer_mark_dirty(minute_dot_layer);
  
  window_stack_push(window, true);
  
  // Set up tick callback and initialize first update
  tick_timer_service_subscribe(MINUTE_UNIT, tick_update);
  repaint();
}

void deinit(void) {
  layer_destroy(minute_dot_layer);
  layer_destroy(hour_dot_layer);
  layer_destroy(hour_layer);
  layer_destroy(minute_tail_layer);
  text_layer_destroy(date_layer);
  #ifdef PBL_RECT
  layer_destroy(tail_mask_layer);
  #endif
  layer_destroy(minute_tail_background_layer);
  layer_destroy(minute_layer);
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}