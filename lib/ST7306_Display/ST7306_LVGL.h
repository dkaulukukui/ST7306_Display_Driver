#pragma once

#include <Arduino.h>
#include <lvgl.h>
#include "ST7306_LCD.h"

// Performance optimization configuration
#define ST7306_FB_ROW_SIZE (ST7306_WIDTH + 2)
#define BATCH_ROW_COUNT 32

const uint16_t white_list_colors[8] = {
    0xFFFF, // 0: White
    0xF800, // 1: Red
    0x07E0, // 2: Green
    0x001F, // 3: Blue
    0xFFE0, // 4: Yellow
    0x07FF, // 5: Cyan
    0xF81F, // 6: Magenta
    0x0000  // 7: Black
};

// --- LVGL driver function declarations ---

void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
void disp_flush_landscape(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);

// --- Transparent (whitelist) versions ---
void trans_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
void trans_disp_flush_landscape(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p);
