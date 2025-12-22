#include <Arduino.h>
#include <lvgl.h>
#include "ST7306_LCD.h"
#include "ST7306_LVGL.h"

// =======================================================
// --- Pin Configuration ---
// Adjust these according to your actual wiring
// =======================================================
#define PIN_TE 5
#define PIN_CLK 4
#define PIN_MOSI 3
#define PIN_CS 2
#define PIN_DC 1
#define PIN_RST 0

// =======================================================
// --- Orientation Settings ---
// Set to 1 for portrait (210x480), 0 for landscape (480x210)
// =======================================================
#define SCREEN_ORIENTATION_PORTRAIT 1

#if SCREEN_ORIENTATION_PORTRAIT
    #define RENDER_MODE_LANDSCAPE 0
#else
    #define RENDER_MODE_LANDSCAPE 1
#endif

// =======================================================
// --- Global Variables ---
// =======================================================
ST7306_LCD lcd(PIN_MOSI, PIN_CLK, PIN_CS, PIN_DC, PIN_RST, PIN_TE);

static lv_disp_draw_buf_t disp_buf;
static lv_disp_drv_t disp_drv;

#define BUF_ROWS 40

// =======================================================
// --- LVGL Demo UI ---
// =======================================================
void lv_demo_ui() {
    lv_obj_t *scr = lv_disp_get_scr_act(NULL);
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    // Title label
    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "ST7306 Display");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_text_color(title, lv_color_hex(0xF800), LV_PART_MAIN);

    // Create a colored rectangle
    lv_obj_t *rect = lv_obj_create(scr);
    lv_obj_set_size(rect, 100, 100);
    lv_obj_align(rect, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(rect, lv_color_hex(0x07E0), LV_PART_MAIN);
    lv_obj_set_style_border_width(rect, 0, LV_PART_MAIN);

    // Add a label inside the rectangle
    lv_obj_t *label = lv_label_create(rect);
    lv_label_set_text(label, "LVGL\nDemo");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0x0000), LV_PART_MAIN);

    // Progress bar at bottom
    lv_obj_t *bar = lv_bar_create(scr);
    lv_obj_set_size(bar, 150, 20);
    lv_bar_set_value(bar, 75, LV_ANIM_ON);
    lv_obj_align(bar, LV_ALIGN_BOTTOM_MID, 0, -10);
}

// =======================================================
// --- Setup Function ---
// =======================================================
void setup() {
    Serial.begin(115200);
    delay(300);
    
    Serial.println("Initializing ST7306 Display...");
    
    // Initialize LCD
    lcd.begin();
    lcd.clearDisplay();
    
    Serial.println("Initializing LVGL...");
    
    // Initialize LVGL
    lv_init();
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf = &disp_buf;

    #if RENDER_MODE_LANDSCAPE
        // Landscape mode (480x210)
        static lv_color_t buf[ST7306_HEIGHT * BUF_ROWS];
        lv_disp_draw_buf_init(&disp_buf, buf, NULL, ST7306_HEIGHT * BUF_ROWS);
        
        disp_drv.hor_res = lcd.height();
        disp_drv.ver_res = lcd.width();
        disp_drv.flush_cb = disp_flush_landscape;
        
        Serial.println("LVGL Mode: Landscape (480x210)");
    #else
        // Portrait mode (210x480)
        static lv_color_t buf[ST7306_WIDTH * BUF_ROWS];
        lv_disp_draw_buf_init(&disp_buf, buf, NULL, ST7306_WIDTH * BUF_ROWS);
        
        disp_drv.hor_res = lcd.width();
        disp_drv.ver_res = lcd.height();
        disp_drv.flush_cb = disp_flush;
        
        Serial.println("LVGL Mode: Portrait (210x480)");
    #endif

    lv_disp_drv_register(&disp_drv);
    
    Serial.println("Creating UI...");
    lv_demo_ui();
    
    Serial.println("Setup complete!");
}

// =======================================================
// --- Loop Function ---
// =======================================================
void loop() {
    lv_timer_handler();
    delay(5);
}
