#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// ==========================
// OLED SH1106 Configuration
// ==========================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ==========================
// EC11 Encoder Pins
// ==========================
#define ENCODER_CLK 19
#define ENCODER_DT  23

// ==========================
// Variables
// ==========================
volatile int counter = 0;

// Debounce timer
volatile unsigned long lastInterruptTime = 0;

// ==========================
// Encoder Interrupt
// ==========================
void IRAM_ATTR readEncoder() {

  unsigned long currentTime = micros();

  // Debounce (adjust if needed)
  if (currentTime - lastInterruptTime > 3000) {

    // Read encoder direction
    if (digitalRead(ENCODER_DT) == HIGH) {
      counter++;
    } else {
      counter--;
    }

    // Keep value between 0 and 9
    if (counter > 99) counter = 0;
    if (counter < 0) counter = 99;

    lastInterruptTime = currentTime;
  }
}

void setup() {
  Serial.begin(115200);

  // I2C Init
  Wire.begin();

  // OLED Init
  if (!display.begin(OLED_ADDRESS, true)) {
    Serial.println("SH1106 initialization failed");
    while (true);
  }

  // Encoder pins
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);

  // Interrupt on CLK rising edge
  attachInterrupt(
    digitalPinToInterrupt(ENCODER_CLK),
    readEncoder,
    RISING
  );

  // Clear display
  display.clearDisplay();
  display.display();
}

void loop() {

  static int lastCounter = -1;

  // Update only if value changes
  if (counter != lastCounter) {

    Serial.println(counter);

    display.clearDisplay();

    display.setTextSize(6);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(30, 10);

    display.println(counter);

    display.display();

    lastCounter = counter;
  }

  delay(1);
}
