// =======================================================
// lv_conf.h - LVGL Configuration for ST7306 Display
// =======================================================

#ifndef LV_CONF_H
#define LV_CONF_H

#include <stdint.h>

// =======================================================
// --- Resolution Configuration ---
// =======================================================
// Portrait mode (210x480)
#define LV_HOR_RES_MAX   210
#define LV_VER_RES_MAX   480

// =======================================================
// --- Color Configuration ---
// =======================================================
#define LV_COLOR_DEPTH 16
#define LV_COLOR_16_SWAP 0

// =======================================================
// --- Memory Configuration ---
// =======================================================
#define LV_MEM_CUSTOM 0
#define LV_MEM_SIZE (64U * 1024U)  /* 64 KB */

// =======================================================
// --- Display Configuration ---
// =======================================================
#define LV_DPI_DEF 100

// =======================================================
// --- Tick Configuration ---
// =======================================================
#define LV_TICK_CUSTOM 1
#if LV_TICK_CUSTOM
    #define LV_TICK_CUSTOM_INCLUDE <Arduino.h>
    #define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())
#endif

// =======================================================
// --- Font Configuration ---
// =======================================================
#define LV_FONT_MONTSERRAT_8     0
#define LV_FONT_MONTSERRAT_10    0
#define LV_FONT_MONTSERRAT_12    1
#define LV_FONT_MONTSERRAT_14    1
#define LV_FONT_MONTSERRAT_16    1
#define LV_FONT_MONTSERRAT_18    1
#define LV_FONT_MONTSERRAT_20    0
#define LV_FONT_MONTSERRAT_22    0
#define LV_FONT_MONTSERRAT_24    1
#define LV_FONT_MONTSERRAT_26    0
#define LV_FONT_MONTSERRAT_28    0
#define LV_FONT_MONTSERRAT_30    0
#define LV_FONT_MONTSERRAT_32    1
#define LV_FONT_MONTSERRAT_34    0
#define LV_FONT_MONTSERRAT_36    0
#define LV_FONT_MONTSERRAT_38    0
#define LV_FONT_MONTSERRAT_40    0
#define LV_FONT_MONTSERRAT_42    0
#define LV_FONT_MONTSERRAT_44    0
#define LV_FONT_MONTSERRAT_46    0
#define LV_FONT_MONTSERRAT_48    0

#define LV_FONT_FMT_TXT_LARGE 1

// =======================================================
// --- Logging Configuration ---
// =======================================================
#define LV_USE_LOG 0

// =======================================================
// --- Rendering Configuration ---
// =======================================================
#define LV_USE_DRAW_SW 1

// Hardware GPU (not used)
#define LV_USE_GPU_STM32_DMA2D 0
#define LV_USE_GPU_NXP_PXP 0
#define LV_USE_GPU_NXP_VG_LITE 0
#define LV_USE_GPU_SDL 0

// =======================================================
// --- Feature Configuration ---
// =======================================================
#define LV_BUILD_EXAMPLES 1

// =======================================================
// --- Theme Configuration ---
// =======================================================
#define LV_USE_THEME_DEFAULT 1
#if LV_USE_THEME_DEFAULT
    #define LV_THEME_DEFAULT_DARK 0
    #define LV_THEME_DEFAULT_GROW 1
    #define LV_THEME_DEFAULT_FONT_SMALL       &lv_font_montserrat_12
    #define LV_THEME_DEFAULT_FONT_NORMAL      &lv_font_montserrat_16
    #define LV_THEME_DEFAULT_FONT_SUBTITLE    &lv_font_montserrat_16
    #define LV_THEME_DEFAULT_FONT_TITLE       &lv_font_montserrat_16
#endif

// =======================================================
// --- Widget Configuration ---
// =======================================================
#define LV_USE_ARC          1
#define LV_USE_BAR          1
#define LV_USE_BTN          1
#define LV_USE_BTNMATRIX    1
#define LV_USE_CANVAS       0
#define LV_USE_CHECKBOX     1
#define LV_USE_DROPDOWN     1
#define LV_USE_IMG          1
#define LV_USE_LABEL        1
#define LV_USE_LINE         0
#define LV_USE_ROLLER       1
#define LV_USE_SLIDER       1
#define LV_USE_SWITCH       1
#define LV_USE_TEXTAREA     1
#define LV_USE_TABLE        1
#define LV_USE_TABVIEW      1

// =======================================================
// --- Layout Configuration ---
// =======================================================
#define LV_USE_FLEX         1
#define LV_USE_GRID         0

// =======================================================
// --- Animation Configuration ---
// =======================================================
#define LV_USE_ANIMATION    1

// =======================================================
// --- File System Configuration ---
// =======================================================
#define LV_USE_FS_STDIO     0
#define LV_USE_FS_POSIX     0
#define LV_USE_FS_WIN32     0
#define LV_USE_FS_FATFS     0

// =======================================================
// --- Demos Configuration ---
// =======================================================
#define LV_USE_DEMO_WIDGETS   0
#define LV_USE_DEMO_STRESS    0
#define LV_USE_DEMO_BENCHMARK 0

// =======================================================
// --- Debug Configuration ---
// =======================================================
#define LV_USE_PERF_MONITOR         0
#define LV_USE_MEM_MONITOR          0
#define LV_USE_REFR_DEBUG           0
#define LV_USE_ASSERT_NULL          0
#define LV_USE_ASSERT_MALLOC        0
#define LV_USE_ASSERT_STYLE         0
#define LV_USE_ASSERT_MEM_INTEGRITY 0
#define LV_USE_ASSERT_OBJ           0

// =======================================================
// --- Other Configuration ---
// =======================================================
#define LV_USE_USER_DATA    1
#define LV_SPRINTF_CUSTOM   0

#endif /*LV_CONF_H*/
