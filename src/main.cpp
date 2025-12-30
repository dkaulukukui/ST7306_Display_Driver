#include <Arduino.h>
#include "ST7305_Mono.h"

// =======================================================
// --- Pin Configuration for 4-wire SPI ---
// =======================================================
#define PIN_DC    10  // Data/Command (RS on display)
#define PIN_RST   9   // Reset
#define PIN_CS    11  // Chip Select
// SPI MOSI and CLK are hardware SPI pins on Feather M4 Express

// =======================================================
// --- Display Instance ---
// =======================================================
ST7305_Mono display(PIN_DC, PIN_RST, PIN_CS);

// =======================================================
// --- Demo Functions ---
// =======================================================

void testSinglePixel() {
    display.clearDisplay();
    
    // Draw one pixel in the center of the screen
    int centerX = ST7305_WIDTH / 2;
    int centerY = ST7305_HEIGHT / 2;
    display.drawPixel(centerX, centerY, ST7305_BLACK);
    
    display.display();
    
    Serial.print("Drew pixel at center: (");
    Serial.print(centerX);
    Serial.print(", ");
    Serial.print(centerY);
    Serial.println(")");
}

void testDrawText() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(ST7305_BLACK);
    display.setCursor(0, 0);
    display.println("ST7305 Mono Display");
    display.println("300x400 pixels");
    display.println();
    display.println("Ultra-low power");
    display.println("TFT controller");
    display.println();
    display.setTextSize(2);
    display.println("Adafruit GFX");
    display.display();
}

void testDrawShapes() {
    display.clearDisplay();
    
    // Draw rectangles
    display.drawRect(10, 10, 100, 50, ST7305_BLACK);
    display.fillRect(120, 10, 100, 50, ST7305_BLACK);
    
    // Draw circles
    display.drawCircle(60, 100, 30, ST7305_BLACK);
    display.fillCircle(170, 100, 30, ST7305_BLACK);
    
    // Draw triangles
    display.drawTriangle(30, 180, 60, 220, 0, 220, ST7305_BLACK);
    display.fillTriangle(140, 180, 170, 220, 110, 220, ST7305_BLACK);
    
    // Draw lines
    for (int i = 0; i < 300; i += 20) {
        display.drawLine(0, 250, i, 380, ST7305_BLACK);
    }
    
    display.display();
}

void testScrollText() {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(ST7305_BLACK);
    
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
    display.fillScreen(ST7305_BLACK);
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
        int x = random(0, ST7305_WIDTH);
        int y = random(0, ST7305_HEIGHT);
        display.drawPixel(x, y, ST7305_BLACK);
    }
    
    display.display();
}

void testPowerModes() {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(ST7305_BLACK);
    display.setCursor(50, 180);
    display.println("High Power");
    display.setCursor(50, 210);
    display.println("Mode Test");
    display.display();
    
    // Switch to High Power Mode for faster updates
    display.setHighPowerMode();
    delay(2000);
    
    display.clearDisplay();
    display.setCursor(50, 180);
    display.println("Low Power");
    display.setCursor(50, 210);
    display.println("Mode Test");
    display.display();
    
    // Switch back to Low Power Mode
    display.setLowPowerMode();
    delay(2000);
}

void drawBitmap() {
    display.clearDisplay();
    display.setTextSize(3);
    display.setTextColor(ST7305_BLACK);
    display.setCursor(70, 180);
    display.println("READY!");
    display.drawRect(0, 0, ST7305_WIDTH - 1, ST7305_HEIGHT - 1, ST7305_BLACK);
    display.display();
}

// =======================================================
// --- Setup Function ---
// =======================================================
void setup() {
    Serial.begin(115200);
    delay(300);
    
    Serial.println("ST7305 Monochrome Display Test");
    Serial.println("300x400 pixels");
    Serial.println();
    
    // Initialize display
    Serial.print("Initializing display... ");
    if (!display.begin(1000000)) {  // 40 MHz SPI
        Serial.println("FAILED!");
        Serial.println("ERROR: Could not allocate frame buffer");
        while (1) delay(1000);
    }
    Serial.println("OK");
    
    Serial.print("Display size: ");
    Serial.print(display.width());
    Serial.print(" x ");
    Serial.println(display.height());
    
    Serial.print("Buffer size: ");
    Serial.print(ST7305_BUFFER_SIZE);
    Serial.println(" bytes");
    Serial.println();
    
    // Show initial message
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(ST7305_BLACK);
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
    Serial.println("Test: Single pixel at center");
    testSinglePixel();
    delay(5000);
    
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
    
    Serial.println("Test 5: Power mode switching");
    testPowerModes();
    delay(1000);
    
    Serial.println("Test 6: Scrolling text");
    testScrollText();
    delay(1000);
    
    Serial.println("Test 7: Ready screen");
    drawBitmap();
    delay(3000);
    
    Serial.println("--- Tests complete, restarting ---\n");
    delay(1000);
}