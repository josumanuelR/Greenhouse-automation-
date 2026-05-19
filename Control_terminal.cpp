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
#define ENCODER_CLK 12
#define ENCODER_DT 4

// Variables
int counter = 0;
int lastCLK;

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

  // Read initial state
  lastCLK = digitalRead(ENCODER_CLK);

  // Clear display
  display.clearDisplay();
  display.display();
}

void loop() {

  // ==========================
  // Read Encoder
  // ==========================
  int currentCLK = digitalRead(ENCODER_CLK);

  // Detect rotation
  if (currentCLK != lastCLK && currentCLK == LOW) {

    // Determine direction
    if (digitalRead(ENCODER_DT) != currentCLK) {
      counter++;
    } else {
      counter--;
    }

    // Limit from 0 to 9
    if (counter > 9) counter = 0;
    if (counter < 0) counter = 9;

    Serial.println(counter);
  }

  lastCLK = currentCLK;

  // ==========================
  // Display Counter
  // ==========================
  display.clearDisplay();

  display.setTextSize(6);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(45, 10);

  display.println(counter);

  display.display();

  delay(1);
}
