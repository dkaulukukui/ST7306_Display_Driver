#include "ST7306_Mono.h"

// Constructor
ST7306_Mono::ST7306_Mono(int8_t dc, int8_t rst, int8_t cs)
    : Adafruit_GFX(ST7306_WIDTH, ST7306_HEIGHT), _dc(dc), _rst(rst), _cs(cs), buffer(nullptr)
{
}

// Destructor
ST7306_Mono::~ST7306_Mono()
{
    if (buffer) {
        free(buffer);
        buffer = nullptr;
    }
}

// Initialize display
bool ST7306_Mono::begin(uint32_t spiFrequency)
{
    // Allocate frame buffer
    buffer = (uint8_t *)malloc(ST7306_BUFFER_SIZE);
    if (!buffer) {
        return false;
    }
    memset(buffer, 0xFF, ST7306_BUFFER_SIZE); // Start with white screen
    
    // Initialize SPI
    SPI.begin();
    spiSettings = SPISettings(spiFrequency, MSBFIRST, SPI_MODE0);
    
    // Setup GPIO pins
    pinMode(_dc, OUTPUT);
    pinMode(_cs, OUTPUT);
    if (_rst >= 0) {
        pinMode(_rst, OUTPUT);
    }
    
    csHigh();
    
    // Hardware reset
    hardwareReset();
    
    // Initialize display
    initDisplay();
    
    // Clear display
    clearDisplay();
    display();
    
    return true;
}

// Hardware reset
void ST7306_Mono::hardwareReset()
{
    if (_rst >= 0) {
        digitalWrite(_rst, HIGH);
        delay(10);
        digitalWrite(_rst, LOW);
        delay(50);
        digitalWrite(_rst, HIGH);
        delay(120);
    }
}

// Initialize ST7306 for monochrome mode
void ST7306_Mono::initDisplay()
{
    // ========================================
    // OTP (One-Time Programmable) Load Control
    // ========================================
    sendCommand(0xD6);  // OTP Load Control
    sendData(0x17);     // Bit[7:5]: Load mode, Bit[4:0]: OTP address
                        // 0x17 = Load from address 0x17
                        // Options: 0x00-0x1F for different OTP addresses
    sendData(0x02);     // OTP data control
                        // 0x02 = Standard load timing
                        // Options: 0x00-0x0F for different timing parameters
    
    // ========================================
    // Auto Power Control
    // ========================================
    sendCommand(0xD1);  // Auto Power Control
    sendData(0x01);     // Bit[0]: Enable auto power control (1=Enable, 0=Disable)
                        // 0x01 = Enable automatic power sequencing
                        // Options: 0x00 (Manual), 0x01 (Auto)
    
    // ========================================
    // Gate Voltage Setting (VGH/VGL)
    // ========================================
    sendCommand(0xC0);  // Gate Voltage Setting
    sendData(0x0E);     // VGH voltage level (Gate High Voltage)
                        // Formula: VGH = 2.4V + (value × 0.2V)
                        // 0x0E = 2.4 + (14 × 0.2) = 5.2V
                        // Range: 0x00-0x0F (2.4V to 5.4V in 0.2V steps)
    sendData(0x0A);     // VGL voltage level (Gate Low Voltage)
                        // Formula: VGL = -2.4V - (value × 0.2V)
                        // 0x0A = -2.4 - (10 × 0.2) = -4.4V
                        // Range: 0x00-0x0F (-2.4V to -5.4V in -0.2V steps)
    
    // ========================================
    // VSH Setting (Source High Voltage)
    // ========================================
    sendCommand(0xC1);  // VSH Setting (4 parameters for different phases)
    sendData(0x41);     // VSH1: First phase source high voltage
                        // Bit[7:6]: Multiplier, Bit[5:0]: Voltage level
                        // 0x41 = 01[multiplier] 000001[level]
                        // Formula: VSH = VCI × multiplier + offset
                        // Range: 0x00-0x7F
    sendData(0x41);     // VSH2: Second phase source high voltage
                        // Same format as VSH1
    sendData(0x41);     // VSH3: Third phase source high voltage
                        // Same format as VSH1
    sendData(0x41);     // VSH4: Fourth phase source high voltage
                        // Same format as VSH1
                        // Typical values: 0x30-0x50
    
    // ========================================
    // VSL Setting (Source Low Voltage)
    // ========================================
    sendCommand(0xC2);  // VSL Setting (4 parameters for different phases)
    sendData(0x32);     // VSL1: First phase source low voltage
                        // Bit[7:6]: Multiplier, Bit[5:0]: Voltage level
                        // 0x32 = 00[multiplier] 110010[level]
                        // Formula: VSL = VCI × multiplier - offset
                        // Range: 0x00-0x7F
    sendData(0x32);     // VSL2: Second phase source low voltage
                        // Same format as VSL1
    sendData(0x32);     // VSL3: Third phase source low voltage
                        // Same format as VSL1
    sendData(0x32);     // VSL4: Fourth phase source low voltage
                        // Same format as VSL1
                        // Typical values: 0x20-0x40
    
    // ========================================
    // VCOMH Setting (Common Voltage High)
    // ========================================
    sendCommand(0xC4);  // VCOMH Setting (4 parameters for adjustment)
    sendData(0x46);     // VCOMH1: First common high voltage parameter
                        // Controls contrast and display quality
                        // 0x46 = Moderate positive common voltage
                        // Range: 0x00-0x7F (adjust for optimal contrast)
    sendData(0x46);     // VCOMH2: Second common high voltage parameter
                        // Fine-tune parameter
    sendData(0x46);     // VCOMH3: Third common high voltage parameter
                        // Fine-tune parameter
    sendData(0x46);     // VCOMH4: Fourth common high voltage parameter
                        // Fine-tune parameter
                        // Typical values: 0x40-0x50 (higher = more positive)
    
    // ========================================
    // VCOML Setting (Common Voltage Low)
    // ========================================
    sendCommand(0xC5);  // VCOML Setting (4 parameters for adjustment)
    sendData(0x46);     // VCOML1: First common low voltage parameter
                        // Controls contrast and display quality
                        // 0x46 = Moderate negative common voltage
                        // Range: 0x00-0x7F (adjust for optimal contrast)
    sendData(0x46);     // VCOML2: Second common low voltage parameter
                        // Fine-tune parameter
    sendData(0x46);     // VCOML3: Third common low voltage parameter
                        // Fine-tune parameter
    sendData(0x46);     // VCOML4: Fourth common low voltage parameter
                        // Fine-tune parameter
                        // Typical values: 0x40-0x50 (balance with VCOMH)
    
    // ========================================
    // Gate EQ (Equalization) Setting
    // ========================================
    sendCommand(0xB2);  // Gate EQ Setting
    sendData(0x12);     // Gate signal equalization control
                        // Bit[7:4]: Pre-charge timing
                        // Bit[3:0]: Discharge timing
                        // 0x12 = Balanced pre-charge and discharge
                        // Range: 0x00-0xFF
                        // Lower values: Faster switching, may cause ghosting
                        // Higher values: Slower switching, cleaner display
                        // Typical values: 0x10-0x20
    
    // ========================================
    // Source EQ (Equalization) Setting
    // ========================================
    sendCommand(0xB3);  // Source EQ Setting (10 parameters for different segments)
    sendData(0xE5);     // SEQ1: Source equalization for segment 1
                        // Controls source driver output timing
                        // 0xE5 = High equalization strength
                        // Range: 0x00-0xFF (higher = stronger equalization)
    sendData(0xF6);     // SEQ2: Source equalization for segment 2
                        // 0xF6 = Very high equalization
    sendData(0x05);     // SEQ3: Source equalization for segment 3
                        // 0x05 = Low equalization (edge segments)
    sendData(0x46);     // SEQ4: Source equalization for segment 4
                        // 0x46 = Medium equalization
    sendData(0x77);     // SEQ5: Source equalization for segment 5
                        // 0x77 = High equalization (center segments)
    sendData(0x77);     // SEQ6: Source equalization for segment 6
                        // 0x77 = High equalization
    sendData(0x77);     // SEQ7: Source equalization for segment 7
                        // 0x77 = High equalization
    sendData(0x77);     // SEQ8: Source equalization for segment 8
                        // 0x77 = High equalization
    sendData(0x76);     // SEQ9: Source equalization for segment 9
                        // 0x76 = High equalization
    sendData(0x45);     // SEQ10: Source equalization for segment 10
                        // 0x45 = Medium equalization (edge segments)
                        // Pattern: Higher values in center, lower at edges
                        // Ensures uniform display across all columns
    
    // ========================================
    // Source EQ Second Bank
    // ========================================
    sendCommand(0xB4);  // Source EQ Setting Bank 2 (10 more parameters)
    sendData(0x05);     // SEQ11: Source equalization for segment 11
                        // Mirrors 0xB3 pattern for second half of display
    sendData(0x46);     // SEQ12: Source equalization for segment 12
    sendData(0x77);     // SEQ13: Source equalization for segment 13
    sendData(0x77);     // SEQ14: Source equalization for segment 14
    sendData(0x77);     // SEQ15: Source equalization for segment 15
    sendData(0x77);     // SEQ16: Source equalization for segment 16
    sendData(0x76);     // SEQ17: Source equalization for segment 17
    sendData(0x45);     // SEQ18: Source equalization for segment 18
                        // Combined with 0xB3, provides equalization for all 300 columns
    
    // ========================================
    // OSC (Oscillator) Setting
    // ========================================
    sendCommand(0xB7);  // OSC Setting
    sendData(0x13);     // Internal oscillator frequency control
                        // Bit[7:4]: Reserved
                        // Bit[3:0]: Frequency adjustment
                        // 0x13 = Standard frequency (default)
                        // Formula: Freq = Base × (1 + value/16)
                        // Range: 0x00-0x1F
                        // 0x00: Slowest (lower power, slower refresh)
                        // 0x1F: Fastest (higher power, faster refresh)
                        // Typical values: 0x10-0x18
    
    // ========================================
    // Duty Setting (Display Lines)
    // ========================================
    sendCommand(0xB0);  // Duty Setting
    sendData(0x78);     // Number of display lines
                        // Formula: Lines = (value + 1) × 2 for 400-line display
                        // 0x78 = 120 decimal → (120 + 1) × 2 = 242 lines
                        // NOTE: May need adjustment for 400 lines
                        // For 400 lines: value = (400/2) - 1 = 199 = 0xC7
                        // Range: 0x00-0xFF
                        // 0x27 = 80 lines, 0x4F = 160 lines
                        // 0x77 = 240 lines, 0xC7 = 400 lines
    
    // ========================================
    // Sleep Out (Wake Display)
    // ========================================
    sendCommand(0x11);  // Sleep Out
    delay(120);         // Required 120ms delay after sleep out
                        // Display needs time to stabilize power
    
    // ========================================
    // Monochrome Mode Configuration
    // ========================================
    sendCommand(0xD8);  // Monochrome Mode Control
    sendData(0x80);     // Enable monochrome display mode
                        // Bit[7]: Monochrome enable (1=Enable, 0=Disable)
                        // Bit[6:0]: Reserved
                        // 0x80 = 10000000b = Monochrome mode ON
                        // Options: 0x00 (8-color mode), 0x80 (Monochrome mode)
    sendData(0xE9);     // Monochrome mode parameters
                        // Bit[7:4]: Refresh rate control
                        // Bit[3:0]: Contrast adjustment
                        // 0xE9 = High refresh, high contrast
                        // Range: 0x00-0xFF (adjust for display quality)
                        // Lower values: Lower contrast, slower
                        // Higher values: Higher contrast, faster
    
    // ========================================
    // External Resistor Configuration
    // ========================================
    sendCommand(0xC9);  // External Resistor Control
    sendData(0x00);     // External resistor mode selection
                        // Bit[1:0]: Resistor mode
                        // 0x00 = Internal resistors (default)
                        // Options: 
                        //   0x00: All internal resistors
                        //   0x01: External VCOMH resistor
                        //   0x02: External VCOML resistor
                        //   0x03: Both external resistors
                        // Use 0x00 unless external resistors are connected
    
    // ========================================
    // Memory Data Access Control (Rotation)
    // ========================================
    sendCommand(0x36);  // Memory Data Access Control (MADCTL)
    sendData(0x48);     // Display orientation and mirror control
                        // Bit[7] MY: Row address order (0=Top→Bottom, 1=Bottom→Top)
                        // Bit[6] MX: Column address order (0=Left→Right, 1=Right→Left)
                        // Bit[5] MV: Row/Column exchange (0=Normal, 1=Swapped)
                        // Bit[4] ML: Vertical refresh order (0=Top→Bottom, 1=Bottom→Top)
                        // Bit[3] BGR: Color order (not used in monochrome)
                        // Bit[2] MH: Horizontal refresh order
                        // 0x48 = 01001000b = MY=0, MX=1, MV=0, ML=0
                        // Common orientations:
                        //   0x00: Normal
                        //   0x40: Horizontal flip (MX=1)
                        //   0x80: Vertical flip (MY=1)
                        //   0xC0: 180° rotation (MY=1, MX=1)
                        //   0x20: 90° clockwise (MV=1)
                        //   0x60: 90° clockwise + flip (MV=1, MX=1)
                        //   0xA0: 270° clockwise (MY=1, MV=1)
                        //   0xE0: 270° clockwise + flip (MY=1, MX=1, MV=1)
    
    // ========================================
    // Interface Pixel Format
    // ========================================
    sendCommand(0x3A);  // Interface Pixel Format (COLMOD)
    sendData(0x00);     // Pixel format selection
                        // Bit[6:4]: RGB interface format (not used)
                        // Bit[2:0]: MCU interface format
                        // 0x00 = 1-bit monochrome mode
                        // Options:
                        //   0x00: 1-bit (monochrome, 1bpp)
                        //   0x03: 8-bit (256 colors, 8bpp)
                        //   0x05: 16-bit RGB565 (65K colors, 16bpp)
                        //   0x06: 18-bit RGB666 (262K colors, 18bpp)
                        // Use 0x00 for monochrome display
    
    // ========================================
    // Source Output Setting
    // ========================================
    sendCommand(0xB9);  // Source Setting
    sendData(0x00);     // Source driver output control
                        // Bit[5]: Source output enable
                        // Bit[4:0]: Output level adjustment
                        // 0x00 = Standard source output
                        // 0x20 = Alternative monochrome setting (try if display issues)
                        // Range: 0x00-0x3F
                        // Adjust if display appears too bright or dim
    
    // ========================================
    // Panel Characteristics Setting
    // ========================================
    sendCommand(0xB8);  // Panel Setting
    sendData(0x0A);     // Panel-specific configuration
                        // Bit[7:4]: Panel type selection
                        // Bit[3:0]: Scan direction and mode
                        // 0x0A = Standard panel configuration
                        // Range: 0x00-0x0F
                        // Panel-specific, use 0x0A for LH420NB-F07
                        // Consult panel datasheet for alternatives
    
    // ========================================
    // Tearing Effect Signal
    // ========================================
    sendCommand(0x35);  // Tearing Effect Line ON
    sendData(0x00);     // TE signal mode
                        // 0x00 = V-Blanking signal only
                        // 0x01 = V-Blanking and H-Blanking signals
                        // Options:
                        //   0x00: Signal during vertical blanking (recommended)
                        //   0x01: Signal during vertical and horizontal blanking
                        // Use 0x00 for standard synchronization
    
    // ========================================
    // Auto Read Enable
    // ========================================
    sendCommand(0xD0);  // Enable Auto Read
    sendData(0xFF);     // Auto read GRAM (Graphics RAM) enable
                        // Bit[7:0]: Enable mask for different functions
                        // 0xFF = Enable all auto-read functions
                        // Enables automatic memory read-back
                        // Range: 0x00-0xFF (bitmask)
                        // 0x00: Disable auto-read
                        // 0xFF: Enable all auto-read features
    
    // ========================================
    // Idle Mode Control
    // ========================================
    sendCommand(0x38);  // Idle Mode OFF (Normal Display Mode)
                        // No parameters
                        // Exits idle mode (reduced color/power mode)
                        // Alternative: 0x39 (Idle Mode ON) for power saving
                        // Normal mode: Full color/contrast
                        // Idle mode: Reduced color/contrast, lower power
    
    // ========================================
    // Display ON
    // ========================================
    sendCommand(0x29);  // Display ON
                        // No parameters
                        // Turns on display output
                        // Alternative: 0x28 (Display OFF) to blank screen
    delay(10);          // Short delay for display stabilization
}

// Set address window for full screen
void ST7306_Mono::setAddressWindow()
{
    // Column address
    sendCommand(0x2A);
    sendData(0x00);
    sendData(0x00);
    sendData(ST7306_WIDTH >> 8);
    sendData(ST7306_WIDTH & 0xFF);
    
    // Row address
    sendCommand(0x2B);
    sendData(0x00);
    sendData(0x00);
    sendData(ST7306_HEIGHT >> 8);
    sendData(ST7306_HEIGHT & 0xFF);
}

// Send frame buffer to display
void ST7306_Mono::display()
{
    setAddressWindow();
    sendCommand(0x2C); // Memory Write
    
    // Send buffer in chunks
    const uint16_t chunkSize = 512;
    for (uint32_t i = 0; i < ST7306_BUFFER_SIZE; i += chunkSize) {
        uint16_t remaining = ST7306_BUFFER_SIZE - i;
        uint16_t toSend = (remaining < chunkSize) ? remaining : chunkSize;
        sendDataBatch(&buffer[i], toSend);
    }
}

// Clear display buffer
void ST7306_Mono::clearDisplay()
{
    memset(buffer, 0xFF, ST7306_BUFFER_SIZE); // 0xFF = white
}

// Invert display
void ST7306_Mono::invertDisplay(bool invert)
{
    sendCommand(invert ? 0x21 : 0x20); // INVON / INVOFF
}

// Set contrast
void ST7306_Mono::setContrast(uint8_t contrast)
{
    sendCommand(0xC0);
    sendData(contrast);
    sendData(contrast);
}

// Draw pixel (required by Adafruit_GFX)
void ST7306_Mono::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    if (x < 0 || x >= ST7306_WIDTH || y < 0 || y >= ST7306_HEIGHT) {
        return;
    }
    
    uint32_t byteIndex = (y * ST7306_WIDTH + x) / 8;
    uint8_t bitIndex = 7 - ((y * ST7306_WIDTH + x) % 8);
    
    if (color == ST7306_BLACK) {
        buffer[byteIndex] &= ~(1 << bitIndex); // Clear bit for black
    } else {
        buffer[byteIndex] |= (1 << bitIndex);  // Set bit for white
    }
}

// Low-level SPI functions
void ST7306_Mono::sendCommand(uint8_t cmd)
{
    dcLow();
    csLow();
    SPI.beginTransaction(spiSettings);
    SPI.transfer(cmd);
    SPI.endTransaction();
    csHigh();
}

void ST7306_Mono::sendData(uint8_t data)
{
    dcHigh();
    csLow();
    SPI.beginTransaction(spiSettings);
    SPI.transfer(data);
    SPI.endTransaction();
    csHigh();
}

void ST7306_Mono::sendDataBatch(const uint8_t *data, uint32_t size)
{
    dcHigh();
    csLow();
    SPI.beginTransaction(spiSettings);
    for (uint32_t i = 0; i < size; i++) {
        SPI.transfer(data[i]);
    }
    SPI.endTransaction();
    csHigh();
}

// GPIO helpers
void ST7306_Mono::csLow()
{
    digitalWrite(_cs, LOW);
}

void ST7306_Mono::csHigh()
{
    digitalWrite(_cs, HIGH);
}

void ST7306_Mono::dcLow()
{
    digitalWrite(_dc, LOW);
}

void ST7306_Mono::dcHigh()
{
    digitalWrite(_dc, HIGH);
}
