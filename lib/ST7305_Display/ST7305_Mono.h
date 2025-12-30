/**
 * ST7305_Mono.h
 * 
 * ST7305 Monochrome Display Driver for Arduino
 * 
 * This library provides a driver for ST7305-based monochrome displays with:
 * - 300×400 pixel resolution
 * - 4 pixels per byte horizontal packing
 * - 2 rows per vertical group
 * - 15,000 byte frame buffer
 * - Adafruit GFX library integration
 * - Multiple initialization configurations
 * - SPI communication up to 40MHz
 * 
 * Memory Layout:
 *   Buffer Size = (WIDTH / 4) × (HEIGHT / 2) = 75 × 200 = 15,000 bytes
 *   Each byte contains 4 horizontal pixels across 2 vertical rows
 *   Bit mapping: 7-(line_bit_4*2+one_two)
 * 
 * Configuration:
 *   Change ACTIVE_INIT_CMDS macro to switch between init command sets:
 *   - st7305_init_cmds_default: Standard initialization
 *   - st7305_init_cmds_FT_tele: FT_tele reference implementation
 *   - st7305_init_cmds_kevin: Custom configuration variant
 *   - st7305_init_cmds_mfg: Manufacturer settings
 * 
 * Author: Based on FT_tele_ST7305 reference implementation
 * License: Open source
 */

#ifndef ST7305_MONO_H
#define ST7305_MONO_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <SPI.h>

// ============================================================================
// Display Configuration
// ============================================================================

// Display resolution
// NOTE: ST7305 datasheet specifies 264x320, but supporting 300x400 as specified
// Adjust these values to match your actual display panel
#define ST7305_WIDTH  300
#define ST7305_HEIGHT 400

// Display buffer size calculation:
// ST7305 uses 4 pixels per byte horizontally, 2 rows per vertical group
// Formula: (WIDTH / 4) * (HEIGHT / 2) = 75 * 200 = 15,000 bytes
// DO NOT change this formula unless hardware layout changes
#define ST7305_BUFFER_SIZE (((ST7305_WIDTH / 4) * (ST7305_HEIGHT / 2)))

// Color definitions for monochrome display
#define ST7305_BLACK 0  // Bit value 0 = Black pixel
#define ST7305_WHITE 1  // Bit value 1 = White pixel

// ============================================================================
// ST7305 Command Definitions
// ============================================================================

// Basic Commands
#define ST7305_NOP        0x00  // No Operation
#define ST7305_SWRESET    0x01  // Software Reset
#define ST7305_RDDID      0x04  // Read Display ID
#define ST7305_RDDST      0x09  // Read Display Status
#define ST7305_RDDPM      0x0A  // Read Display Power Mode
#define ST7305_SLPIN      0x10  // Sleep In
#define ST7305_SLPOUT     0x11  // Sleep Out
#define ST7305_PTLON      0x12  // Partial Mode ON
#define ST7305_PTLOFF     0x13  // Partial Mode OFF
#define ST7305_INVOFF     0x20  // Display Inversion OFF
#define ST7305_INVON      0x21  // Display Inversion ON
#define ST7305_DISPOFF    0x28  // Display OFF
#define ST7305_DISPON     0x29  // Display ON
#define ST7305_CASET      0x2A  // Column Address Set
#define ST7305_RASET      0x2B  // Row Address Set
#define ST7305_RAMWR      0x2C  // Memory Write
#define ST7305_RAMRD      0x2E  // Memory Read
#define ST7305_TEOFF      0x34  // Tearing Effect Line OFF
#define ST7305_TEON       0x35  // Tearing Effect Line ON
#define ST7305_MADCTL     0x36  // Memory Access Control
#define ST7305_VSCSAD     0x37  // Vertical Scroll Start Address
#define ST7305_HPM        0x38  // Idle Mode OFF (High Power Mode)
#define ST7305_LPM        0x39  // Idle Mode ON (Low Power Mode)
#define ST7305_DTFORM     0x3A  // Data Format Select
#define ST7305_WRMEMC     0x3C  // Write Memory Continue
#define ST7305_RDMEMC     0x3E  // Read Memory Continue
#define ST7305_TESCAN     0x44  // Set Tear Scanline
#define ST7305_RDID1      0xDA  // Read ID1
#define ST7305_RDID2      0xDB  // Read ID2
#define ST7305_RDID3      0xDC  // Read ID3

// Extended Commands (Voltage, Timing, and Power Control)
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
// Configuration Selection - CHANGE THIS LINE TO SWITCH INIT COMMANDS
// ============================================================================
// Uncomment ONE line below to select the active init command set:
//#define ACTIVE_INIT_CMDS st7305_init_cmds_default
#define ACTIVE_INIT_CMDS st7305_init_cmds_mfg
//#define ACTIVE_INIT_CMDS st7305_init_cmds_FT_tele
//#define ACTIVE_INIT_CMDS st7305_init_cmds_kevin
//
// Available configurations:
// - st7305_init_cmds_default: Standard initialization sequence
// - st7305_init_cmds_FT_tele: Based on FT_tele reference library
// - st7305_init_cmds_kevin: Custom configuration example
// - st7305_init_cmds_mfg: Manufacturer recommended settings
//
// After changing, rebuild and upload to apply new configuration.
// ============================================================================

// ============================================================================
// Initialization Command Structure
// ============================================================================

/**
 * Initialization command structure for ST7305
 * 
 * Each command consists of:
 * - cmd: Command byte to send
 * - data[10]: Array of up to 10 data bytes
 * - len: Number of data bytes (0-10)
 * - delay_ms: Milliseconds to wait after sending command
 */
typedef struct {
    uint8_t cmd;         // Command byte
    uint8_t data[10];    // Data bytes (up to 10)
    uint8_t len;         // Number of data bytes
    uint8_t delay_ms;    // Delay after command (milliseconds)
} st7305_lcd_init_cmd_t;

// ============================================================================
// Initialization Command Arrays
// ============================================================================

/**
 * Default Initialization Commands
 * Standard initialization sequence for ST7305 displays
 */
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

// ============================================================================
// ST7305_Mono Class - Main Display Driver
// ============================================================================

/**
 * ST7305_Mono - Monochrome Display Driver Class
 * 
 * Inherits from Adafruit_GFX to provide full graphics library support.
 * 
 * Features:
 * - 300×400 pixel monochrome display support
 * - 15KB frame buffer with efficient 4-pixel-per-byte layout
 * - Hardware SPI communication up to 40MHz
 * - Multiple initialization configurations
 * - Power management (high/low power modes)
 * - Display inversion and fill operations
 * 
 * Usage:
 *   ST7305_Mono display(DC_PIN, RST_PIN, CS_PIN);
 *   display.begin(1000000);  // Initialize at 1MHz
 *   display.clearDisplay();
 *   display.drawPixel(10, 10, ST7305_WHITE);
 *   display.display();  // Update screen
 */
class ST7305_Mono : public Adafruit_GFX {
public:
    // ========================================================================
    // Constructor & Destructor
    // ========================================================================
    
    /**
     * Constructor - Create display driver instance
     * @param dc  Data/Command control pin
     * @param rst Reset pin (-1 if not used)
     * @param cs  Chip Select pin
     */
    ST7305_Mono(int8_t dc, int8_t rst, int8_t cs);
    
    /**
     * Destructor - Free allocated frame buffer
     */
    ~ST7305_Mono();

    // ========================================================================
    // Core Display Functions
    // ========================================================================
    
    /**
     * drawPixel - Draw single pixel (Adafruit_GFX override)
     * 
     * Core drawing primitive used by all GFX functions.
     * Maps pixel coordinates to buffer using 4-pixel-per-byte layout.
     * 
     * @param x     X coordinate (0-299)
     * @param y     Y coordinate (0-399)
     * @param color ST7305_BLACK or ST7305_WHITE
     */
    void drawPixel(int16_t x, int16_t y, uint16_t color) override;
    
    // ========================================================================
    // Display Control
    // ========================================================================
    
    /**
     * begin - Initialize display hardware
     * 
     * Allocates frame buffer, configures SPI, performs hardware reset,
     * and sends initialization command sequence.
     * 
     * @param spiFrequency SPI clock speed (default 40MHz, recommend 1MHz for stability)
     * @param initCmds     Pointer to init command array (defaults to ACTIVE_INIT_CMDS)
     * @param cmdCount     Number of commands in array (auto-calculated)
     * @return true if successful, false if out of memory
     */
    bool begin(uint32_t spiFrequency = 40000000, 
               const st7305_lcd_init_cmd_t* initCmds = ACTIVE_INIT_CMDS,
               size_t cmdCount = sizeof(ACTIVE_INIT_CMDS) / sizeof(st7305_lcd_init_cmd_t));
    
    /**
     * display - Transfer frame buffer to display
     * 
     * Sends entire 15KB buffer to display hardware via SPI.
     * Must be called after drawing to make changes visible.
     * 
     * Sequence:
     * 1. Set column address range (0x12-0x2A)
     * 2. Set row address range (0x00-0xC7)
     * 3. Send memory write command (0x2C)
     * 4. Transfer buffer via SPI
     * 
     * Time: ~100ms @ 40MHz, ~300ms @ 1MHz
     */
    void display();
    
    /**
     * clearDisplay - Fill buffer with black (0x00)
     * 
     * Clears entire frame buffer to black.
     * Call display() to update screen.
     */
    void clearDisplay();
    
    /**
     * fill - Fill buffer with specific byte value
     * 
     * @param data Byte value to fill buffer with
     *             0x00=all black, 0xFF=all white, 0xAA/0x55=patterns
     */
    void fill(uint8_t data);
    
    /**
     * invertDisplay - Hardware pixel inversion
     * 
     * Inverts all display pixels (white<->black).
     * Hardware feature - doesn't modify buffer.
     * 
     * @param invert true to invert, false for normal
     */
    void invertDisplay(bool invert);
    
    /**
     * setContrast - Adjust display contrast (placeholder)
     * 
     * ST7305 requires voltage adjustments for contrast control.
     * Modify init command voltages (C1, C2, C4, C5) instead.
     * 
     * @param contrast Contrast value (not implemented)
     */
    void setContrast(uint8_t contrast);
    
    // ========================================================================
    // Power Management
    // ========================================================================
    
    /**
     * setHighPowerMode - Enter high power mode (0x38)
     * For faster refresh rates and better performance.
     */
    void setHighPowerMode();
    
    /**
     * setLowPowerMode - Enter low power mode (0x39)
     * For reduced power consumption (idle mode).
     */
    void setLowPowerMode();
    
    // ========================================================================
    // Buffer Access
    // ========================================================================
    
    /**
     * getBuffer - Get direct frame buffer pointer
     * 
     * Provides access to raw buffer for advanced manipulation.
     * Use with caution - incorrect writes can corrupt display.
     * 
     * Buffer format: 15,000 bytes
     * Layout: (y/2) * 75 + (x/4)
     * Bit position: 7 - ((x%4)*2 + (y%2))
     * 
     * @return Pointer to frame buffer
     */
    uint8_t* getBuffer() { return buffer; }
    
private:
    // ========================================================================
    // Private Members
    // ========================================================================
    
    int8_t _dc, _rst, _cs;      // Pin assignments
    uint8_t *buffer;             // Frame buffer pointer (15KB)
    SPISettings spiSettings;     // SPI configuration
    
    // ========================================================================
    // Low-Level SPI Communication
    // ========================================================================
    
    void sendCommand(uint8_t cmd);                           // Send command byte
    void sendData(uint8_t data);                             // Send data byte
    void sendDataBatch(const uint8_t *data, uint32_t size);  // Send multiple bytes
    
    // ========================================================================
    // Initialization Helpers
    // ========================================================================
    
    void hardwareReset();  // Perform hardware reset sequence
    void initDisplay(const st7305_lcd_init_cmd_t* st7305_init_cmds, size_t cmd_count);  // Send init commands
    
    // ========================================================================
    // Utility Functions
    // ========================================================================
    
    void setAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);  // Define update region
    void csLow();    // CS pin low (select device)
    void csHigh();   // CS pin high (deselect device)
    void dcLow();    // DC pin low (command mode)
    void dcHigh();   // DC pin high (data mode)
};

#endif // ST7305_MONO_H
