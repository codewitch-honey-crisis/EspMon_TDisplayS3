# tiny_ttf

## Relatively lightweight TrueType font support for LVGL
----

### Setup 

Your PlatformIO INI should look something like the following
```
[env:tiny_ttf-lvgl]
platform = espressif32
board = node32s
framework = arduino
lib_deps = 
	codewitch-honey-crisis/tiny_ttf@^0.1.0
lib_ldf_mode = deep
```
----
### Use

```c
...
// for file support, define the following
// before including tiny_ttf
// #define TINY_TTF_FILE_SUPPORT 1
#include "tiny_ttf.h"
...
// inside your function

// create a font from Test_ttf with a line height of 30px
// use this method to load a font from an embedded header
// this is the recommended way to load a font
lv_font_t* my_font = tiny_ttf_create_data(Test_ttf,sizeof(Test_ttf),30);

// if file support is enabled the following will also
// be available. Load from file Tiny.ttf
lv_font_t* my_font2 = tiny_ttf_create_file("A:/Tiny.ttf",30);

...
// you can change the size of a font after the fact
tiny_ttf_set_size(my_font,45);
...
// destroy the fonts
// tiny_ttf_destroy(my_font);
// tiny_ttf_destroy(my_font2);
```