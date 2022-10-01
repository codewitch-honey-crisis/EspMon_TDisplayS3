#include "Arduino.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "lvgl.h"
#include "pin_config.h"
#include "ui.h"

esp_lcd_panel_io_handle_t io_handle = NULL;
static lv_disp_draw_buf_t disp_buf;  // contains internal graphic buffer(s) called draw buffer(s)
static lv_disp_drv_t disp_drv;       // contains callback functions
static lv_color_t *lv_disp_buf;
static bool is_initialized_lvgl = false;

static bool notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx) {
    if (is_initialized_lvgl) {
        lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
        lv_disp_flush_ready(disp_driver);
    }
    return false;
}

static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map) {
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)drv->user_data;
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    // copy a buffer's content to a specific area of the display
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
}

void setup() {
    pinMode(PIN_POWER_ON, OUTPUT);
    digitalWrite(PIN_POWER_ON, HIGH);
    Serial.begin(115200);

    pinMode(PIN_LCD_RD, OUTPUT);
    digitalWrite(PIN_LCD_RD, HIGH);
    esp_lcd_i80_bus_handle_t i80_bus = NULL;
    esp_lcd_i80_bus_config_t bus_config = {
        .dc_gpio_num = PIN_LCD_DC,
        .wr_gpio_num = PIN_LCD_WR,
        .clk_src = LCD_CLK_SRC_PLL160M,
        .data_gpio_nums =
            {
                PIN_LCD_D0,
                PIN_LCD_D1,
                PIN_LCD_D2,
                PIN_LCD_D3,
                PIN_LCD_D4,
                PIN_LCD_D5,
                PIN_LCD_D6,
                PIN_LCD_D7,
            },
        .bus_width = 8,
        .max_transfer_bytes = LVGL_LCD_BUF_SIZE * sizeof(uint16_t),
    };
    esp_lcd_new_i80_bus(&bus_config, &i80_bus);

    esp_lcd_panel_io_i80_config_t io_config = {
        .cs_gpio_num = PIN_LCD_CS,
        .pclk_hz = EXAMPLE_LCD_PIXEL_CLOCK_HZ,
        .trans_queue_depth = 20,
        .on_color_trans_done = notify_lvgl_flush_ready,
        .user_ctx = &disp_drv,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .dc_levels =
            {
                .dc_idle_level = 0,
                .dc_cmd_level = 0,
                .dc_dummy_level = 0,
                .dc_data_level = 1,
            },
    };
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_i80(i80_bus, &io_config, &io_handle));
    esp_lcd_panel_handle_t panel_handle = NULL;
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_LCD_RES,
        .color_space = ESP_LCD_COLOR_SPACE_RGB,
        .bits_per_pixel = 16,
    };
    esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle);
    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    esp_lcd_panel_invert_color(panel_handle, true);

    esp_lcd_panel_swap_xy(panel_handle, true);
    esp_lcd_panel_mirror(panel_handle, false, true);
    // the gap is LCD panel specific, even panels with the same driver IC, can
    // have different gap value
    esp_lcd_panel_set_gap(panel_handle, 0, 35);

    /* Lighten the screen with gradient */
    ledcSetup(0, 10000, 8);
    ledcAttachPin(PIN_LCD_BL, 0);
    for (uint8_t i = 0; i < 0xFF; i++) {
        ledcWrite(0, i);
        delay(2);
    }

    lv_init();
    lv_disp_buf = (lv_color_t *)heap_caps_malloc(LVGL_LCD_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL);

    lv_disp_draw_buf_init(&disp_buf, lv_disp_buf, NULL, LVGL_LCD_BUF_SIZE);
    /*Initialize the display*/
    lv_disp_drv_init(&disp_drv);
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = EXAMPLE_LCD_H_RES;
    disp_drv.ver_res = EXAMPLE_LCD_V_RES;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = panel_handle;
    lv_disp_drv_register(&disp_drv);

    is_initialized_lvgl = true;

    ui_init();

    USBSerial.begin(115200);
}
static int ticker = 0;
void loop() {
    char sz[64];
    union {
        float f;
        uint8_t b[4];
    } fbu;
    if(ticker++>=33) {
      ticker = 0;
      
      if (USBSerial.available()) {
          int i = USBSerial.readBytes(fbu.b, sizeof(fbu.b));
          if (i == 0) {
              USBSerial.write('#');
          } else {
              lv_bar_set_value(ui_CpuBar, (int)(fbu.f + .5), LV_ANIM_ON);
              if (USBSerial.available()) {
                  i = USBSerial.readBytes(fbu.b, sizeof(fbu.b));
                  if (i != 0) {
                      snprintf(sz, sizeof(sz), "%0.1fC/%0.1fF", fbu.f, fbu.f * (9.0f / 5.0f) + 32);
                      lv_label_set_text(ui_CpuTempLabel, sz);
                      if (USBSerial.available()) {
                          i = USBSerial.readBytes(fbu.b, sizeof(fbu.b));
                          if (i != 0) {
                              lv_bar_set_value(ui_GpuBar, (int)(fbu.f + .5), LV_ANIM_ON);
                              if (USBSerial.available()) {
                                  i = USBSerial.readBytes(fbu.b, sizeof(fbu.b));
                                  if (i != 0) {
                                      snprintf(sz, sizeof(sz), "%0.1fC/%0.1fF", fbu.f, fbu.f * (9.0f / 5.0f) + 32);
                                      lv_label_set_text(ui_GpuTempLabel, sz);
                                  } else {
                                      USBSerial.write('#');
                                  }
                              } else {
                                  USBSerial.write('#');
                              }
                          } else {
                              USBSerial.write('#');
                          }
                      } else {
                          USBSerial.write('#');
                      }
                  } else {
                      USBSerial.write('#');
                  }
              } else {
                  USBSerial.write('#');
              }
          }
      } else {
        USBSerial.write('#');
      }
    }
    lv_timer_handler();
    delay(3);
}