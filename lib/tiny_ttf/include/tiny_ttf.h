// honey the codewitch (github codewitch-honey-crisis)
#ifndef TINY_TTF_H
#define TINY_TTF_H
#include <lvgl.h>
#ifdef __cplusplus
extern "C" {
#endif
#if TINY_TTF_FILE_SUPPORT !=0
// create a font from the specified file or path with the specified line height.
lv_font_t * tiny_ttf_create_file(const char * path, lv_coord_t line_height);
#endif
// create a font from the specified data pointer with the specified line height.
lv_font_t * tiny_ttf_create_data(const void* data, size_t data_size, lv_coord_t line_height);
// set the size of the font to a new line_height
void tiny_ttf_set_size(lv_font_t * font, lv_coord_t line_height);
// destroy a font previously created with lv_tiny_ttf_create_xxxx()
void tiny_ttf_destroy(lv_font_t * font);
#ifdef __cplusplus
}
#endif
#endif // TINY_TTF_H