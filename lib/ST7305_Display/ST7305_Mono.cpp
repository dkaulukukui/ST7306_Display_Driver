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
    // ========================================================================
    // ST7305 Initialization Sequence
    // ========================================================================
    
    // ------------------------------------------------------------------------
    // NVM Load Control (0xD6)
    // Parameter 1 [7:0]: NVM load item selection
    //   Bit 0-6: Select which NVM parameters to load
    //   0x17 = Load multiple NVM parameters
    // Parameter 2 [7:0]: NVM load control settings
    //   Bit 0: Load by Timer (0=Disable, 1=Enable)
    //   Bit 1: Load by Sleep-out (0=Disable, 1=Enable)  
    //   0x02 = Enable load by Sleep-out
    // ------------------------------------------------------------------------
    sendCommand(ST7305_NVMLOADCTRL);
    sendData(0x17);  // Load NVM items
    sendData(0x00);  // Enable load on sleep-out
    
    // ------------------------------------------------------------------------
    // Booster Enable (0xD1)
    // Parameter 1 [0]: Booster circuit control
    //   0x00 = Disable booster (external VDDI supply)
    //   0x01 = Enable booster (internal charge pump)
    // ------------------------------------------------------------------------
    sendCommand(ST7305_BSTEN);
    sendData(0x01);  // Enable internal booster
    
    // ------------------------------------------------------------------------
    // Gate Voltage Control (0xC0)
    // Controls VGH (gate-on voltage) and VGL (gate-off voltage)
    // Parameter 1 [7:0]: VGH voltage level
    //   Range: 0x00-0x0F
    //   Formula: VGH = 10V + (value × 0.5V)
    //   0x0E = 10V + (14 × 0.5V) = 17V
    // Parameter 2 [7:0]: VGL voltage level  
    //   Range: 0x00-0x0F
    //   Formula: VGL = -5V - (value × 0.5V)
    //   0x0A = -5V - (10 × 0.5V) = -10V
    // ------------------------------------------------------------------------
    sendCommand(ST7305_GCTRL);
    sendData(0x0E);  // VGH = 15V (gate high voltage)
    sendData(0x0A);  // VGL = -10V (gate low voltage)
    
    // ------------------------------------------------------------------------
    // VSHP Control (0xC1) - Source High Positive Voltage
    // Sets positive voltage levels for grayscale (4 levels in mono mode)
    // Parameter 1-4 [7:0]: VSHP1, VSHP2, VSHP3, VSHP4 voltage levels
    //   Range: 0x00-0x7F
    //   Formula: VSHP = 2.4V + (value × 0.04V)
    //   0x41 = 2.4V + (65 × 0.04V) = 5.0V
    // ------------------------------------------------------------------------
    sendCommand(ST7305_VSHPCTRL);
    sendData(0x41);  // VSHP1 = 5.0V
    sendData(0x41);  // VSHP2 = 5.0V
    sendData(0x41);  // VSHP3 = 5.0V
    sendData(0x41);  // VSHP4 = 5.0V
    
    // ------------------------------------------------------------------------
    // VSLP Control (0xC2) - Source Low Positive Voltage
    // Sets low positive voltage levels for grayscale
    // Parameter 1-4 [7:0]: VSLP1, VSLP2, VSLP3, VSLP4 voltage levels
    //   Range: 0x00-0x7F
    //   Formula: VSLP = -0.4V + (value × 0.04V)
    //   0x32 = -0.4V + (50 × 0.04V) = 1.6V
    // ------------------------------------------------------------------------
    sendCommand(ST7305_VSLPCTRL);
    sendData(0x32);  // VSLP1 = 1.6V
    sendData(0x32);  // VSLP2 = 1.6V
    sendData(0x32);  // VSLP3 = 1.6V
    sendData(0x32);  // VSLP4 = 1.6V
    
    // ------------------------------------------------------------------------
    // VSHN Control (0xC4) - Source High Negative Voltage
    // Sets negative voltage levels for grayscale
    // Parameter 1-4 [7:0]: VSHN1, VSHN2, VSHN3, VSHN4 voltage levels
    //   Range: 0x00-0x7F
    //   Formula: VSHN = -2.4V - (value × 0.04V)
    //   0x46 = -2.4V - (70 × 0.04V) = -5.2V
    // ------------------------------------------------------------------------
    sendCommand(ST7305_VSHNCTRL);
    sendData(0x46);  // VSHN1 = -5.2V
    sendData(0x46);  // VSHN2 = -5.2V
    sendData(0x46);  // VSHN3 = -5.2V
    sendData(0x46);  // VSHN4 = -5.2V
    
    // ------------------------------------------------------------------------
    // VSLN Control (0xC5) - Source Low Negative Voltage
    // Sets low negative voltage levels for grayscale
    // Parameter 1-4 [7:0]: VSLN1, VSLN2, VSLN3, VSLN4 voltage levels
    //   Range: 0x00-0x7F
    //   Formula: VSLN = 0.4V - (value × 0.04V)
    //   0x46 = 0.4V - (70 × 0.04V) = -2.4V
    // ------------------------------------------------------------------------
    sendCommand(ST7305_VSLNCTRL);
    sendData(0x46);  // VSLN1 = -2.4V
    sendData(0x46);  // VSLN2 = -2.4V
    sendData(0x46);  // VSLN3 = -2.4V
    sendData(0x46);  // VSLN4 = -2.4V
    
    // ------------------------------------------------------------------------
    // Frame Rate Control (0xB2)
    // Parameter 1 [7:0]: Frame rate settings
    //   Bits [7:4]: HPM (High Power Mode) frame rate
    //     0x0 = 128Hz, 0x1 = 64Hz, 0x2 = 32Hz, 0x3 = 16Hz
    //     0x4 = 8Hz,   0x5 = 4Hz,  0x6 = 2Hz,  0x7 = 1Hz
    //   Bits [3:0]: LPM (Low Power Mode) frame rate
    //     Same values as HPM
    //   0x12 = HPM:32Hz (0x1), LPM:4Hz (0x2)
    // ------------------------------------------------------------------------
    sendCommand(ST7305_FRCTRL);
    sendData(0x12);  // HPM=32Hz, LPM=4Hz
    
    // ------------------------------------------------------------------------
    // Gate EQ Control in HPM (0xB3) - Update Period Gate Equalization
    // 10 parameters control gate signal equalization timing in High Power Mode
    // These values fine-tune the gate driver timing to reduce crosstalk
    // Values are typically copied from reference design
    // ------------------------------------------------------------------------
    sendCommand(ST7305_GTUPEQH);
    sendData(0xE5);  // Gate EQ timing parameter 1
    sendData(0xF6);  // Gate EQ timing parameter 2
    sendData(0x05);  // Gate EQ timing parameter 3
    sendData(0x46);  // Gate EQ timing parameter 4
    sendData(0x77);  // Gate EQ timing parameter 5
    sendData(0x77);  // Gate EQ timing parameter 6
    sendData(0x77);  // Gate EQ timing parameter 7
    sendData(0x77);  // Gate EQ timing parameter 8
    sendData(0x76);  // Gate EQ timing parameter 9
    sendData(0x45);  // Gate EQ timing parameter 10
    
    // ------------------------------------------------------------------------
    // Gate EQ Control in LPM (0xB4) - Update Period Gate Equalization
    // 8 parameters control gate signal equalization timing in Low Power Mode
    // Similar to HPM but optimized for slower refresh rates
    // ------------------------------------------------------------------------
    sendCommand(ST7305_GTUPEQL);
    sendData(0x05);  // Gate EQ timing parameter 1
    sendData(0x46);  // Gate EQ timing parameter 2
    sendData(0x77);  // Gate EQ timing parameter 3
    sendData(0x77);  // Gate EQ timing parameter 4
    sendData(0x77);  // Gate EQ timing parameter 5
    sendData(0x77);  // Gate EQ timing parameter 6
    sendData(0x76);  // Gate EQ timing parameter 7
    sendData(0x45);  // Gate EQ timing parameter 8
    
    // ------------------------------------------------------------------------
    // Source EQ Enable (0xB7)
    // Parameter 1 [7:0]: Source equalization settings
    //   Enables source driver equalization to reduce crosstalk
    //   0x00 = Disabled
    //   0x13 = Enabled with specific timing
    // ------------------------------------------------------------------------
    sendCommand(ST7305_SOUEQ);
    sendData(0x13);  // Enable source EQ with timing=0x13
    
    // ------------------------------------------------------------------------
    // Gate Line Setting (0xB0)
    // Parameter 1 [7:0]: Number of gate lines
    //   Bits [7:0]: Gate line count
    //   For 320 lines with 2-line interlace:
    //     0x50 = 160 physical scan lines × 2 = 320 display lines
    // ------------------------------------------------------------------------
    sendCommand(ST7305_GATESET);
    //sendData(0x50);  // 320 lines (160 × 2-line interlace)
    //sendData(ST7305_HEIGHT / 4);
    sendData(0x64);   //400 lines
    
    // ------------------------------------------------------------------------
    // Sleep Out (0x11)
    // Wakes display from sleep mode, must wait 120ms after this command
    // No parameters
    // ------------------------------------------------------------------------
    sendCommand(ST7305_SLPOUT);
    delay(120);  // Must wait after sleep out
    
    // ------------------------------------------------------------------------
    // OSC Setting (0xD8)
    // Parameter 1 [7:0]: Oscillator frequency setting
    //   Bits [7:0]: OSC frequency control
    //   0x80 = ~51Hz oscillator frequency
    // Parameter 2 [7:0]: Additional OSC control
    //   0xE9 = Extended OSC settings
    // ------------------------------------------------------------------------
    sendCommand(ST7305_OSCSET);
    sendData(0x26);  // OSC frequency ~51Hz
    sendData(0xE9);  // Extended OSC settings
    
    // ------------------------------------------------------------------------
    // Source Voltage Select (0xC9)
    // Parameter 1 [7:0]: Select which VSHP/VSLP/VSHN/VSLN to use
    //   Bits [7:6]: VSHP select (00=VSHP1, 01=VSHP2, 10=VSHP3, 11=VSHP4)
    //   Bits [5:4]: VSLP select (00=VSLP1, 01=VSLP2, 10=VSLP3, 11=VSLP4)
    //   Bits [3:2]: VSHN select (00=VSHN1, 01=VSHN2, 10=VSHN3, 11=VSHN4)
    //   Bits [1:0]: VSLN select (00=VSLN1, 01=VSLN2, 10=VSLN3, 11=VSLN4)
    //   0x00 = Use all "1" voltages (VSHP1, VSLP1, VSHN1, VSLN1)
    // ------------------------------------------------------------------------
    sendCommand(ST7305_VSHLSEL);
    sendData(0x00);  // Use VSHP1, VSLP1, VSHN1, VSLN1
    
    // ------------------------------------------------------------------------
    // Memory Data Access Control (0x36)
    // Parameter 1 [7:0]: Display orientation and color order
    //   Bit 7 (MY): Row address order (0=top-to-bottom, 1=bottom-to-top)
    //   Bit 6 (MX): Column address order (0=left-to-right, 1=right-to-left)
    //   Bit 5 (MV): Row/column exchange (0=normal, 1=reversed)
    //   Bit 4 (ML): Vertical refresh order (0=LCD refresh top-to-bottom)
    //   Bit 3 (RGB): Color order (0=RGB, 1=BGR) - N/A for mono
    //   Bit 2 (MH): Horizontal refresh order
    //   0x48 = 01001000b = MY=0, MX=1, MV=0, ML=0, RGB=1
    //   Common values:
    //     0x00 = Normal
    //     0x40 = X-mirror
    //     0x80 = Y-mirror  
    //     0xC0 = XY-mirror
    //     0x20 = X-Y exchange
    // ------------------------------------------------------------------------
    sendCommand(ST7305_MADCTL);
    sendData(0x00);  // X-mirror, specific refresh order
    
    // ------------------------------------------------------------------------
    // Data Format Select (0x3A)
    // Parameter 1 [7:0]: Data input format
    //   Bit 4 (DPI): Display pixel format
    //     0 = 8-bit/pixel, 1 = 1-bit/pixel (monochrome)
    //   Bit 0 (BPS): Bit per symbol
    //     0 = 4 operations for 24-bit
    //     1 = 3 operations for 24-bit
    //   0x10 = 00010000b = 1-bit monochrome mode
    // ------------------------------------------------------------------------
    sendCommand(ST7305_DTFORM);
    sendData(0x10);  // 1-bit monochrome mode
    
    // ------------------------------------------------------------------------
    // Gamma Mode Setting (0xB9)
    // Parameter 1 [7:0]: Gamma curve selection
    //   0x20 = Monochrome mode
    //   0x48 = Grayscale mode with gamma curve 1
    // Options:
    //   0x20 = Mono mode (black/white only)
    //   0x48 = 4-level grayscale
    // ------------------------------------------------------------------------
    sendCommand(ST7305_GAMAMS);
    sendData(0x20);  // Monochrome mode (not grayscale)
    
    // ------------------------------------------------------------------------
    // Panel Setting (0xB8)
    // Parameter 1 [7:0]: Panel configuration
    //   Bit 0: Panel layout (0=1-line, 1=2-line interlace)
    //   0x00 = Specific panel configuration
    // ------------------------------------------------------------------------
    sendCommand(ST7305_PNLSET);
    sendData(0x29);  // Panel configuration
    
    // ------------------------------------------------------------------------
    // Column Address Set (0x2A)
    // For ST7305 in 2-line interlace mode, uses simplified 2-byte format
    // Parameter 1: Start column MSB
    // Parameter 2: End column LSB
    //   0x13 = 19 (start column)
    //   0x28 = 40 (end column offset)
    // This maps to the 264 source columns
    // ------------------------------------------------------------------------
    sendCommand(ST7305_CASET);
    sendData(0x13);  // Column start = 19
    sendData(0x28);  // Column range = 40
    //sendData(0x12);  // Column range = 40
    //sendData(0x2B);  // Column range = 40
    
    // ------------------------------------------------------------------------
    // Row Address Set (0x2B)
    // For ST7305 in 2-line interlace mode, uses simplified 2-byte format
    // Parameter 1: Start row
    // Parameter 2: End row
    //   0x00 = Row 0
    //   0x9F = Row 159 (160 physical rows × 2 interlace = 320 lines)
    // ------------------------------------------------------------------------
    sendCommand(ST7305_RASET);
    sendData(0x00);  // Row start = 0
    //sendData(0x9F);  // Row end = 159 (×2 interlace = 320)
    //sendData((ST7305_HEIGHT / 2) - 1); // End row
    
    sendData(0xC7);  
    
    // ------------------------------------------------------------------------
    // Tearing Effect Line On (0x35)
    // Parameter 1 [0]: TE signal mode
    //   0x00 = Mode 1: V-blanking only
    //   0x01 = Mode 2: V-blanking and H-blanking
    // Enables synchronization signal for smooth updates
    // ------------------------------------------------------------------------
    //sendCommand(ST7305_TEON);
    sendCommand(ST7305_TEON);
    sendData(0x00);  // TE mode 1 (V-blanking)
    
    // ------------------------------------------------------------------------
    // Enable Auto Power Down (0xD0)
    // Parameter 1 [7:0]: Auto power down settings
    //   0x00 = Disable auto power down in normal mode
    //   0xFF = Enable auto power down with maximum timeout
    // ------------------------------------------------------------------------
    sendCommand(ST7305_AUTOPWRCTRL);
    sendData(0xFF);  // Enable auto power down
    
    // ------------------------------------------------------------------------
    // Low Power Mode (0x39)
    // Enters low power mode (1Hz refresh in LPM setting)
    // No parameters
    // Alternative: Use ST7305_HPM (0x38) for High Power Mode
    // ------------------------------------------------------------------------
    sendCommand(ST7305_LPM);
    
    // ------------------------------------------------------------------------
    // Display On (0x29)
    // Turns on display output
    // No parameters
    // ------------------------------------------------------------------------
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
