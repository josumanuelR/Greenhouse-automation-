#include <Wire.h>
#include <U8g2lib.h>
#include <Adafruit_NeoPixel.h>

// =====================================================
// OLED SH1106 - U8G2
// =====================================================
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

// =====================================================
// ENCODER
// =====================================================
#define ENCODER_CLK 19
#define ENCODER_DT  23

volatile int encoderCount = 0;
volatile bool encoderMoved = false;

// =====================================================
// BUTTONS
// =====================================================
#define MENU_BUTTON     5
#define CONFIRM_BUTTON  18
#define BACK_BUTTON     16

volatile bool menuPressed = false;
volatile bool confirmPressed = false;
volatile bool backPressed = false;

// =====================================================
// INTERRUPT DEBOUNCE
// =====================================================
volatile unsigned long lastEncoderInterrupt = 0;
volatile unsigned long lastMenuInterrupt = 0;
volatile unsigned long lastConfirmInterrupt = 0;
volatile unsigned long lastBackInterrupt = 0;

// =====================================================
// NEOPIXEL
// =====================================================
#define LED_PIN     4
#define NUMPIXELS   1

Adafruit_NeoPixel pixel(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);

// =====================================================
// DASHBOARD VALUES
// =====================================================
int fanPower = 70;
int lightPower = 50;

int temperature = 24;
int humidity = 60;

int changeTime = 10;

// =====================================================
// STATUS BAR
// =====================================================
bool autoMode = true;

int currentHour = 14;
int currentMinute = 35;

// =====================================================
// UI STATES
// =====================================================
enum ScreenState {
  DASHBOARD,
  MAIN_MENU,
  ADVANCED_MENU,
  EDIT_VALUE
};

ScreenState currentScreen = DASHBOARD;

// =====================================================
// MENU INDEXES
// =====================================================
int dashboardIndex = 0;
int mainMenuIndex = 0;
int advancedMenuIndex = 0;

// =====================================================
// EDIT POINTER
// =====================================================
int* currentEditValue = nullptr;

// =====================================================
// MENU ITEMS
// =====================================================
const char* mainMenuItems[] = {
  "Change Temp",
  "Change Humidity",
  "Advanced Options"
};

const char* advancedMenuItems[] = {
  "Change Time"
};

// =====================================================
// ANIMATED 12x12 BITMAP FRAMES
// =====================================================

// Frame 1
const unsigned char frame1[] PROGMEM = {

  0x07, 0x80,
  0x1F, 0xC0,
  0x1F, 0xC0,
  0x0E, 0x00,
  0x00, 0x00,
  0x66, 0xC0,
  0x66, 0xE0,
  0x70, 0xF0,
  0x7C, 0x70,
  0x7C, 0x70,
  0x38, 0x60,
  0x00, 0x00
};

// Frame 2
const unsigned char frame2[] PROGMEM = {

  0x00, 0x00,
  0x01, 0xE0,
  0x63, 0xF0,
  0xE3, 0xF0,
  0xE0, 0x70,
  0xE6, 0x30,
  0xF6, 0x00,
  0xF0, 0xC0,
  0x70, 0xC0,
  0x31, 0xC0,
  0x07, 0xC0,
  0x07, 0x80
};

// Frame 3
const unsigned char frame3[] PROGMEM = {

  0x00, 0x00,
  0x3C, 0x00,
  0x7C, 0xE0,
  0xF8, 0xF0,
  0xE0, 0xF0,
  0xE6, 0x70,
  0x66, 0x70,
  0x00, 0x30,
  0x39, 0x80,
  0x7F, 0x80,
  0x3F, 0x00,
  0x0E, 0x00
};

// =====================================================
// BITMAP OBJECT ARRAY
// =====================================================

const unsigned char* animationFrames[] = {
  frame1,
  frame2,
  frame3
};

// =====================================================
// ANIMATION VARIABLES
// =====================================================

int currentFrame = 0;

unsigned long lastFrameChange = 0;

const int frameInterval = 100;

// =====================================================
// 12x12 BITMAPS
// =====================================================

// Light Icon
const unsigned char lightBitmap[] PROGMEM = {
  0x00, 0x00,
  0x40, 0x20,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x06, 0x00,
  0x06, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x40, 0x20,
  0x00, 0x00,
};

// Temperature Icon
const unsigned char temperatureBitmap[] PROGMEM = {
  0x00, 0x00,
  0x40, 0x20,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x06, 0x00,
  0x06, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x00, 0x00,
  0x40, 0x20,
  0x00, 0x00,
};

// Humidity Icon
const unsigned char humidityBitmap[] PROGMEM = {
  0xFF, 0xF0,
  0xFF, 0xF0,
  0xFF, 0xF0,
  0xFF, 0xF0,
  0xFF, 0xF0,
  0xFF, 0xF0,
  0xFF, 0xF0,
  0xFF, 0xF0,
  0xFF, 0xF0,
  0xFF, 0xF0,
  0xFF, 0xF0,
  0xFF, 0xF0
};


// =====================================================
// ENCODER INTERRUPT
// =====================================================
void IRAM_ATTR handleEncoderInterrupt() {

  unsigned long currentTime = micros();

  if (currentTime - lastEncoderInterrupt > 2000) {

    if (digitalRead(ENCODER_DT) == HIGH) {
      encoderCount++;
    }
    else {
      encoderCount--;
    }

    encoderMoved = true;

    lastEncoderInterrupt = currentTime;
  }
}

// =====================================================
// MENU BUTTON INTERRUPT
// =====================================================
void IRAM_ATTR menuButtonInterrupt() {

  unsigned long currentTime = micros();

  if (currentTime - lastMenuInterrupt > 200000) {

    menuPressed = true;

    lastMenuInterrupt = currentTime;
  }
}

// =====================================================
// CONFIRM BUTTON INTERRUPT
// =====================================================
void IRAM_ATTR confirmButtonInterrupt() {

  unsigned long currentTime = micros();

  if (currentTime - lastConfirmInterrupt > 200000) {

    confirmPressed = true;

    lastConfirmInterrupt = currentTime;
  }
}

// =====================================================
// BACK BUTTON INTERRUPT
// =====================================================
void IRAM_ATTR backButtonInterrupt() {

  unsigned long currentTime = micros();

  if (currentTime - lastBackInterrupt > 200000) {

    backPressed = true;

    lastBackInterrupt = currentTime;
  }
}

// =====================================================
// SETUP
// =====================================================
void setup() {

  Serial.begin(115200);

  // OLED
  u8g2.begin();

  // Encoder Pins
  pinMode(ENCODER_CLK, INPUT_PULLUP);
  pinMode(ENCODER_DT, INPUT_PULLUP);

  // Buttons
  pinMode(MENU_BUTTON, INPUT_PULLUP);
  pinMode(CONFIRM_BUTTON, INPUT_PULLUP);
  pinMode(BACK_BUTTON, INPUT_PULLUP);

  // =====================================================
  // INTERRUPTS
  // =====================================================

  attachInterrupt(
    digitalPinToInterrupt(ENCODER_CLK),
    handleEncoderInterrupt,
    RISING
  );

  attachInterrupt(
    digitalPinToInterrupt(MENU_BUTTON),
    menuButtonInterrupt,
    FALLING
  );

  attachInterrupt(
    digitalPinToInterrupt(CONFIRM_BUTTON),
    confirmButtonInterrupt,
    FALLING
  );

  attachInterrupt(
    digitalPinToInterrupt(BACK_BUTTON),
    backButtonInterrupt,
    FALLING
  );

  // NeoPixel
  pixel.begin();
  pixel.setBrightness(50);

  showStartupScreen();
}

// =====================================================
// LOOP
// =====================================================
void loop() {

  handleEncoder();
  handleButtons();
  drawCurrentScreen();
  updateBitmapAnimation();

  delay(10);
}

// =====================================================
// HANDLE ENCODER
// =====================================================
void handleEncoder() {

  static int lastEncoder = 0;

  if (!encoderMoved) return;

  encoderMoved = false;

  int delta = encoderCount - lastEncoder;

  if (delta == 0) return;

  // =====================================================
  // DASHBOARD
  // =====================================================
  if (currentScreen == DASHBOARD) {

    dashboardIndex += delta;

    if (dashboardIndex > 1)
      dashboardIndex = 0;

    if (dashboardIndex < 0)
      dashboardIndex = 1;
  }

  // =====================================================
  // MAIN MENU
  // =====================================================
  else if (currentScreen == MAIN_MENU) {

    mainMenuIndex += delta;

    if (mainMenuIndex > 2)
      mainMenuIndex = 0;

    if (mainMenuIndex < 0)
      mainMenuIndex = 2;
  }

  // =====================================================
  // ADVANCED MENU
  // =====================================================
  else if (currentScreen == ADVANCED_MENU) {

    advancedMenuIndex += delta;

    if (advancedMenuIndex > 0)
      advancedMenuIndex = 0;

    if (advancedMenuIndex < 0)
      advancedMenuIndex = 0;
  }

  // =====================================================
  // EDIT VALUE
  // =====================================================
  else if (currentScreen == EDIT_VALUE) {

    if (currentEditValue != nullptr) {

      *currentEditValue += delta;

      if (*currentEditValue < 0)
        *currentEditValue = 0;

      if (*currentEditValue > 100)
        *currentEditValue = 100;
    }
  }

  lastEncoder = encoderCount;
}

// =====================================================
// HANDLE BUTTONS
// =====================================================
void handleButtons() {

  // =====================================================
  // MENU BUTTON
  // =====================================================
  if (menuPressed) {

    menuPressed = false;

    if (currentScreen == DASHBOARD) {
      currentScreen = MAIN_MENU;
    }
  }

  // =====================================================
  // CONFIRM BUTTON
  // =====================================================
  if (confirmPressed) {

    confirmPressed = false;

    // MAIN MENU
    if (currentScreen == MAIN_MENU) {

      switch (mainMenuIndex) {

        case 0:
          currentEditValue = &temperature;
          currentScreen = EDIT_VALUE;
          break;

        case 1:
          currentEditValue = &humidity;
          currentScreen = EDIT_VALUE;
          break;

        case 2:
          currentScreen = ADVANCED_MENU;
          break;
      }
    }

    // ADVANCED MENU
    else if (currentScreen == ADVANCED_MENU) {

      currentEditValue = &changeTime;
      currentScreen = EDIT_VALUE;
    }

    // SAVE VALUE
    else if (currentScreen == EDIT_VALUE) {

      currentScreen = DASHBOARD;
    }
  }

  // =====================================================
  // BACK BUTTON
  // =====================================================
  if (backPressed) {

    backPressed = false;

    if (currentScreen == MAIN_MENU) {

      currentScreen = DASHBOARD;
    }

    else if (currentScreen == ADVANCED_MENU) {

      currentScreen = MAIN_MENU;
    }

    else if (currentScreen == EDIT_VALUE) {

      if (currentEditValue == &changeTime) {

        currentScreen = ADVANCED_MENU;
      }
      else {

        currentScreen = MAIN_MENU;
      }
    }
  }
}

// =====================================================
// DRAW CURRENT SCREEN
// =====================================================
void drawCurrentScreen() {

  u8g2.clearBuffer();

  switch (currentScreen) {

    case DASHBOARD:
      drawDashboard();
      break;

    case MAIN_MENU:
      drawMainMenu();
      break;

    case ADVANCED_MENU:
      drawAdvancedMenu();
      break;

    case EDIT_VALUE:
      drawEditScreen();
      break;
  }

  u8g2.sendBuffer();
}

// =====================================================
// DRAW STATUS BAR
// =====================================================
void drawStatusBar() {

  // White top bar
  u8g2.drawBox(0, 0, 128, 12);

  // Black text
  u8g2.setDrawColor(0);

  u8g2.setFont(u8g2_font_5x8_tf);

  // AUTO / MANUAL
  if (autoMode) {
    u8g2.drawStr(3, 9, "AUTO");
  }
  else {
    u8g2.drawStr(3, 9, "MANUAL");
  }

  // TIME
  char timeBuffer[10];

  sprintf(
    timeBuffer,
    "%02d:%02d",
    currentHour,
    currentMinute
  );

  u8g2.drawStr(90, 9, timeBuffer);

  // Restore color
  u8g2.setDrawColor(1);
}

// =====================================================
// DASHBOARD
// =====================================================
void drawDashboard() {

  drawStatusBar();

  u8g2.setFont(u8g2_font_6x12_tf);

  // =====================================================
  // DASHBOARD 1
  // =====================================================
  if (dashboardIndex == 0) {

    // Fan icon
    u8g2.drawBitmap(
      0,
      21,
      2,   // width in bytes
      12,  // height
      animationFrames[currentFrame]
    );


    u8g2.drawStr(15, 30, "Fan Power:");
    u8g2.setCursor(95, 30);
    u8g2.print(fanPower);
    u8g2.print("%");

    // Light icon
    u8g2.drawBitmap(
      0,
      43,
      2,   // width in bytes
      12,  // height
      lightBitmap
    );

    u8g2.drawStr(15, 52, "Lamp Power:");
    u8g2.setCursor(95, 52);
    u8g2.print(lightPower);
    u8g2.print("%");
  }

  // =====================================================
  // DASHBOARD 2
  // =====================================================
  else {

    u8g2.drawStr(0, 30, "Temperature:");
    u8g2.setCursor(95, 30);
    u8g2.print(temperature);
    u8g2.print("C");

    u8g2.drawStr(0, 52, "Humidity:");
    u8g2.setCursor(95, 52);
    u8g2.print(humidity);
    u8g2.print("%");
  }
}


// =====================================================
// UPDATE BITMAP ANIMATION
// =====================================================
void updateBitmapAnimation() {

  if (millis() - lastFrameChange >= frameInterval) {

    currentFrame++;

    if (currentFrame > 2) {
      currentFrame = 0;
    }

    lastFrameChange = millis();
  }
}

// =====================================================
// MAIN MENU
// =====================================================
void drawMainMenu() {

  u8g2.setFont(u8g2_font_6x12_tf);

  u8g2.drawStr(0, 12, "MAIN MENU");

  for (int i = 0; i < 3; i++) {

    if (i == mainMenuIndex) {

      u8g2.drawBox(0, 18 + (i * 14), 128, 12);

      u8g2.setDrawColor(0);
    }

    u8g2.drawStr(5, 28 + (i * 14), mainMenuItems[i]);

    u8g2.setDrawColor(1);
  }
}

// =====================================================
// ADVANCED MENU
// =====================================================
void drawAdvancedMenu() {

  u8g2.setFont(u8g2_font_6x12_tf);

  u8g2.drawStr(0, 12, "ADVANCED");

  if (advancedMenuIndex == 0) {

    u8g2.drawBox(0, 22, 128, 12);

    u8g2.setDrawColor(0);

    u8g2.drawStr(5, 32, advancedMenuItems[0]);

    u8g2.setDrawColor(1);
  }
}

// =====================================================
// EDIT SCREEN
// =====================================================
void drawEditScreen() {

  u8g2.setFont(u8g2_font_6x12_tf);

  u8g2.drawStr(0, 15, "EDIT VALUE");

  u8g2.setFont(u8g2_font_inb24_mn);

  u8g2.setCursor(35, 55);

  if (currentEditValue != nullptr) {

    u8g2.print(*currentEditValue);
  }
}

// =====================================================
// STARTUP SCREEN
// =====================================================
void showStartupScreen() {

  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_logisoso16_tf);

  u8g2.drawStr(10, 35, "SYSTEM");

  u8g2.sendBuffer();

  delay(5000);
}
