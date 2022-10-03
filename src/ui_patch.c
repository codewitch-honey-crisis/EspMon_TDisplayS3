#include "ui_patch.h"

lv_obj_t * ui_CpuGraph;
lv_obj_t * ui_GpuGraph;
lv_obj_t * ui_CpuGhzGraph;
lv_obj_t * ui_GpuGhzGraph;

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

    ui_CpuGhzGraph = lv_canvas_create(ui_Screen2);
    lv_obj_set_width(ui_CpuGhzGraph, 135);
    lv_obj_set_height(ui_CpuGhzGraph, 37);
    lv_obj_set_x(ui_CpuGhzGraph, -18);
    lv_obj_set_y(ui_CpuGhzGraph, -31);
    lv_obj_set_align(ui_CpuGhzGraph, LV_ALIGN_RIGHT_MID);
    lv_obj_clear_flag(ui_CpuGhzGraph, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

    ui_GpuGhzGraph = lv_canvas_create(ui_Screen2);
    lv_obj_set_width(ui_GpuGhzGraph, 135);
    lv_obj_set_height(ui_GpuGhzGraph, 37);
    lv_obj_set_x(ui_GpuGhzGraph, -18);
    lv_obj_set_y(ui_GpuGhzGraph, 45);
    lv_obj_set_align(ui_GpuGhzGraph, LV_ALIGN_RIGHT_MID);
    lv_obj_clear_flag(ui_GpuGhzGraph, LV_OBJ_FLAG_SCROLLABLE);      /// Flags

}