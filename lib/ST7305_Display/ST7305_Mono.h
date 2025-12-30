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

// Display buffer size (1 bit per pixel for monochrome)
#define ST7305_BUFFER_SIZE ((ST7305_WIDTH * ST7305_HEIGHT) / 8)

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

class ST7305_Mono : public Adafruit_GFX {
public:
    ST7305_Mono(int8_t dc, int8_t rst, int8_t cs);
    ~ST7305_Mono();

    // Required GFX functions
    void drawPixel(int16_t x, int16_t y, uint16_t color) override;
    
    // Display control
    bool begin(uint32_t spiFrequency = 40000000);
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
    void initDisplay();
    
    // Helper functions
    void setAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1);
    void csLow();
    void csHigh();
    void dcLow();
    void dcHigh();
};

#endif // ST7305_MONO_H
