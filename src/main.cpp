/*
 *
 *  BEFORE BUILDING
 *
 * Copy include/lv_conf.h to your .pio.../libdeps folder next to the lvgl folder, but outside of it.
 */
#include "Arduino.h"
#include "circular_buffer.hpp"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "htcw_button.hpp"
#include "lvgl.h"
#include "pin_config.h"
#include "ui.h"
#include "ui_patch.h"
using namespace arduino;
esp_lcd_panel_io_handle_t io_handle = NULL;
static lv_disp_draw_buf_t disp_buf;  // contains internal graphic buffer(s) called draw buffer(s)
static lv_disp_drv_t disp_drv;       // contains callback functions
static lv_color_t *lv_disp_buf;
static bool is_initialized_lvgl = false;
circular_buffer<uint8_t, 135> cpu_graph;
circular_buffer<uint8_t, 135> gpu_graph;
static lv_color_t cpu_graph_buf[LV_IMG_BUF_SIZE_TRUE_COLOR(135, 37)];
static lv_color_t gpu_graph_buf[LV_IMG_BUF_SIZE_TRUE_COLOR(135, 37)];
static size_t screen = 0;
button<PIN_BUTTON_1, 10, true> button_prev;
button<PIN_BUTTON_2, 10, true> button_next;
static void button_prev_cb(bool pressed, void *state) {
    if (!pressed) {
        if (screen == 0) {
            screen = 1;
        } else {
            --screen;
        }
        cpu_graph.clear();
        gpu_graph.clear();
        switch(screen) {
            case 0:
            lv_scr_load(ui_Screen1);
            break;
            case 1:
            lv_scr_load(ui_Screen2);
            break;
        }
    }
}
static void button_next_cb(bool pressed, void *state) {
    if (!pressed) {
        if (screen == 1) {
            screen = 0;
        } else {
            ++screen;
        }
        cpu_graph.clear();
        gpu_graph.clear();
        switch(screen) {
            case 0:
            lv_scr_load(ui_Screen1);
            break;
            case 1:
            lv_scr_load(ui_Screen2);
            break;
        }
    }
}
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
    disp_drv.hor_res = LCD_H_RES;
    disp_drv.ver_res = LCD_V_RES;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = panel_handle;
    lv_disp_drv_register(&disp_drv);

    is_initialized_lvgl = true;

    ui_init();
    ui_patch();
    lv_canvas_set_buffer(ui_CpuGraph, cpu_graph_buf, 135, 37, LV_IMG_CF_TRUE_COLOR);
    lv_canvas_set_buffer(ui_GpuGraph, gpu_graph_buf, 135, 37, LV_IMG_CF_TRUE_COLOR);
    lv_canvas_set_buffer(ui_CpuGhzGraph, cpu_graph_buf, 135, 37, LV_IMG_CF_TRUE_COLOR);
    lv_canvas_set_buffer(ui_GpuGhzGraph, gpu_graph_buf, 135, 37, LV_IMG_CF_TRUE_COLOR);
    button_prev.callback(button_prev_cb);
    button_next.callback(button_next_cb);
    USBSerial.begin(115200);
}
static void update_screen_0() {
    uint8_t tmp;
    uint8_t v;
    bool redraw_cpu, redraw_gpu;
    char sz[64];
    union {
        float f;
        uint8_t b[4];
    } fbu;
    redraw_cpu = false;
    redraw_gpu = false;
    if (USBSerial.available()) {
        int i = USBSerial.readBytes(fbu.b, sizeof(fbu.b));
        if (i == 0) {
            USBSerial.write('#');
        } else {
            if (cpu_graph.full()) {
                cpu_graph.get(&tmp);
            }
            v = (fbu.f + .5);
            cpu_graph.put(v);
            redraw_cpu = true;
            lv_bar_set_value(ui_CpuBar, v, LV_ANIM_ON);
            if (USBSerial.available()) {
                i = USBSerial.readBytes(fbu.b, sizeof(fbu.b));
                if (i != 0) {
                    snprintf(sz, sizeof(sz), "%0.1fC/%0.1fF", fbu.f, fbu.f * (9.0f / 5.0f) + 32);
                    lv_label_set_text(ui_CpuTempLabel, sz);
                    if (USBSerial.available()) {
                        i = USBSerial.readBytes(fbu.b, sizeof(fbu.b));
                        if (i != 0) {
                            if (gpu_graph.full()) {
                                gpu_graph.get(&tmp);
                            }
                            v = (fbu.f + .5);
                            gpu_graph.put(v);
                            redraw_gpu = true;
                            lv_bar_set_value(ui_GpuBar, v, LV_ANIM_ON);
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
    if (redraw_cpu) {
        lv_point_t pts[sizeof(cpu_graph)];
        lv_draw_line_dsc_t dsc;
        lv_draw_line_dsc_init(&dsc);
        dsc.width = 1;
        dsc.color = lv_color_hex(0x0000FF);
        dsc.opa = LV_OPA_100;
        lv_canvas_fill_bg(ui_CpuGraph, lv_color_white(), LV_OPA_100);
        v = *cpu_graph.peek(0);
        pts[0].x = 0;
        pts[0].y = 36 - (v / 100.0f) * 36;
        for (size_t i = 1; i < cpu_graph.size(); ++i) {
            v = *cpu_graph.peek(i);
            pts[i].x = i;
            pts[i].y = 36 - (v / 100.0f) * 36;
        }
        lv_canvas_draw_line(ui_CpuGraph, pts, cpu_graph.size(), &dsc);
    }
    if (redraw_gpu) {
        lv_point_t pts[sizeof(gpu_graph)];
        lv_draw_line_dsc_t dsc;
        lv_draw_line_dsc_init(&dsc);
        dsc.width = 1;
        dsc.color = lv_color_hex(0xFF0000);
        dsc.opa = LV_OPA_100;
        lv_canvas_fill_bg(ui_GpuGraph, lv_color_white(), LV_OPA_100);
        v = *gpu_graph.peek(0);
        pts[0].x = 0;
        pts[0].y = 36 - (v / 100.0f) * 36;
        for (size_t i = 1; i < gpu_graph.size(); ++i) {
            v = *gpu_graph.peek(i);
            pts[i].x = i;
            pts[i].y = 36 - (v / 100.0f) * 36;
        }
        lv_canvas_draw_line(ui_GpuGraph, pts, gpu_graph.size(), &dsc);
    }
}
static float screen_1_cpu_min=NAN,screen_1_cpu_max=NAN;
static float screen_1_gpu_min=NAN,screen_1_gpu_max=NAN;
static void update_screen_1() {
    uint8_t tmp;
    uint8_t v;
    bool redraw_cpu, redraw_gpu;
    float cpu_scale, gpu_scale;
    char sz[64];
    union {
        float f;
        uint8_t b[4];
    } fbu;
    redraw_cpu = false;
    redraw_gpu = false;
    if (USBSerial.available()) {
        int i = USBSerial.readBytes(fbu.b, sizeof(fbu.b));
        if (i == 0) {
            USBSerial.write('@');
        } else {
            if (cpu_graph.full()) {
                cpu_graph.get(&tmp);
            }
            v = (fbu.f + .5);
            cpu_graph.put(v);
            if(screen_1_cpu_min!=screen_1_cpu_min||screen_1_cpu_min>v) {
                screen_1_cpu_min = v;
            }
            if(screen_1_cpu_max!=screen_1_cpu_max||screen_1_cpu_max<v) {
                screen_1_cpu_max = v;
            }
            cpu_scale = screen_1_cpu_max-screen_1_cpu_min+1;
            float offs = - (screen_1_cpu_min*cpu_scale);
            redraw_cpu = true;
            lv_bar_set_value(ui_CpuBar, ((v/cpu_scale)+offs)*100, LV_ANIM_ON);
            snprintf(sz, sizeof(sz), "%0.1fGHz", fbu.f/1000.0);
            lv_label_set_text(ui_CpuGhzLabel, sz);
            if (USBSerial.available()) {
                i = USBSerial.readBytes(fbu.b, sizeof(fbu.b));
                if (i != 0) {
                    if (gpu_graph.full()) {
                        gpu_graph.get(&tmp);
                    }
                    v = (fbu.f + .5);
                    gpu_graph.put(v);
                    if(screen_1_gpu_min!=screen_1_gpu_min||screen_1_gpu_min>v) {
                        screen_1_gpu_min = v;
                    }
                    if(screen_1_gpu_max!=screen_1_gpu_max||screen_1_gpu_max<v) {
                        screen_1_gpu_max = v;
                    }
                    gpu_scale = screen_1_gpu_max-screen_1_gpu_min+1;
                    offs = - (screen_1_gpu_min*gpu_scale);
                    redraw_gpu = true;
                    lv_bar_set_value(ui_CpuBar, ((v/gpu_scale)+offs)*100, LV_ANIM_ON);
                    snprintf(sz, sizeof(sz), "%0.1fGHz", fbu.f/1000.0);
                    lv_label_set_text(ui_GpuGhzLabel, sz);
                } else {
                    USBSerial.write('@');
                }
            } else {
                USBSerial.write('@');
            }
        }
    } else {
        USBSerial.write('@');
    }
    if (redraw_cpu) {
        float offs = - (screen_1_cpu_min*cpu_scale);
        lv_point_t pts[sizeof(cpu_graph)];
        lv_draw_line_dsc_t dsc;
        lv_draw_line_dsc_init(&dsc);
        dsc.width = 1;
        dsc.color = lv_color_hex(0x0000FF);
        dsc.opa = LV_OPA_100;
        lv_canvas_fill_bg(ui_CpuGraph, lv_color_white(), LV_OPA_100);
        v = *cpu_graph.peek(0);
        pts[0].x = 0;
        pts[0].y = 36 - ((v/cpu_scale)+offs) * 36;
        for (size_t i = 1; i < cpu_graph.size(); ++i) {
            v = *cpu_graph.peek(i);
            pts[i].x = i;
            pts[i].y = 36 - ((v/cpu_scale)+offs) * 36;
        }
        lv_canvas_draw_line(ui_CpuGraph, pts, cpu_graph.size(), &dsc);
    }
    if (redraw_gpu) {
        float offs = - (screen_1_gpu_min*gpu_scale);
        lv_point_t pts[sizeof(gpu_graph)];
        lv_draw_line_dsc_t dsc;
        lv_draw_line_dsc_init(&dsc);
        dsc.width = 1;
        dsc.color = lv_color_hex(0xFF0000);
        dsc.opa = LV_OPA_100;
        lv_canvas_fill_bg(ui_GpuGraph, lv_color_white(), LV_OPA_100);
        v = *gpu_graph.peek(0);
        pts[0].x = 0;
        pts[0].y = 36 - ((v/cpu_scale)+offs) * 36;
        for (size_t i = 1; i < gpu_graph.size(); ++i) {
            v = *gpu_graph.peek(i);
            pts[i].x = i;
            pts[i].y = 36 - ((v/cpu_scale)+offs) * 36;
        }
        lv_canvas_draw_line(ui_GpuGraph, pts, gpu_graph.size(), &dsc);
    }
}
static int ticker = 0;
void loop() {
    button_prev.update();
    button_next.update();
    if (ticker++ >= 33) {
        ticker = 0;
        switch (screen) {
            case 0:
                update_screen_0();
            break;
            case 1:
                update_screen_1(); 
            break;
        }
    }

    lv_timer_handler();
    delay(3);
}