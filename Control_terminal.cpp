#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Adafruit_NeoPixel.h>

// ==========================
// OLED SH1106 Configuration
// ==========================
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ==========================
// NeoPixel Configuration
// ==========================
#define LED_PIN     16
#define NUMPIXELS   1

Adafruit_NeoPixel pixel(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// ==========================
// Button Pins
// ==========================
#define NEXT_BUTTON  26
#define PREV_BUTTON  18


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

// 12 HEX Colors
uint32_t colors[12];

int colorIndex = 0;

// Button states
bool lastNextState = HIGH;
bool lastPrevState = HIGH;

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

  // Interrupt on CLK rising edge
  attachInterrupt(
    digitalPinToInterrupt(ENCODER_CLK),
    readEncoder,
    RISING
  );

  // OLED Init
  if (!display.begin(OLED_ADDRESS, true)) {
    Serial.println("SH1106 initialization failed");
    while (true);
  }

  // Encoder pins
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);

  // Clear display
  display.clearDisplay();
  display.display();

  // Initialize NeoPixel
  pixel.begin();
  pixel.setBrightness(50);

  // Buttons
  pinMode(NEXT_BUTTON, INPUT_PULLUP);
  pinMode(PREV_BUTTON, INPUT_PULLUP);

  // Define colors
  colors[0]  = pixel.Color(0xFF, 0x00, 0x00); // #FF0000 Red
  colors[1]  = pixel.Color(0x00, 0xFF, 0x00); // #00FF00 Green
  colors[2]  = pixel.Color(0x00, 0x00, 0xFF); // #0000FF Blue
  colors[3]  = pixel.Color(0xFF, 0xFF, 0x00); // #FFFF00 Yellow
  colors[4]  = pixel.Color(0xFF, 0x00, 0xFF); // #FF00FF Magenta
  colors[5]  = pixel.Color(0x00, 0xFF, 0xFF); // #00FFFF Cyan
  colors[6]  = pixel.Color(0xFF, 0x80, 0x00); // #FF8000 Orange
  colors[7]  = pixel.Color(0x80, 0x00, 0xFF); // #8000FF Purple
  colors[8]  = pixel.Color(0xFF, 0xFF, 0xFF); // #FFFFFF White
  colors[9]  = pixel.Color(0x80, 0x80, 0x80); // #808080 Gray
  colors[10] = pixel.Color(0x00, 0x80, 0xFF); // #0080FF Light Blue
  colors[11] = pixel.Color(0x00, 0xFF, 0x80); // #00FF80 Mint

  updateColor();
}

void loop() {

  bool nextState = digitalRead(NEXT_BUTTON);
  bool prevState = digitalRead(PREV_BUTTON);

  // ==========================
  // NEXT COLOR BUTTON
  // ==========================
  if (lastNextState == HIGH && nextState == LOW) {

    colorIndex++;

    if (colorIndex > 11) {
      colorIndex = 0;
    }

    updateColor();

    delay(200); // debounce
  }

  // ==========================
  // PREVIOUS COLOR BUTTON
  // ==========================
  if (lastPrevState == HIGH && prevState == LOW) {

    colorIndex--;

    if (colorIndex < 0) {
      colorIndex = 11;
    }

    updateColor();

    delay(200); // debounce
  }

  lastNextState = nextState;
  lastPrevState = prevState;
  
  // ==========================
  // ENCODER COUNTING
  // ==========================
  
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

// ==========================
// Update LED Color
// ==========================
void updateColor() {

  pixel.setPixelColor(0, colors[colorIndex]);
  pixel.show();

  Serial.print("Color index: ");
  Serial.println(colorIndex);
}
