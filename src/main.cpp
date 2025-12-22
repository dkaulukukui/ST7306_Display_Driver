#include <Arduino.h>
#include "ST7306_Mono.h"

// =======================================================
// --- Pin Configuration for 4-wire SPI ---
// Adjust these according to your actual wiring
// =======================================================
#define PIN_DC    1  // Data/Command
#define PIN_RST   0  // Reset
#define PIN_CS    2  // Chip Select
// SPI MOSI and CLK are hardware SPI pins on nRF52840

// =======================================================
// --- Display Instance ---
// =======================================================
ST7306_Mono display(PIN_DC, PIN_RST, PIN_CS);

// =======================================================
// --- Demo Functions ---
// =======================================================

void testDrawText() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(ST7306_BLACK);
    display.setCursor(0, 0);
    display.println("ST7306 Mono Display");
    display.println("LH420NB-F07");
    display.println("300x400 pixels");
    display.println();
    display.setTextSize(2);
    display.println("Adafruit GFX");
    display.display();
}

void testDrawShapes() {
    display.clearDisplay();
    
    // Draw rectangles
    display.drawRect(10, 10, 100, 50, ST7306_BLACK);
    display.fillRect(120, 10, 100, 50, ST7306_BLACK);
    
    // Draw circles
    display.drawCircle(60, 100, 30, ST7306_BLACK);
    display.fillCircle(170, 100, 30, ST7306_BLACK);
    
    // Draw triangles
    display.drawTriangle(30, 180, 60, 220, 0, 220, ST7306_BLACK);
    display.fillTriangle(140, 180, 170, 220, 110, 220, ST7306_BLACK);
    
    // Draw lines
    for (int i = 0; i < 300; i += 20) {
        display.drawLine(0, 250, i, 380, ST7306_BLACK);
    }
    
    display.display();
}

void testScrollText() {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(ST7306_BLACK);
    
    for (int16_t y = 400; y > -100; y -= 2) {
        display.clearDisplay();
        display.setCursor(50, y);
        display.println("Scrolling");
        display.setCursor(50, y + 30);
        display.println("Text");
        display.setCursor(50, y + 60);
        display.println("Demo");
        display.display();
        delay(10);
    }
}

void testFillScreen() {
    // Test fill white
    display.clearDisplay();
    display.display();
    delay(1000);
    
    // Test fill black
    display.fillScreen(ST7306_BLACK);
    display.display();
    delay(1000);
    
    // Test invert
    display.invertDisplay(true);
    delay(1000);
    display.invertDisplay(false);
}

void testPixels() {
    display.clearDisplay();
    
    // Draw random pixels
    for (int i = 0; i < 1000; i++) {
        int x = random(0, ST7306_WIDTH);
        int y = random(0, ST7306_HEIGHT);
        display.drawPixel(x, y, ST7306_BLACK);
    }
    
    display.display();
}

void drawBitmap() {
    display.clearDisplay();
    display.setTextSize(3);
    display.setTextColor(ST7306_BLACK);
    display.setCursor(30, 150);
    display.println("READY!");
    display.drawRect(0, 0, ST7306_WIDTH - 1, ST7306_HEIGHT - 1, ST7306_BLACK);
    display.display();
}

// =======================================================
// --- Setup Function ---
// =======================================================
void setup() {
    Serial.begin(115200);
    delay(300);
    
    Serial.println("ST7306 Monochrome Display Test");
    Serial.println("LH420NB-F07 300x400");
    Serial.println();
    
    // Initialize display
    Serial.print("Initializing display... ");
    if (!display.begin(40000000)) {  // 40 MHz SPI
        Serial.println("FAILED!");
        Serial.println("ERROR: Could not allocate frame buffer");
        while (1) delay(1000);
    }
    Serial.println("OK");
    
    Serial.print("Display size: ");
    Serial.print(display.width());
    Serial.print(" x ");
    Serial.println(display.height());
    Serial.println();
    
    // Show initial message
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(ST7306_BLACK);
    display.setCursor(40, 180);
    display.println("Initializing...");
    display.display();
    delay(1000);
    
    Serial.println("Running demo tests...");
}

// =======================================================
// --- Loop Function ---
// =======================================================
void loop() {
    Serial.println("Test 1: Drawing text");
    testDrawText();
    delay(3000);
    
    Serial.println("Test 2: Drawing shapes");
    testDrawShapes();
    delay(3000);
    
    Serial.println("Test 3: Fill screen test");
    testFillScreen();
    delay(1000);
    
    Serial.println("Test 4: Random pixels");
    testPixels();
    delay(3000);
    
    Serial.println("Test 5: Scrolling text");
    testScrollText();
    delay(1000);
    
    Serial.println("Test 6: Ready screen");
    drawBitmap();
    delay(3000);
    
    Serial.println("--- Tests complete, restarting ---\n");
    delay(1000);
}

