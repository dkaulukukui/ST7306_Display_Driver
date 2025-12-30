#ifndef ST7305_MONO_H
#define ST7305_MONO_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <SPI.h>

// Display resolution
// NOTE: ST7305 datasheet specifies 264x320, but supporting 300x400 as specified
// Adjust these values to match your actual display panel
#define ST7305_WIDTH  300
#define ST7305_HEIGHT 400

// Display buffer size (1 bit per pixel: 24 bits per 12×2 pixel block)
// Formula: (WIDTH / 12) * 3 bytes * (HEIGHT / 2) = 25 * 3 * 200 = 15,000 bytes
#define ST7305_BUFFER_SIZE (((ST7305_WIDTH / 12) * 3 * (ST7305_HEIGHT / 2)))

// Color definitions for monochrome display
#define ST7305_BLACK 0
#define ST7305_WHITE 1

// ST7305 Commands
#define ST7305_NOP        0x00
#define ST7305_SWRESET    0x01
#define ST7305_RDDID      0x04
#define ST7305_RDDST      0x09
#define ST7305_RDDPM      0x0A
#define ST7305_SLPIN      0x10
#define ST7305_SLPOUT     0x11
#define ST7305_PTLON      0x12
#define ST7305_PTLOFF     0x13
#define ST7305_INVOFF     0x20
#define ST7305_INVON      0x21
#define ST7305_DISPOFF    0x28
#define ST7305_DISPON     0x29
#define ST7305_CASET      0x2A
#define ST7305_RASET      0x2B
#define ST7305_RAMWR      0x2C
#define ST7305_RAMRD      0x2E
#define ST7305_TEOFF      0x34
#define ST7305_TEON       0x35
#define ST7305_MADCTL     0x36
#define ST7305_VSCSAD     0x37
#define ST7305_HPM        0x38  // High Power Mode
#define ST7305_LPM        0x39  // Low Power Mode
#define ST7305_DTFORM     0x3A  // Data Format Select
#define ST7305_WRMEMC     0x3C
#define ST7305_RDMEMC     0x3E
#define ST7305_TESCAN     0x44
#define ST7305_RDID1      0xDA
#define ST7305_RDID2      0xDB
#define ST7305_RDID3      0xDC

// ST7305 Extended Commands
#define ST7305_GATESET    0xB0  // Gate Line Setting
#define ST7305_FSTCOM     0xB1  // First Gate Setting
#define ST7305_FRCTRL     0xB2  // Frame Rate Control
#define ST7305_GTUPEQH    0xB3  // Gate EQ Control HPM
#define ST7305_GTUPEQL    0xB4  // Gate EQ Control LPM
#define ST7305_SOUEQ      0xB7  // Source EQ Enable
#define ST7305_PNLSET     0xB8  // Panel Setting
#define ST7305_GAMAMS     0xB9  // Gamma Mode Setting
#define ST7305_CLRAM      0xBB  // Clear RAM
#define ST7305_GCTRL      0xC0  // Gate Voltage Control
#define ST7305_VSHPCTRL   0xC1  // VSHP Control
#define ST7305_VSLPCTRL   0xC2  // VSLP Control
#define ST7305_VSHNCTRL   0xC4  // VSHN Control
#define ST7305_VSLNCTRL   0xC5  // VSLN Control
#define ST7305_VSIKCTRL   0xC8  // Source Gamma Voltage
#define ST7305_VSHLSEL    0xC9  // Source Voltage Select
#define ST7305_ID1SET     0xCA  // ID1 Setting
#define ST7305_ID2SET     0xCB  // ID2 Setting
#define ST7305_ID3SET     0xCC  // ID3 Setting
#define ST7305_AUTOPWRCTRL 0xD0 // Auto Power Down
#define ST7305_BSTEN      0xD1  // Booster Enable
#define ST7305_NVMLOADCTRL 0xD6 // NVM Load Control
#define ST7305_OSCSET     0xD8  // OSC Setting

// ============================================================================
// Select active init commands - CHANGE THIS LINE TO SWITCH CONFIGS
// ============================================================================
//#define ACTIVE_INIT_CMDS st7305_init_cmds_default
#define ACTIVE_INIT_CMDS st7305_init_cmds_mfg
//#define ACTIVE_INIT_CMDS st7305_init_cmds_FT_tele
//#define ACTIVE_INIT_CMDS st7305_init_cmds_kevin
// Options: st7305_init_cmds_default, st7305_init_cmds_FT_tele, st7305_init_cmds_kevin
// ============================================================================

// Initialization command structure
typedef struct {
    uint8_t cmd;
    uint8_t data[10];
    uint8_t len;
    uint8_t delay_ms;
} st7305_lcd_init_cmd_t;

static const st7305_lcd_init_cmd_t st7305_init_cmds_default[] = {
    {0xD6, {0x17, 0x00}, 2, 0},                                      // NVM Load Control
    {0xD1, {0x01}, 1, 0},                                            // Booster Enable
    {0xC0, {0x0E, 0x0A}, 2, 0},                                      // Gate Voltage Setting
    {0xC1, {0x41, 0x41, 0x41, 0x41}, 4, 0},                          // VSHP Setting
    {0xC2, {0x32, 0x32, 0x32, 0x32}, 4, 0},                          // VSLP Setting
    {0xC4, {0x46, 0x46, 0x46, 0x46}, 4, 0},                          // VSHN Setting
    {0xC5, {0x46, 0x46, 0x46, 0x46}, 4, 0},                          // VSLN Setting
    {0xB2, {0x12}, 1, 0},                                            // Frame Rate Control
    {0xB3, {0xE5, 0xF6, 0x05, 0x46, 0x77, 0x77, 0x77, 0x77, 0x76, 0x45}, 10, 0}, // Gate EQ HPM
    {0xB4, {0x05, 0x46, 0x77, 0x77, 0x77, 0x77, 0x76, 0x45}, 8, 0}, // Gate EQ LPM
    {0xB7, {0x13}, 1, 0},                                            // Source EQ Enable
    {0xB0, {0x64}, 1, 0},                                            // Gate Line Setting: 400 lines
    {0x11, {}, 0, 120},                                              // Sleep Out
    {0xD8, {0x26, 0xE9}, 2, 0},                                      // OSC Setting
    {0xC9, {0x00}, 1, 0},                                            // Source Voltage Select
    {0x36, {0x00}, 1, 0},                                            // Memory Data Access Control
    {0x3A, {0x11}, 1, 0},                                            // Data Format Select
    {0xB9, {0x20}, 1, 0},                                            // Gamma Mode Setting: Mono
    {0xB8, {0x29}, 1, 0},                                            // Panel Setting
    {0x2A, {0x13, 0x28}, 2, 0},                                      // Column Address Setting
    {0x2B, {0x00, 0xC7}, 2, 0},                                      // Row Address Setting
    {0x35, {0x00}, 1, 0},                                            // Tearing Effect Line On
    {0xD0, {0xFF}, 1, 0},                                            // Auto Power Down
    {0x39, {}, 0, 0},                                                // Low Power Mode
    {0x29, {}, 0, 10},                                               // Display On
};

static const st7305_lcd_init_cmd_t st7305_init_cmds_FT_tele[] = {
    {0xD6, {0x17, 0x02}, 2, 0},                                      // NVM Load Control
    {0xD1, {0x01}, 1, 0},                                            // Booster Enable
    {0xC0, {0x11, 0x04}, 2, 0},                                      // Gate Voltage Setting
    {0xC1, {0x41, 0x41, 0x41, 0x41}, 4, 0},                          // VSHP Setting
    {0xC2, {0x19, 0x19, 0x19, 0x19}, 4, 0},                          // VSLP Setting
    {0xC4, {0x41, 0x41, 0x41, 0x41}, 4, 0},                          // VSHN Setting
    {0xC5, {0x19, 0x19, 0x19, 0x19}, 4, 0},                          // VSLN Setting
    {0xB2, {0x05}, 1, 0},                                            // Frame Rate Control
    {0xB3, {0xE5, 0xF6, 0x05, 0x46, 0x77, 0x77, 0x77, 0x77, 0x76, 0x45}, 10, 0}, // Gate EQ HPM
    {0xB4, {0x05, 0x46, 0x77, 0x77, 0x77, 0x77, 0x76, 0x45}, 8, 0}, // Gate EQ LPM
    {0x62, {0x32, 0x03, 0x1F}, 3, 0},                              // Gate Timing Control 
    {0xB7, {0x13}, 1, 0},                                            // Source EQ Enable
    {0xB0, {0x64}, 1, 0},                                            // Gate Line Setting: 400 lines
    {0x11, {}, 0, 255},                                              // Sleep Out
    {0xD8, {0xA6, 0xE9}, 2, 0},                                      // OSC Setting
    {0xC9, {0x00}, 1, 0},                                            // Source Voltage Select
    {0x36, {0x48}, 1, 0},                                            // Memory Data Access Control
    {0x3A, {0x11}, 1, 0},                                            // Data Format Select
    {0xB9, {0x20}, 1, 0},                                            // Gamma Mode Setting: Mono
    {0xB8, {0x29}, 1, 0},                                            // Panel Setting
    {0x2A, {0x12, 0x2B}, 2, 0},                                      // Column Address Setting
    {0x2B, {0x00, 0xC7}, 2, 0},                                      // Row Address Setting
    {0x35, {0x00}, 1, 0},                                            // Tearing Effect Line On
    {0xD0, {0xFF}, 1, 0},                                            // Auto Power Down
    {0x39, {}, 0, 0},                                                // Low Power Mode
    {0x29, {}, 0, 10},                                               // Display On
};

static const st7305_lcd_init_cmd_t st7305_init_cmds_kevin[] = {
    {0xD6, {0x13, 0x02}, 2, 0},                                      // NVM Load Control
    {0xD1, {0x01}, 1, 0},                                            // Booster Enable
    {0xC0, {0x08, 0x06}, 2, 0},                                      // Gate Voltage Setting
    {0xC1, {0x3C, 0x3E, 0x3C, 0x3C}, 4, 0},                          // VSHP Setting (4.8V)
    {0xC2, {0x23, 0x21, 0x23, 0x23}, 4, 0},                          // VSLP Setting (0.98V)
    {0xC4, {0x5A, 0x5C, 0x5A, 0x5A}, 4, 0},                          // VSHN Setting (-3.6V)
    {0xC5, {0x37, 0x35, 0x37, 0x37}, 4, 0},                          // VSLN Setting (0.22V)
    {0xB2, {0x05}, 1, 0},                                            // Frame Rate Control
    {0xB3, {0xE5, 0xF6, 0x17, 0x77, 0x77, 0x77, 0x77, 0x77, 0x77, 0x71}, 10, 0}, // Gate EQ HPM
    {0xB4, {0x05, 0x46, 0x77, 0x77, 0x77, 0x77, 0x76, 0x45}, 8, 0}, // Gate EQ LPM
    {0x62, {0x32, 0x03, 0x1F}, 3, 0},                                // Gate Timing Control
    {0xB7, {0x13}, 1, 0},                                            // Source EQ Enable
    {0xB0, {0x60}, 1, 0},                                            // Gate Line Setting: 384 lines
    {0x11, {}, 0, 100},                                              // Sleep Out
    {0xC9, {0x00}, 1, 0},                                            // Source Voltage Select
    {0x36, {0x00}, 1, 0},                                            // Memory Data Access Control
    {0x3A, {0x11}, 1, 0},                                            // Data Format Select
    {0xB9, {0x20}, 1, 0},                                            // Gamma Mode Setting: Mono
    {0xB8, {0x29}, 1, 0},                                            // Panel Setting
    {0x2A, {0x17, 0x24}, 2, 0},                                      // Column Address Setting
    {0x2B, {0x00, 0xBF}, 2, 0},                                      // Row Address Setting
    {0xD0, {0xFF}, 1, 0},                                            // Auto Power Down
    {0x39, {}, 0, 0},                                                // Low Power Mode
    {0x29, {}, 0, 100},                                              // Display On
};

static const st7305_lcd_init_cmd_t st7305_init_cmds_mfg[] = {
    {0xD6, {0x17, 0x02}, 2, 0},                                      // NVM Load Control
    {0xD1, {0x01}, 1, 0},                                            // Booster Enable
    {0xC0, {0x11, 0x04}, 2, 0},                                      // Gate Voltage Setting
    {0xC1, {0x69, 0x69, 0x69, 0x69}, 4, 0},                          // VSHP Setting (4.8V)
    {0xC2, {0x19, 0x19, 0x19, 0x19}, 4, 0},                          // VSLP Setting (0.98V)
    {0xC4, {0x4B, 0x4B, 0x4B, 0x4B}, 4, 0},                          // VSHN Setting (-3.6V)
    {0xC5, {0x19, 0x19, 0x19, 0x19}, 4, 0},                          // VSLN Setting (0.22V)
    {0xD8, {0xA6, 0xE9}, 2, 0},                                      // OSC Setting
    {0xB2, {0x02}, 1, 0},                                            // Frame Rate Control
    {0xB3, {0xE5, 0xF6, 0x05, 0x46, 0x77, 0x77, 0x77, 0x77, 0x76, 0x45}, 10, 0}, // Gate EQ HPM
    {0xB4, {0x05, 0x46, 0x77, 0x77, 0x77, 0x77, 0x76, 0x45}, 8, 0}, // Gate EQ LPM
    {0x62, {0x32, 0x03, 0x1F}, 3, 0},                                // Gate Timing Control
    {0xB7, {0x13}, 1, 0},                                            // Source EQ Enable
    {0xB0, {0x64}, 1, 0},                                            // Gate Line Setting: 384 lines
    {0x11, {}, 0, 100},                                              // Sleep Out
    {0xC9, {0x00}, 1, 0},                                            // Source Voltage Select
    {0x36, {0x48}, 1, 0},                                            // Memory Data Access Control
    {0x3A, {0x11}, 1, 0},                                            // Data Format Select
    {0xB9, {0x20}, 1, 0},                                            // Gamma Mode Setting: Mono
    {0xB8, {0x29}, 1, 0},
    {0x21, {}, 0, 0},                                            // Panel Setting
    {0x2A, {0x12, 0x2A}, 2, 0},                                      // Column Address Setting
    {0x2B, {0x00, 0xC7}, 2, 0},                                      // Row Address Setting
    {0xD0, {0xFF}, 1, 0},                                            // Auto Power Down
    {0x38, {}, 0, 0},                                                // Low Power Mode
    {0x29, {}, 0, 100},                                              // Display On
};

class ST7305_Mono : public Adafruit_GFX {
public:
    ST7305_Mono(int8_t dc, int8_t rst, int8_t cs);
    ~ST7305_Mono();

    // Required GFX functions
    void drawPixel(int16_t x, int16_t y, uint16_t color) override;
    
    // Display control
    bool begin(uint32_t spiFrequency = 40000000, 
               const st7305_lcd_init_cmd_t* initCmds = ACTIVE_INIT_CMDS,
               size_t cmdCount = sizeof(ACTIVE_INIT_CMDS) / sizeof(st7305_lcd_init_cmd_t));
    void display();
    void clearDisplay();
    void invertDisplay(bool invert);
    void setContrast(uint8_t contrast);
    
    // Power mode control
    void setHighPowerMode();    // For faster refresh (HPM)
    void setLowPowerMode();     // For power saving (LPM)
    
    // Direct buffer access
    uint8_t* getBuffer() { return buffer; }
    
private:
    int8_t _dc, _rst, _cs;
    uint8_t *buffer;
    SPISettings spiSettings;
    
    // Low-level SPI functions
    void sendCommand(uint8_t cmd);
    void sendData(uint8_t data);
    void sendDataBatch(const uint8_t *data, uint32_t size);
    
    // Initialization
    void hardwareReset();
    //void initDisplay();
    void initDisplay(const st7305_lcd_init_cmd_t* st7305_init_cmds, size_t cmd_count);
    
    
    // Helper functions
    void setAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void csLow();
    void csHigh();
    void dcLow();
    void dcHigh();
};

#endif // ST7305_MONO_H
