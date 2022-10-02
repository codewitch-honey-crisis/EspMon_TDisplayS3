#ifndef UI_PATCH_H
#define UI_PATCH_H
#include "ui.h"
#ifdef __cplusplus
extern "C" {
#endif

#if defined __has_include
#if __has_include("lvgl.h")
#include "lvgl.h"
#elif __has_include("lvgl/lvgl.h")
#include "lvgl/lvgl.h"
#else
#include "lvgl.h"
#endif
#else
#include "lvgl.h"
#endif
extern lv_obj_t * ui_CpuGraph;
extern lv_obj_t * ui_GpuGraph;

void ui_patch(void);
#ifdef __cplusplus
}
#endif

#endif