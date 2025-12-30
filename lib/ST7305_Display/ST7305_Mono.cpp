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

bool ST7305_Mono::begin(uint32_t spiFrequency, const st7305_lcd_init_cmd_t* initCmds, size_t cmdCount) {
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
    
    // Clear buffer to white (0xFF for white background)
    memset(buffer, 0xFF, ST7305_BUFFER_SIZE);  // Start with white
    
    // Hardware reset and initialize
    hardwareReset();
    initDisplay(initCmds, cmdCount);
    
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

void ST7305_Mono::initDisplay(const st7305_lcd_init_cmd_t* st7305_init_cmds, size_t cmd_count) {
    
    for (size_t i = 0; i < cmd_count; i++) {
        sendCommand(st7305_init_cmds[i].cmd);
        for (uint8_t j = 0; j < st7305_init_cmds[i].len; j++) {
            sendData(st7305_init_cmds[i].data[j]);
        }
        if (st7305_init_cmds[i].delay_ms > 0) {
            delay(st7305_init_cmds[i].delay_ms);
        }
    }
}

void ST7305_Mono::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if ((x < 0) || (x >= ST7305_WIDTH) || (y < 0) || (y >= ST7305_HEIGHT)) {
        return;
    }
    
    // ST7305 memory layout matching reference code:
    // 4 pixels horizontally, 2 rows vertically per byte
    // Bit mapping: [x%4=0,y%2=0]->bit7, [x%4=1,y%2=0]->bit5, [x%4=2,y%2=0]->bit3, [x%4=3,y%2=0]->bit1
    //              [x%4=0,y%2=1]->bit6, [x%4=1,y%2=1]->bit4, [x%4=2,y%2=1]->bit2, [x%4=3,y%2=1]->bit0
    
    uint32_t real_x = x / 4;  // 0->0, 3->0, 4->1, 7->1
    uint32_t real_y = y / 2;  // 0->0, 1->0, 2->1, 3->1
    uint32_t LCD_DATA_WIDTH = ST7305_WIDTH / 4;  // 75 bytes per row
    uint32_t write_byte_index = real_y * LCD_DATA_WIDTH + real_x;
    uint32_t one_two = (y % 2 == 0) ? 0 : 1;  // 0 or 1
    uint32_t line_bit_4 = x % 4;  // 0, 1, 2, or 3
    uint8_t write_bit = 7 - (line_bit_4 * 2 + one_two);
    
    if (color) {
        // Set pixel (white)
        buffer[write_byte_index] |= (1 << write_bit);
    } else {
        // Clear pixel (black)
        buffer[write_byte_index] &= ~(1 << write_bit);
    }
}

void ST7305_Mono::display() {
    // Set column address
    sendCommand(ST7305_CASET);
    sendData(0x12);
    sendData(0x2A);
    
    // Set row address
    sendCommand(ST7305_RASET);
    sendData(0x00);
    sendData(0xC7);
    
    // Write RAM
    sendCommand(ST7305_RAMWR);
    
    // Send buffer data
    digitalWrite(_dc, HIGH);
    digitalWrite(_cs, LOW);
    SPI.beginTransaction(spiSettings);
    SPI.transfer(buffer, ST7305_BUFFER_SIZE);
    SPI.endTransaction();
    digitalWrite(_cs, HIGH);
}

void ST7305_Mono::clearDisplay() {
    memset(buffer, 0x00, ST7305_BUFFER_SIZE);  // Clear to black (0x00)
}

void ST7305_Mono::fill(uint8_t data) {
    memset(buffer, data, ST7305_BUFFER_SIZE);
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
