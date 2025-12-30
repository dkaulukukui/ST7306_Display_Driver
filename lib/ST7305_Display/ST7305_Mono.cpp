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
    
    // Clear buffer
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
    
    // ST7305 memory layout per datasheet:
    // 24 bits (3 bytes) map to 12×2 pixel block
    // Top row: odd bits P1, P3, P5... P23 (bits 23, 21, 19... 1)
    // Bottom row: even bits P2, P4, P6... P24 (bits 22, 20, 18... 0)
    
    uint32_t block_x = x / 12;          // Which 12-pixel-wide block horizontally
    uint32_t block_y = y / 2;           // Which 2-pixel-high block vertically
    uint32_t col_in_block = x % 12;     // Column within block (0-11)
    uint32_t row_in_block = y % 2;      // Row within block (0=top, 1=bottom)
    
    // Calculate bit position in 24-bit word (MSB=bit 23, LSB=bit 0)
    // Top row uses odd bits: 23, 21, 19... 1
    // Bottom row uses even bits: 22, 20, 18... 0
    uint8_t bit_pos = 23 - (col_in_block * 2) - row_in_block;
    
    // Calculate byte index
    // Each row has (WIDTH/12) blocks, each block is 3 bytes
    uint32_t bytes_per_row = (ST7305_WIDTH / 12) * 3;  // 25 * 3 = 75 bytes
    uint32_t base_byte = block_y * bytes_per_row + block_x * 3;
    
    // Determine which of the 3 bytes and which bit within that byte
    uint8_t byte_offset = bit_pos / 8;           // 0, 1, or 2
    uint8_t bit_in_byte = 7 - (bit_pos % 8);     // MSB-first within byte
    
    uint32_t byte_index = base_byte + byte_offset;
    
    if (color) {
        // Set pixel (white)
        buffer[byte_index] |= (1 << bit_in_byte);
    } else {
        // Clear pixel (black)
        buffer[byte_index] &= ~(1 << bit_in_byte);
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
