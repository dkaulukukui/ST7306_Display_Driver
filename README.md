# ST7305 Monochrome Display Driver for Arduino

Arduino library for ST7305 monochrome TFT displays using SPI. Designed for displays with 300×400 resolution using the ST7305 controller chip.

## Features

- **Monochrome Display Support**: 4 pixels per byte packed format
- **Memory Efficient**: 15KB frame buffer for 300×400 display  
- **SPI Interface**: Hardware SPI for high-speed communication (up to 40MHz)
- **Adafruit GFX Compatible**: Full support for Adafruit GFX library functions
- **Flexible Configuration**: Multiple init command sets with single-point switching
- **Display Control**: Power management, inversion, and fill operations
- **Tested Platform**: Adafruit Feather M4 Express (SAMD51)

## Hardware Specifications

- **Controller**: ST7305
- **Resolution**: 300×400 pixels (width × height)
- **Interface**: 4-wire SPI (MOSI, SCK, CS, DC) + Reset
- **Colors**: Monochrome (Black & White)
- **Memory Layout**: 4 pixels per byte horizontally, 2 rows per vertical group
- **SPI Mode**: Mode 0 (CPOL=0, CPHA=0)
- **SPI Speed**: Up to 40MHz (tested stable, 1MHz for debugging)
- **Voltage**: 3.3V logic level

## Memory Layout

The ST7305 uses a unique memory organization:
- **Buffer Size**: 15,000 bytes = (WIDTH/4) × (HEIGHT/2)
- **Horizontal**: 4 pixels per byte (300 pixels ÷ 4 = 75 bytes per row)
- **Vertical**: 2 rows grouped together (400 rows ÷ 2 = 200 row-pairs)
- **Bit Mapping**: Pixels mapped to bits using formula: `7 - (line_bit_4 * 2 + one_two)`

### Buffer Calculation
```
Total bytes = (300 / 4) × (400 / 2)
           = 75 bytes/row × 200 row-pairs
           = 15,000 bytes
```

## Pin Configuration

The library requires 3 control pins:
- **DC (Data/Command)**: Selects command or data mode
- **RST (Reset)**: Hardware reset line
- **CS (Chip Select)**: SPI chip select

Standard SPI pins (MOSI, SCK) are shared with other SPI devices.

Example pin configuration for Feather M4 Express:
```cpp
#define TFT_DC   10
#define TFT_RST  9
#define TFT_CS   11
```

## Installation

### Option 1: PlatformIO (Recommended)
1. Add the library to your `lib` folder
2. Include in your `platformio.ini`:
```ini
lib_deps = 
    adafruit/Adafruit GFX Library@^1.11.9
    adafruit/Adafruit BusIO@^1.15.0
```

### Option 2: Arduino IDE
1. Copy the `ST7305_Display` folder to your Arduino libraries folder
2. Install dependencies via Library Manager:
   - Adafruit GFX Library
   - Adafruit BusIO

## Quick Start

```cpp
#include <ST7305_Mono.h>

// Pin definitions
#define TFT_DC   10
#define TFT_RST  9
#define TFT_CS   11

// Create display object
ST7305_Mono display(TFT_DC, TFT_RST, TFT_CS);

void setup() {
  Serial.begin(115200);
  
  // Initialize display (defaults to 1MHz for stability)
  if (!display.begin()) {
    Serial.println("Display init failed!");
    while(1);
  }
  
  // Clear to black background
  display.clearDisplay();
  
  // Draw white text on black background
  display.setTextSize(2);
  display.setTextColor(ST7305_WHITE);
  display.setCursor(10, 10);
  display.println("Hello World!");
  
  // Update display
  display.display();
}

void loop() {
  // Your code here
}
```

## Project Structure

```
lib/
├── ST7305_Display/
│   ├── ST7305_Mono.h      # Header with class definition and init configs
│   └── ST7305_Mono.cpp    # Implementation
src/
└── main.cpp               # Example application with 8 test functions
```

## Configuration System

The library supports multiple initialization command sets that can be switched with a single line change.

### Available Configurations
- `st7305_init_cmds_default` - Standard initialization
- `st7305_init_cmds_FT_tele` - FT_tele reference implementation
- `st7305_init_cmds_kevin` - Custom configuration variant
- `st7305_init_cmds_mfg` - Manufacturer settings

### Switching Configurations

Edit **ST7305_Mono.h** and change the `ACTIVE_INIT_CMDS` macro:

```cpp
// Select which init command set to use
#define ACTIVE_INIT_CMDS st7305_init_cmds_default
```

Change to:
```cpp
#define ACTIVE_INIT_CMDS st7305_init_cmds_FT_tele
```

Recompile and upload. No other code changes needed!

## Building the Project

### PlatformIO
```bash
pio run                      # Build
pio run --target upload      # Upload to board
pio device monitor           # Open serial monitor
```

### Arduino IDE
1. Open `src/main.cpp` or create a new sketch
2. Select Tools → Board → Adafruit Feather M4 Express
3. Select the correct COM port
4. Click Upload

## API Reference

### Display Control Functions

#### `bool begin(uint32_t freq = 1000000)`
Initialize the display with specified SPI frequency. Returns `true` on success. Default 1MHz for stability.
```cpp
// Initialize at 1MHz (recommended)
if (!display.begin()) {
  Serial.println("Init failed!");
}

// Or initialize at 40MHz for faster updates
if (!display.begin(40000000)) {
  Serial.println("Init failed!");
}
```

#### `void display()`
Transfer the frame buffer to the display. Must be called after drawing to make changes visible.

This function:
1. Sets column address range (0x2A): 0x12 to 0x2A
2. Sets row address range (0x2B): 0x00 to 0xC7  
3. Sends memory write command (0x2C)
4. Transfers entire 15KB buffer via SPI

```cpp
display.drawPixel(10, 10, ST7305_WHITE);
display.display();  // Now the pixel is visible
```

#### `void clearDisplay()`
Clear the entire frame buffer to black (0x00). Doesn't update the display until `display()` is called.
```cpp
display.clearDisplay();  // Buffer cleared to black
display.display();       // Screen now black
```

#### `void fill(uint8_t value)`
Fill the entire frame buffer with a specific byte value. Useful for patterns or testing.
```cpp
display.fill(0x00);    // Fill with black
display.fill(0xFF);    // Fill with white
display.fill(0xAA);    // Fill with alternating pattern
display.display();     // Make visible
```

#### `void invertDisplay(bool invert)`
Invert all pixels on the display (hardware inversion).
```cpp
display.invertDisplay(true);   // White becomes black, black becomes white
display.invertDisplay(false);  // Normal display
```

### Power Management

#### `void displayOn()`
Turn the display on (exit sleep mode). Sends command 0x29.

#### `void displayOff()`
Turn the display off (enter sleep mode) to save power. Sends command 0x28.

#### `void setIdleMode(bool idle)`
Enable or disable idle mode for reduced power consumption.
```cpp
display.setIdleMode(true);   // Enter idle mode (0x39)
display.setIdleMode(false);  // Exit idle mode (0x38)
```

### Drawing Functions (via Adafruit GFX)

The library inherits all drawing functions from Adafruit_GFX. **Important**: Draw with `ST7305_WHITE` for visibility on black background.

#### `void drawPixel(int16_t x, int16_t y, uint16_t color)` (Override)
Draw a single pixel. This is the core drawing primitive used by all other GFX functions.

**Pixel Mapping Algorithm**:
```cpp
real_x = x / 4;              // 4 pixels per byte horizontally
real_y = y / 2;              // 2 rows per vertical group
line_bit_4 = x % 4;          // Which pixel in the 4-pixel group
one_two = y % 2;             // Which of the 2 rows
write_bit = 7 - (line_bit_4 * 2 + one_two);  // Final bit position
```

Example:
```cpp
display.drawPixel(10, 20, ST7305_WHITE);  // Draw white pixel
display.drawPixel(10, 20, ST7305_BLACK);  // Draw black pixel
```

#### Text
```cpp
display.setTextSize(2);               // 2x size
display.setTextColor(ST7305_WHITE);   // White text on black background
display.setCursor(0, 0);              // Position
display.print("Hello");               // Draw text
display.display();                    // Make visible
```

#### Shapes
```cpp
display.drawPixel(x, y, ST7305_WHITE);            // Single pixel
display.drawLine(x0, y0, x1, y1, ST7305_WHITE);   // Line
display.drawRect(x, y, w, h, ST7305_WHITE);       // Rectangle outline
display.fillRect(x, y, w, h, ST7305_WHITE);       // Filled rectangle
display.drawCircle(x, y, r, ST7305_WHITE);        // Circle outline
display.fillCircle(x, y, r, ST7305_WHITE);        // Filled circle
display.drawTriangle(x0, y0, x1, y1, x2, y2, ST7305_WHITE);  // Triangle
```

#### Advanced
```cpp
// Draw bitmap (1-bit)
display.drawBitmap(x, y, bitmap, w, h, ST7305_WHITE);

// Draw XBitmap
display.drawXBitmap(x, y, bitmap, w, h, ST7305_WHITE);
```

### Color Constants
```cpp
ST7305_BLACK  // 0 - Black pixel
ST7305_WHITE  // 1 - White pixel
```

**Tip**: Use `ST7305_WHITE` for visible drawing on the default black background.

## Memory Usage

- **Frame Buffer**: 15,000 bytes = (300/4) × (400/2)
- **Code Size**: ~10KB flash
- **RAM Overhead**: ~200 bytes (excluding frame buffer)

The frame buffer is allocated on the heap during `begin()`.

## Performance

| Operation | Time | Notes |
|-----------|------|-------|
| Hardware Reset | ~180ms | One-time at startup |
| Init Sequence | ~250ms | One-time at startup |
| Full Screen Refresh | ~100-150ms | 15,000 bytes @ 40MHz |
| drawPixel() | <1μs | Just updates RAM buffer |
| display() call | ~100ms | Transfers entire buffer |
| SPI Transaction | ~2.5μs/byte | At 40MHz |

## Example Applications

The library includes 8 demonstration functions in `src/main.cpp`:

1. **testSinglePixel()** - Draw and move a single white pixel
2. **testDrawText()** - Display text at various sizes
3. **testDrawShapes()** - Draw rectangles, circles, triangles, and lines
4. **testFillScreen()** - Fill screen with patterns using fill() method
5. **testPixels()** - Draw random pixels across the display
6. **testPowerModes()** - Test display on/off and idle modes
7. **testScrollText()** - Animate scrolling text
8. **drawBitmap()** - Display "READY" message centered on screen

## Troubleshooting

### Display shows nothing
- Check SPI connections (MOSI, SCK, CS, DC, RST)
- Verify 3.3V power supply
- Try lower SPI speed: `display.begin(1000000)`
- Ensure you're drawing with `ST7305_WHITE` not `ST7305_BLACK`
- Call `display.display()` after drawing

### Garbled display
- Buffer overflow or memory corruption
- Check available RAM (15KB needed for buffer)
- Verify correct pin configuration
- Try different init command set via `ACTIVE_INIT_CMDS`

### Black screen after init
- Display may have black background by default
- Draw with `ST7305_WHITE` for visibility
- Check `clearDisplay()` fills with 0x00 (black)

### Slow updates
- Increase SPI speed to 40MHz
- Minimize number of `display()` calls
- Batch drawing operations before calling `display()`

### Initialization fails
- Check return value of `begin()`
- Verify all pin connections
- Ensure sufficient power supply current
- Try different init configuration sets

## Advanced Usage

### Direct Buffer Access
```cpp
// Access the raw buffer (use with caution)
uint8_t* buffer = display.getBuffer();
buffer[0] = 0xFF;  // Directly modify buffer
display.display(); // Update display
```

### Custom Init Commands
Create your own init command set in `ST7305_Mono.h`:
```cpp
static const st7305_lcd_init_cmd_t st7305_init_cmds_custom[] = {
    {0x11, {}, 0, 120},              // Sleep out, delay 120ms
    {0x29, {}, 0, 0},                // Display on
};

static const int st7305_init_cmds_custom_count = 
    sizeof(st7305_init_cmds_custom) / sizeof(st7305_lcd_init_cmd_t);
```

Then set:
```cpp
#define ACTIVE_INIT_CMDS st7305_init_cmds_custom
```

### Partial Screen Updates
While the library doesn't support partial updates natively, you can optimize by only calling `display()` when needed:
```cpp
// Bad: Updates after every pixel
for (int i = 0; i < 100; i++) {
    display.drawPixel(i, 10, ST7305_WHITE);
    display.display();  // 100 full screen updates!
}

// Good: Batch operations
for (int i = 0; i < 100; i++) {
    display.drawPixel(i, 10, ST7305_WHITE);
}
display.display();  // Single full screen update
```

## Technical Details

### Pixel Bit Mapping
The ST7305 uses a unique addressing scheme:
- Each byte contains 4 horizontal pixels
- Every 2 vertical rows share the same byte group
- Bit position calculated as: `7 - (line_bit_4 * 2 + one_two)`

Example for pixel at (5, 10):
```
x = 5, y = 10
real_x = 5 / 4 = 1
real_y = 10 / 2 = 5
line_bit_4 = 5 % 4 = 1
one_two = 10 % 2 = 0
write_bit = 7 - (1 * 2 + 0) = 5

Buffer index = real_y * 75 + real_x = 5 * 75 + 1 = 376
Bit position = 5

buffer[376] bit 5 controls pixel (5, 10)
```

### Display Update Sequence
The `display()` function follows this sequence:
1. Send column address command (0x2A) with range 0x12-0x2A
2. Send row address command (0x2B) with range 0x00-0xC7
3. Send memory write command (0x2C)
4. Transfer all 15,000 bytes via SPI

This matches the reference implementation and ensures proper rendering.

## References

- Adafruit GFX Library: https://github.com/adafruit/Adafruit-GFX-Library
- Reference implementation: FT_tele_ST7305 library
- ST7305 controller documentation (if available)

## Support

For issues, questions, or contributions:
- Check troubleshooting section above
- Review existing code comments
- Test with different init command sets
- Verify hardware connections and power supply

## License

This library is provided as-is for educational and commercial use. Please respect any applicable display manufacturer licensing requirements.

## Version History

- **v1.0**: Initial ST7305 driver implementation
  - 15KB frame buffer with 4-pixel-per-byte layout
  - Multiple init command configurations
  - Single-point configuration switching
  - Full Adafruit GFX integration
  - Tested on Feather M4 Express
