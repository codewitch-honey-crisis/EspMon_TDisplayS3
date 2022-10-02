#include "ui_patch.h"

lv_obj_t * ui_CpuGraph;
lv_obj_t * ui_GpuGraph;

void ui_patch(void) {
    ui_CpuGraph = lv_canvas_create(ui_Screen1);
    lv_obj_set_width(ui_CpuGraph, 135);
    lv_obj_set_height(ui_CpuGraph, 37);
    lv_obj_set_x(ui_CpuGraph, -18);
    lv_obj_set_y(ui_CpuGraph, -31);
    lv_obj_set_align(ui_CpuGraph, LV_ALIGN_RIGHT_MID);
    lv_obj_clear_flag(ui_CpuGraph, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_GpuGraph = lv_canvas_create(ui_Screen1);
    lv_obj_set_width(ui_GpuGraph, 135);
    lv_obj_set_height(ui_GpuGraph, 37);
    lv_obj_set_x(ui_GpuGraph, -18);
    lv_obj_set_y(ui_GpuGraph, 45);
    lv_obj_set_align(ui_GpuGraph, LV_ALIGN_RIGHT_MID);
    lv_obj_clear_flag(ui_GpuGraph, LV_OBJ_FLAG_SCROLLABLE);      /// Flags
}