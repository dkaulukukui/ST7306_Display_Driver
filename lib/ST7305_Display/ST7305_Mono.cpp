/**
 * ST7305_Mono.cpp
 * 
 * ST7305 Monochrome Display Driver Implementation
 * 
 * This driver supports ST7305-based displays with:
 * - 300×400 pixel resolution
 * - 4 pixels per byte horizontal layout
 * - 2 rows per vertical group layout
 * - 15,000 byte frame buffer
 * 
 * Memory Layout:
 *   Buffer size = (WIDTH/4) × (HEIGHT/2) = 75 × 200 = 15,000 bytes
 *   Each byte contains 4 horizontal pixels across 2 vertical rows
 * 
 * Author: Based on FT_tele_ST7305 reference implementation
 */

#include "ST7305_Mono.h"

/**
 * Constructor - Initialize display driver
 * 
 * @param dc  Data/Command pin (selects between command and data mode)
 * @param rst Reset pin (hardware reset, -1 if not used)
 * @param cs  Chip Select pin (SPI device selection)
 */
ST7305_Mono::ST7305_Mono(int8_t dc, int8_t rst, int8_t cs)
    : Adafruit_GFX(ST7305_WIDTH, ST7305_HEIGHT),
      _dc(dc), _rst(rst), _cs(cs), buffer(nullptr) {
}

/**
 * Destructor - Free allocated frame buffer
 */
ST7305_Mono::~ST7305_Mono() {
    if (buffer) {
        free(buffer);
        buffer = nullptr;
    }
}

/**
 * Initialize display - Configure hardware and allocate buffer
 * 
 * This function:
 * 1. Configures GPIO pins for SPI communication
 * 2. Initializes SPI bus with specified frequency
 * 3. Allocates 15KB frame buffer
 * 4. Performs hardware reset
 * 5. Sends initialization command sequence
 * 
 * @param spiFrequency SPI clock frequency (default 1MHz, max 40MHz tested)
 * @param initCmds     Pointer to init command array (defaults to ACTIVE_INIT_CMDS)
 * @param cmdCount     Number of commands in array
 * @return true if initialization successful, false if buffer allocation fails
 */
bool ST7305_Mono::begin(uint32_t spiFrequency, const st7305_lcd_init_cmd_t* initCmds, size_t cmdCount) {
    // Configure pins
    pinMode(_dc, OUTPUT);
    pinMode(_cs, OUTPUT);
    if (_rst >= 0) {
        pinMode(_rst, OUTPUT);
    }
    
    // Initialize SPI with specified frequency and mode
    // Mode 0: CPOL=0, CPHA=0, MSBFIRST
    SPI.begin();
    spiSettings = SPISettings(spiFrequency, MSBFIRST, SPI_MODE0);
    
    // Allocate frame buffer: (300/4) × (400/2) = 15,000 bytes
    buffer = (uint8_t*)malloc(ST7305_BUFFER_SIZE);
    if (!buffer) {
        return false;  // Out of memory
    }
    
    // Clear buffer to white (0xFF for white background)
    memset(buffer, 0xFF, ST7305_BUFFER_SIZE);
    
    // Perform hardware reset and send init commands
    hardwareReset();
    initDisplay(initCmds, cmdCount);
    
    return true;
}

/**
 * Hardware Reset - Perform display reset sequence
 * 
 * Timing sequence:
 * 1. RST HIGH for 10ms (stable state)
 * 2. RST LOW for 10ms (reset active)
 * 3. RST HIGH for 120ms (recovery time)
 * 
 * Total reset time: ~140ms
 */
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

/**
 * Initialize Display - Send initialization command sequence
 * 
 * Iterates through the selected init command array (configured via ACTIVE_INIT_CMDS).
 * Each command structure contains:
 * - cmd: Command byte to send
 * - data[]: Array of up to 10 data bytes
 * - len: Number of data bytes to send
 * - delay_ms: Milliseconds to delay after command
 * 
 * Available configurations:
 * - st7305_init_cmds_default: Standard initialization
 * - st7305_init_cmds_FT_tele: FT_tele reference implementation
 * - st7305_init_cmds_kevin: Custom configuration
 * - st7305_init_cmds_mfg: Manufacturer settings
 * 
 * @param st7305_init_cmds Pointer to init command array
 * @param cmd_count        Number of commands in array
 */
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

/**
 * Draw Pixel - Set or clear a single pixel in the frame buffer
 * 
 * ST7305 Memory Layout:
 * - 4 pixels per byte horizontally (300 ÷ 4 = 75 bytes per row)
 * - 2 rows per vertical group (400 ÷ 2 = 200 row-pairs)
 * - Total buffer: 75 × 200 = 15,000 bytes
 * 
 * Pixel Mapping Algorithm:
 * 1. real_x = x / 4        (which byte horizontally, 0-74)
 * 2. real_y = y / 2        (which row-pair, 0-199)
 * 3. line_bit_4 = x % 4    (which of 4 pixels in byte, 0-3)
 * 4. one_two = y % 2       (which of 2 rows in pair, 0-1)
 * 5. write_bit = 7 - (line_bit_4 * 2 + one_two)  (final bit position, 0-7)
 * 
 * Byte Index Formula:
 *   write_byte_index = real_y * LCD_DATA_WIDTH + real_x
 *                    = (y/2) * 75 + (x/4)
 * 
 * Bit Mapping Examples:
 *   Pixel (0,0): byte[0] bit 7    Pixel (1,0): byte[0] bit 5
 *   Pixel (0,1): byte[0] bit 6    Pixel (1,1): byte[0] bit 4
 *   Pixel (2,0): byte[0] bit 3    Pixel (3,0): byte[0] bit 1
 *   Pixel (2,1): byte[0] bit 2    Pixel (3,1): byte[0] bit 0
 *   Pixel (4,0): byte[1] bit 7    ...and so on
 * 
 * Single Byte Layout (bits 7-0):
 *   [x+0,y+0] [x+0,y+1] [x+1,y+0] [x+1,y+1] [x+2,y+0] [x+2,y+1] [x+3,y+0] [x+3,y+1]
 *   
 * @param x     X coordinate (0-299)
 * @param y     Y coordinate (0-399)
 * @param color 0=BLACK, 1=WHITE
 */
void ST7305_Mono::drawPixel(int16_t x, int16_t y, uint16_t color) {
    if ((x < 0) || (x >= ST7305_WIDTH) || (y < 0) || (y >= ST7305_HEIGHT)) {
        return;
    }
    
    // ST7305 memory layout matching reference code:
    // 4 pixels horizontally, 2 rows vertically per byte
    
    uint32_t real_x = x / 4;  // 0->0, 3->0, 4->1, 7->1
    uint32_t real_y = y / 2;  // 0->0, 1->0, 2->1, 3->1
    uint32_t LCD_DATA_WIDTH = ST7305_WIDTH / 4;  // 75 bytes per row
    uint32_t write_byte_index = real_y * LCD_DATA_WIDTH + real_x;
    uint32_t one_two = (y % 2 == 0) ? 0 : 1;  // 0 for even rows, 1 for odd rows
    uint32_t line_bit_4 = x % 4;  // 0, 1, 2, or 3 (which of 4 pixels)
    uint8_t write_bit = 7 - (line_bit_4 * 2 + one_two);  // Calculate final bit position
    
    if (color) {
        // Set pixel (white) - set bit to 1
        buffer[write_byte_index] |= (1 << write_bit);
    } else {
        // Clear pixel (black) - set bit to 0
        buffer[write_byte_index] &= ~(1 << write_bit);
    }
}

/**
 * Display - Transfer frame buffer to display hardware
 * 
 * This function sends the entire 15KB frame buffer to the display using SPI.
 * 
 * Sequence:
 * 1. Set column address range (0x2A): 0x12 to 0x2A
 * 2. Set row address range (0x2B): 0x00 to 0xC7
 * 3. Send memory write command (0x2C)
 * 4. Transfer all 15,000 bytes via SPI in one transaction
 * 
 * Address ranges match reference implementation:
 * - Column: 0x12 (18) to 0x2A (42)
 * - Row: 0x00 (0) to 0xC7 (199)
 * 
 * Performance:
 * - @ 1MHz: ~300ms for full screen update
 * - @ 40MHz: ~100ms for full screen update
 * 
 * Note: This transfers the entire buffer. For partial updates,
 * implement custom address windowing.
 */
void ST7305_Mono::display() {
    // Set column address range
    sendCommand(ST7305_CASET);
    sendData(0x12);  // Start column
    sendData(0x2A);  // End column
    
    // Set row address range
    sendCommand(ST7305_RASET);
    sendData(0x00);  // Start row
    sendData(0xC7);  // End row
    
    // Write RAM command
    sendCommand(ST7305_RAMWR);
    
    // Send entire buffer in one SPI transaction for efficiency
    digitalWrite(_dc, HIGH);  // Data mode
    digitalWrite(_cs, LOW);   // Select display
    SPI.beginTransaction(spiSettings);
    SPI.transfer(buffer, ST7305_BUFFER_SIZE);  // Send all 15,000 bytes
    SPI.endTransaction();
    digitalWrite(_cs, HIGH);  // Deselect display
}

/**
 * Clear Display - Fill frame buffer with black
 * 
 * Sets all buffer bytes to 0x00 (black pixels).
 * Call display() afterwards to update the screen.
 */
void ST7305_Mono::clearDisplay() {
    memset(buffer, 0x00, ST7305_BUFFER_SIZE);  // Clear to black (0x00)
}

/**
 * Fill - Fill frame buffer with specified byte value
 * 
 * Useful for:
 * - 0x00: All black
 * - 0xFF: All white
 * - 0xAA/0x55: Checkerboard patterns
 * 
 * @param data Byte value to fill buffer with
 */
void ST7305_Mono::fill(uint8_t data) {
    memset(buffer, data, ST7305_BUFFER_SIZE);
}

/**
 * Invert Display - Hardware pixel inversion
 * 
 * Inverts all pixels on the display:
 * - Black becomes white
 * - White becomes black
 * 
 * This is a hardware feature and doesn't modify the frame buffer.
 * 
 * @param invert true to invert, false for normal display
 */
void ST7305_Mono::invertDisplay(bool invert) {
    sendCommand(invert ? ST7305_INVON : ST7305_INVOFF);
}

/**
 * Set Contrast - Adjust display contrast (placeholder)
 * 
 * ST7305 doesn't have a simple contrast command.
 * Contrast adjustment requires modifying voltage/gamma settings.
 * Implement this by adjusting specific voltage commands in init sequence.
 * 
 * @param contrast Contrast value (not currently used)
 */
void ST7305_Mono::setContrast(uint8_t contrast) {
    // ST7305 doesn't have a simple contrast command
    // Contrast would need to be adjusted through gamma/voltage settings
    // This is a placeholder - implement if needed with specific voltage adjustments
}

/**
 * Set High Power Mode - Enter high power mode
 * 
 * Sends 0xB9 command for high power operation.
 */
void ST7305_Mono::setHighPowerMode() {
    sendCommand(ST7305_HPM);
}

/**
 * Set Low Power Mode - Enter low power mode
 * 
 * Sends 0xBA command for low power operation.
 */
void ST7305_Mono::setLowPowerMode() {
    sendCommand(ST7305_LPM);
}

/**
 * Set Address Window - Define rectangular update region
 * 
 * Sets the active drawing window for partial updates.
 * Subsequent memory writes will only affect this region.
 * 
 * @param x0 Start column (0-299)
 * @param y0 Start row (0-399)
 * @param x1 End column (0-299)
 * @param y1 End row (0-399)
 */
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

// ===== Low-Level SPI Communication Functions =====

/**
 * Send Command - Send single command byte
 * 
 * @param cmd Command byte to send
 */
void ST7305_Mono::sendCommand(uint8_t cmd) {
    dcLow();
    csLow();
    SPI.beginTransaction(spiSettings);
    SPI.transfer(cmd);
    SPI.endTransaction();
    csHigh();
}

/**
 * Send Data - Send single data byte
 * 
 * @param data Data byte to send
 */
void ST7305_Mono::sendData(uint8_t data) {
    dcHigh();
    csLow();
    SPI.beginTransaction(spiSettings);
    SPI.transfer(data);
    SPI.endTransaction();
    csHigh();
}

/**
 * Send Data Batch - Send multiple data bytes efficiently
 * 
 * Transfers data in 256-byte chunks for efficiency.
 * 
 * @param data Pointer to data array
 * @param size Number of bytes to send
 */
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

// ===== GPIO Helper Functions =====

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
