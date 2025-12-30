#include "ST7305_Mono.h"

ST7305_Mono::ST7305_Mono(int8_t dc, int8_t rst, int8_t cs)
    : Adafruit_GFX(ST7305_WIDTH, ST7305_HEIGHT),
      _dc(dc), _rst(rst), _cs(cs), buffer(nullptr) {
}

ST7305_Mono::~ST7305_Mono() {
    if (buffer) {
        free(buffer);
        buffer = nullptr;
    }
}

bool ST7305_Mono::begin(uint32_t spiFrequency) {
    // Configure pins
    pinMode(_dc, OUTPUT);
    pinMode(_cs, OUTPUT);
    if (_rst >= 0) {
        pinMode(_rst, OUTPUT);
    }
    
    // Initialize SPI
    SPI.begin();
    spiSettings = SPISettings(spiFrequency, MSBFIRST, SPI_MODE0);
    
    // Allocate frame buffer
    buffer = (uint8_t*)malloc(ST7305_BUFFER_SIZE);
    if (!buffer) {
        return false;
    }
    
    // Clear buffer
    memset(buffer, 0xFF, ST7305_BUFFER_SIZE);  // Start with white
    
    // Hardware reset and initialize
    hardwareReset();
    initDisplay();
    
    return true;
}

void ST7305_Mono::hardwareReset() {
    if (_rst >= 0) {
        digitalWrite(_rst, HIGH);
        delay(10);
        digitalWrite(_rst, LOW);
        delay(10);
        digitalWrite(_rst, HIGH);
        delay(120);  // Wait for reset to complete
    }
}

void ST7305_Mono::initDisplay() {
    // ST7305 initialization sequence based on datasheet
    
    // NVM Load Control
    sendCommand(ST7305_NVMLOADCTRL);
    sendData(0x17);
    sendData(0x02);
    
    // Booster Enable
    sendCommand(ST7305_BSTEN);
    
    // Gate Voltage Control: VGL=-10V, VGH=15V
    sendCommand(ST7305_GCTRL);
    sendData(0x0A);  // VGL
    sendData(0x01);
    sendData(0x0E);  // VGH
    
    // VSHP Setting (Source High Positive): VSHP=5V
    sendCommand(ST7305_VSHPCTRL);
    sendData(0x41);  // VSHP1
    sendData(0x41);  // VSHP2
    sendData(0x41);  // VSHP3
    sendData(0x41);  // VSHP4
    
    // VSLP Setting (Source Low Positive): VSLP=1V
    sendCommand(ST7305_VSLPCTRL);
    sendData(0x32);  // VSLP1
    sendData(0x32);  // VSLP2
    sendData(0x32);  // VSLP3
    sendData(0x32);  // VSLP4
    
    // VSHN Setting (Source High Negative): VSHN=-3.9V
    sendCommand(ST7305_VSHNCTRL);
    sendData(0x46);  // VSHN1
    sendData(0x46);  // VSHN2
    sendData(0x46);  // VSHN3
    sendData(0x46);  // VSHN4
    
    // VSLN Setting (Source Low Negative): VSLN=-0.4V
    sendCommand(ST7305_VSLNCTRL);
    sendData(0x46);  // VSLN1
    sendData(0x46);  // VSLN2
    sendData(0x46);  // VSLN3
    sendData(0x46);  // VSLN4
    
    // Frame Rate Control: HPM=32Hz, LPM=1Hz
    sendCommand(ST7305_FRCTRL);
    sendData(0x12);
    
    // Update Period Gate EQ Control in HPM
    sendCommand(ST7305_GTUPEQH);
    sendData(0xE5);
    sendData(0xF6);
    sendData(0x05);
    sendData(0x46);
    sendData(0x77);
    sendData(0x77);
    sendData(0x77);
    sendData(0x77);
    sendData(0x76);
    sendData(0x45);
    
    // Update Period Gate EQ Control in LPM
    sendCommand(ST7305_GTUPEQL);
    sendData(0x05);
    sendData(0x46);
    sendData(0x77);
    sendData(0x77);
    sendData(0x77);
    sendData(0x77);
    sendData(0x76);
    sendData(0x45);
    
    // Source EQ Enable
    sendCommand(ST7305_SOUEQ);
    
    // Gate Line Setting
    // Adjust based on your display: 
    // For 400 lines: may need different values
    // Datasheet example shows 0x13, 0x50 for 320 lines
    sendCommand(ST7305_GATESET);
    sendData(0x19);  // Adjusted for 400 lines (you may need to tune this)
    sendData(0x00);
    
    // Sleep Out
    sendCommand(ST7305_SLPOUT);
    delay(120);  // Must wait after sleep out
    
    // OSC Setting: 51Hz
    sendCommand(ST7305_OSCSET);
    sendData(0x80);
    
    // Source Voltage Select: VSHP1, VSLP1, VSHN1, VSLN1
    sendCommand(ST7305_VSHLSEL);
    sendData(0xE9);
    sendData(0x00);
    
    // Memory Data Access Control
    sendCommand(ST7305_MADCTL);
    sendData(0x00);  // Default orientation
    
    // Data Format Select (Mono mode)
    sendCommand(ST7305_DTFORM);
    sendData(0x10);  // 1-bit monochrome
    
    // Gamma Mode Setting: Mono, 2-line interlace
    sendCommand(ST7305_GAMAMS);
    sendData(0x48);  // Mono mode
    sendData(0x10);  // 2-line interlace
    sendData(0x20);  // 2-line panel layout
    
    // Panel Setting
    sendCommand(ST7305_PNLSET);
    
    // Column Address Set (0 to WIDTH-1)
    sendCommand(ST7305_CASET);
    sendData(0x00);
    sendData(0x00);
    sendData((ST7305_WIDTH - 1) >> 8);
    sendData((ST7305_WIDTH - 1) & 0xFF);
    
    // Row Address Set (0 to HEIGHT-1)
    sendCommand(ST7305_RASET);
    sendData(0x00);
    sendData(0x00);
    sendData((ST7305_HEIGHT - 1) >> 8);
    sendData((ST7305_HEIGHT - 1) & 0xFF);
    
    // Tearing Effect Line On
    sendCommand(ST7305_TEON);
    sendData(0x00);
    
    // Enable Auto Power Down
    sendCommand(ST7305_AUTOPWRCTRL);
    sendData(0x00);
    sendData(0xFF);
    
    // Start in Low Power Mode (LPM)
    sendCommand(ST7305_LPM);
    
    // Display On
    sendCommand(ST7305_DISPON);
    
    delay(10);
}

void ST7305_Mono::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if ((x < 0) || (x >= ST7305_WIDTH) || (y < 0) || (y >= ST7305_HEIGHT)) {
        return;
    }
    
    // Calculate byte position and bit position
    uint32_t byteIndex = (y * ST7305_WIDTH + x) / 8;
    uint8_t bitMask = 0x80 >> (x & 7);
    
    if (color) {
        // Set pixel (white)
        buffer[byteIndex] |= bitMask;
    } else {
        // Clear pixel (black)
        buffer[byteIndex] &= ~bitMask;
    }
}

void ST7305_Mono::display() {
    // Set address window to full screen
    setAddressWindow(0, 0, ST7305_WIDTH - 1, ST7305_HEIGHT - 1);
    
    // Write display buffer
    sendCommand(ST7305_RAMWR);
    sendDataBatch(buffer, ST7305_BUFFER_SIZE);
}

void ST7305_Mono::clearDisplay() {
    memset(buffer, 0xFF, ST7305_BUFFER_SIZE);  // White
}

void ST7305_Mono::invertDisplay(bool invert) {
    sendCommand(invert ? ST7305_INVON : ST7305_INVOFF);
}

void ST7305_Mono::setContrast(uint8_t contrast) {
    // ST7305 doesn't have a simple contrast command
    // Contrast would need to be adjusted through gamma/voltage settings
    // This is a placeholder - implement if needed with specific voltage adjustments
}

void ST7305_Mono::setHighPowerMode() {
    sendCommand(ST7305_HPM);
}

void ST7305_Mono::setLowPowerMode() {
    sendCommand(ST7305_LPM);
}

void ST7305_Mono::setAddressWindow(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    // Column address set
    sendCommand(ST7305_CASET);
    sendData(x0 >> 8);
    sendData(x0 & 0xFF);
    sendData(x1 >> 8);
    sendData(x1 & 0xFF);
    
    // Row address set
    sendCommand(ST7305_RASET);
    sendData(y0 >> 8);
    sendData(y0 & 0xFF);
    sendData(y1 >> 8);
    sendData(y1 & 0xFF);
}

void ST7305_Mono::sendCommand(uint8_t cmd) {
    dcLow();
    csLow();
    SPI.beginTransaction(spiSettings);
    SPI.transfer(cmd);
    SPI.endTransaction();
    csHigh();
}

void ST7305_Mono::sendData(uint8_t data) {
    dcHigh();
    csLow();
    SPI.beginTransaction(spiSettings);
    SPI.transfer(data);
    SPI.endTransaction();
    csHigh();
}

void ST7305_Mono::sendDataBatch(const uint8_t *data, uint32_t size) {
    dcHigh();
    csLow();
    SPI.beginTransaction(spiSettings);
    
    // Transfer in chunks for efficiency
    const uint32_t chunkSize = 256;
    for (uint32_t i = 0; i < size; i += chunkSize) {
        uint32_t len = (size - i) > chunkSize ? chunkSize : (size - i);
        SPI.transfer((void*)(data + i), len);
    }
    
    SPI.endTransaction();
    csHigh();
}

void ST7305_Mono::csLow() {
    digitalWrite(_cs, LOW);
}

void ST7305_Mono::csHigh() {
    digitalWrite(_cs, HIGH);
}

void ST7305_Mono::dcLow() {
    digitalWrite(_dc, LOW);
}

void ST7305_Mono::dcHigh() {
    digitalWrite(_dc, HIGH);
}
