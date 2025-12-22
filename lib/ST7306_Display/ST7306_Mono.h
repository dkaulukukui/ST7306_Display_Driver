#ifndef ST7306_MONO_H
#define ST7306_MONO_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <SPI.h>

// Display resolution for LH420NB-F07
#define ST7306_WIDTH  300
#define ST7306_HEIGHT 400

// Display buffer size (1 bit per pixel)
#define ST7306_BUFFER_SIZE ((ST7306_WIDTH * ST7306_HEIGHT) / 8)

// Color definitions
#define ST7306_BLACK 0
#define ST7306_WHITE 1

class ST7306_Mono : public Adafruit_GFX {
public:
    ST7306_Mono(int8_t dc, int8_t rst, int8_t cs);
    ~ST7306_Mono();

    // Required GFX functions
    void drawPixel(int16_t x, int16_t y, uint16_t color) override;
    
    // Display control
    bool begin(uint32_t spiFrequency = 40000000);
    void display();
    void clearDisplay();
    void invertDisplay(bool invert);
    void setContrast(uint8_t contrast);
    
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
    void setAddressWindow();
    void csLow();
    void csHigh();
    void dcLow();
    void dcHigh();
};

#endif // ST7306_MONO_H
