# ST7306 Display Driver for PlatformIO

This project provides a complete driver implementation for the ST7306 8-color reflective display with LVGL (Light and Versatile Graphics Library) support for PlatformIO.

## Hardware Specifications

ST7306 specifications: https://admin.osptek.com/uploads/ST_7306_V0_1_c30c3541a3.pdf

## Hardware Requirements

- ESP32-based board (Nordic nRF52840 in this example)
- ST7306 8-color reflective display (210x480 pixels)
- SPI connection:
  - MOSI (Data)
  - CLK (Clock)
  - CS (Chip Select)
  - DC (Data/Command)
  - RST (Reset)
  - TE (Tearing Effect)

## Features

- Full ST7306 display driver with SPI DMA support
- LVGL 8.3.11 integration
- Portrait and landscape orientation support
- Hardware interrupt-based refresh using TE signal
- Optimized batch rendering for landscape mode
- Support for 8 colors (White, Red, Green, Blue, Yellow, Cyan, Magenta, Black)
- Transparent overlay mode (whitelist-based rendering)

## Pin Configuration

Default pin assignments (modify in `src/main.cpp`):

```cpp
#define PIN_TE    5
#define PIN_CLK   4
#define PIN_MOSI  3
#define PIN_CS    2
#define PIN_DC    1
#define PIN_RST   0
```

## Project Structure

```
ST7306/
├── include/
│   └── lv_conf.h          # LVGL configuration
├── lib/
│   └── ST7306_Display/
│       ├── ST7306_LCD.h        # Display driver header
│       ├── ST7306_LCD.cpp      # Display driver implementation
│       ├── ST7306_LVGL.h       # LVGL integration header
│       └── ST7306_LVGL.cpp     # LVGL integration implementation
├── src/
│   └── main.cpp           # Example application
├── platformio.ini         # PlatformIO configuration
└── README.md             # This file
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

## Orientation Configuration

To switch between portrait and landscape modes, edit `src/main.cpp`:

```cpp
// Portrait mode (210x480)
#define SCREEN_ORIENTATION_PORTRAIT 1

// Landscape mode (480x210)
#define SCREEN_ORIENTATION_PORTRAIT 0
```

## API Usage

### Basic Display Operations

```cpp
// Initialize display
lcd.begin();

// Clear display
lcd.clearDisplay();

// Draw single pixel
lcd.drawPixel(x, y, color);

// Fill entire screen
lcd.fillScreen(0xFFFF); // White

// Show color bars (test pattern)
lcd.drawColorBars();

// Refresh display
lcd.refresh();
```

### LVGL Integration

The project includes LVGL display flush callbacks:

- `disp_flush()` - Portrait mode (210x480)
- `disp_flush_landscape()` - Landscape mode (480x210)
- `trans_disp_flush()` - Transparent portrait mode
- `trans_disp_flush_landscape()` - Transparent landscape mode

### Supported Colors (RGB565)

```cpp
0xFFFF  // White
0xF800  // Red
0x07E0  // Green
0x001F  // Blue
0xFFE0  // Yellow
0x07FF  // Cyan
0xF81F  // Magenta
0x0000  // Black
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

## License

Based on the ST7306_8color_lvgl library by FT-tele
MIT License

## References

- Original library: https://github.com/FT-tele/ST7306_8color_lvgl
- LVGL documentation: https://docs.lvgl.io/
- ST7306 datasheet: https://admin.osptek.com/uploads/ST_7306_V0_1_c30c3541a3.pdf

## Troubleshooting

### Display not working
- Check SPI pin connections
- Verify power supply (3.3V)
- Ensure TE signal is connected properly

### Colors incorrect
- Verify color format is RGB565
- Check that colors match the 8 supported colors

### LVGL errors
- Verify lv_conf.h is properly configured
- Check memory allocation (LV_MEM_SIZE)
- Ensure LVGL is initialized before creating objects

## Support

For issues and questions, please refer to:
- Original repository: https://github.com/FT-tele/ST7306_8color_lvgl
- LVGL forums: https://forum.lvgl.io/
- ST1307 Datasheet: https://admin.osptek.com/uploads/ST_7306_V0_1_c30c3541a3.pdf
