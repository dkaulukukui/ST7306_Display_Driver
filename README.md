# ST7306 Monochrome Display Driver for PlatformIO

This project provides a complete monochrome (1-bit) display driver for the ST7306-based LH420NB-F07 4.2" display with Adafruit GFX library support for PlatformIO.

## Hardware Specifications

**Display:** LH420NB-F07 4.2" Monochrome EPD  
**Resolution:** 300×400 pixels  
**Controller:** ST7306  
**Colors:** Monochrome (Black & White)  
**Interface:** 4-wire SPI  

ST7306 specifications: https://admin.osptek.com/uploads/ST_7306_V0_1_c30c3541a3.pdf

## Hardware Requirements

- ESP32-based board (Nordic nRF52840 or similar)
- LH420NB-F07 display (300×400 pixels, ST7306 controller)
- 4-wire SPI connection:
  - MOSI (Data) - Hardware SPI pin
  - CLK (Clock) - Hardware SPI pin
  - CS (Chip Select)
  - DC (Data/Command)
  - RST (Reset) - Optional but recommended

## Features

- **Monochrome display driver** with 1-bit per pixel (15KB frame buffer)
- **Adafruit GFX library integration** for easy graphics
- **4-wire SPI** with hardware SPI support (40MHz)
- Simple and efficient memory usage
- Full GFX primitives: text, shapes, lines, pixels
- Optimized batch data transfers
- Easy to use API compatible with Adafruit GFX ecosystem

## Pin Configuration

Default pin assignments (modify in `src/main.cpp`):

```cpp
#define PIN_DC    1  // Data/Command
#define PIN_RST   0  // Reset (optional)
#define PIN_CS    2  // Chip Select
// MOSI and CLK use hardware SPI pins
```

## Project Structure

```
ST7306/
├── lib/
│   └── ST7306_Display/
│       ├── ST7306_Mono.h       # Monochrome display driver header
│       └── ST7306_Mono.cpp     # Monochrome display driver implementation
├── src/
│   └── main.cpp                # Demo application with GFX examples
├── platformio.ini              # PlatformIO configuration
└── README.md                   # This file
```

## Building and Uploading

1. Open the project in PlatformIO (VS Code)
2. Build the project:
   ```
   pio run
   ```
3. Upload to your board:
   ```
   pio run --target upload
   ```
4. Monitor serial output:
   ```
   pio device monitor
   ```

## API Usage

### Basic Display Operations

```cpp
// Create display instance
ST7306_Mono display(PIN_DC, PIN_RST, PIN_CS);

// Initialize display
display.begin(40000000); // 40 MHz SPI

// Clear display (white)
display.clearDisplay();

// Draw single pixel
display.drawPixel(x, y, ST7306_BLACK);  // Black pixel
display.drawPixel(x, y, ST7306_WHITE);  // White pixel

// Update display (send buffer to screen)
display.display();

// Invert display colors
display.invertDisplay(true);  // Invert
display.invertDisplay(false); // Normal

// Set contrast (if needed)
display.setContrast(128);
```

### Adafruit GFX Functions

All standard Adafruit GFX functions are supported:

```cpp
// Text
display.setTextSize(2);
display.setTextColor(ST7306_BLACK);
display.setCursor(10, 10);
display.println("Hello World!");

// Shapes
display.drawRect(x, y, width, height, ST7306_BLACK);
display.fillRect(x, y, width, height, ST7306_BLACK);
display.drawCircle(x, y, radius, ST7306_BLACK);
display.fillCircle(x, y, radius, ST7306_BLACK);
display.drawTriangle(x1, y1, x2, y2, x3, y3, ST7306_BLACK);
display.fillTriangle(x1, y1, x2, y2, x3, y3, ST7306_BLACK);

//Adafruit GFX Library (automatically installed by PlatformIO)
- Adafruit BusIO (automatically installed by PlatformIO)

## Memory Usage

- **Frame Buffer:** ~15KB (300×400 pixels ÷ 8 bits/byte)
- **Total RAM:** ~16-17KB including driver overhead
- **Perfect for nRF52840** with 256KB RAM

## Performance

- **SPI Speed:** 40 MHz (configurable)
- **Full Screen Refresh:** ~100-150ms
- **Efficient batch transfers** for optimal speed

### Colors

```cpp
ST7306_BLACK  // 0 - Black pixel
ST7306_WHITE  // 1 - White pixel
```

## Library Dependencies

- LVGL 8.3.11 (automatically installed by PlatformIO)

## Performance Optimization

- Uses DMA for SPI transfers
- Batch processing for landscape rendering (32 rows per batch)
- Hardware interrupt-driven refresh using TE signal
- Optimized bit manipulation for color conversion

## Transparent Overlay Mode

The transparent functions (`trans_disp_flush` and `trans_disp_flush_landscape`) only render pixels that match the 8 supported colors, leaving other pixels unchanged. This is useful for overlay applications.

## LicensST7306 display controller implementation  
MIT License

## References

- Adafruit GFX Library: https://github.com/adafruit/Adafruit-GFX-Library
- ST7306 datasheet: https://admin.osptek.com/uploads/ST_7306_V0_1_c30c3541a3.pdf
- LH420NB-F07 display documentation

## Troubleshooting

### Display not working
- Check SPI pin connections (CS, DC, RST)
- Verify power supply (3.3V)
- Ensure MOSI and CLK are on hardware SPI pins
- Try lowering SPI speed: `display.begin(20000000)` // 20MHz

### Nothing appears on screen
- Make sure to call `display.display()` after drawing
- Try `display.invertDisplay(true)` - some displays have inverted logic
- Check if display.begin() returns true

### Partial display or artifacts
- Check frame buffer allocation succeeded
- Verify sufficient RAM is available
- Try reducing SPI speed
- Check for loose connections

### Text looks wrong
- Ensure font size is set: `display.setTextSize(1)` or higher
- Verify text color: `display.setTextColor(ST7306_BLACK)`
- Check cursor position is on screen

## Demo Tests Included

The example code includes comprehensive tests:
1. **Text rendering** - Multiple sizes and positions
2. **Shape drawing** - Rectangles, circles, triangles
3. **Fill screen** - Black, white, and invert test
4. **Random pixels** - Performance test
5. **Scrolling text** - Animation demo
6. **Border frame** - Full screen test

## Support

For issues and questions:
- Adafruit GFX documentation: https://learn.adafruit.com/adafruit-gfx-graphics-library
- ST7306 controller referencer to:
- Original repository: https://github.com/FT-tele/ST7306_8color_lvgl
- LVGL forums: https://forum.lvgl.io/
- ST1307 Datasheet: https://admin.osptek.com/uploads/ST_7306_V0_1_c30c3541a3.pdf
