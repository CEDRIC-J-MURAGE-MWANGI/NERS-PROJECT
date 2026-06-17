#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// ── Motor pins ───────────────────────────────────────────
const uint8_t leftMotor[4]    = {8, 9, 10, 11};
const uint8_t rightMotor[4]   = {4, 5,  6,  7};
const uint8_t suctionMotor[4] = {2, 3, 12, 13};

// ── Buttons & LED ────────────────────────────────────────
#define BTN1    A0
#define BTN2    A1
#define BTN3    A2
#define RED_LED A3

// ── Mode flags ───────────────────────────────────────────
bool warmingOn = false;
bool suctionOn = false;
bool massageOn = false;

// ── Stepper half-step sequence ───────────────────────────
const uint8_t stepSeq[8] = {
  0b0001, 0b0011, 0b0010, 0b0110,
  0b0100, 0b1100, 0b1000, 0b1001
};
uint8_t stepIdx[3] = {0, 0, 0}; // [0]=left [1]=right [2]=suction

// ── Step timing ──────────────────────────────────────────
// 28BYJ-48 needs ~2-15ms per step to physically move
// 8ms gives smooth rotation with good torque — tune if needed
#define MASSAGE_STEP_MS  1    // chest motors step interval
#define SUCTION_STEP_MS  1    // suction motor step interval
unsigned long lastMassageStep = 0;
unsigned long lastSuctionStep = 0;

// ── Simulated temperature ────────────────────────────────
float simTemp = 36.8;
unsigned long lastTempTick = 0;

// ── Debounce ─────────────────────────────────────────────
#define DEBOUNCE_MS 300
unsigned long lastPress[3] = {0, 0, 0};

// ── Motor helpers ────────────────────────────────────────
void stepMotor(const uint8_t pins[4], uint8_t &idx) {
  uint8_t s = stepSeq[idx];
  for (uint8_t i = 0; i < 4; i++)
    digitalWrite(pins[i], (s >> i) & 1);
  idx = (idx + 1) % 8;
}

void stopMotor(const uint8_t pins[4]) {
  for (uint8_t i = 0; i < 4; i++)
    digitalWrite(pins[i], LOW);
}

// ── Button helper ────────────────────────────────────────
bool btnPressed(uint8_t pin, uint8_t idx) {
  if (digitalRead(pin) == LOW) {
    unsigned long now = millis();
    if (now - lastPress[idx] > DEBOUNCE_MS) {
      lastPress[idx] = now;
      while (digitalRead(pin) == LOW); // wait for release
      delay(20);
      return true;
    }
  }
  return false;
}

// ── Draw home screen ─────────────────────────────────────
void drawHome() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  if (warmingOn) {
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print(F("Temp: "));
    display.print(simTemp, 1);
    display.print(F("C "));
    if      (simTemp < 36.0) display.print(F("! LOW"));
    else if (simTemp > 37.5) display.print(F("! HIGH"));
    else                     display.print(F("OK"));
    display.drawLine(0, 11, 127, 11, SSD1306_WHITE);
  }

  uint8_t y = warmingOn ? 14 : 2;
  display.setTextSize(1);

  display.setCursor(0, y);
  display.print(warmingOn ? F("> [WARMING ON]  ") : F("  1: Warming    "));

  display.setCursor(0, y + 12);
  display.print(suctionOn ? F("> [SUCTION ON]  ") : F("  2: Suction    "));

  display.setCursor(0, y + 24);
  display.print(massageOn ? F("> [MASSAGE ON]  ") : F("  3: Massage    "));

  uint8_t divY = warmingOn ? 54 : 44;
  display.drawLine(0, divY, 127, divY, SSD1306_WHITE);
  display.setCursor(0, divY + 3);
  display.print(F("B1=Warm B2=Suct B3=Msg"));

  display.display();
}

// ── Setup ────────────────────────────────────────────────
void setup() {
  // Init motor pins LOW
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(leftMotor[i],    OUTPUT); digitalWrite(leftMotor[i],    LOW);
    pinMode(rightMotor[i],   OUTPUT); digitalWrite(rightMotor[i],   LOW);
    pinMode(suctionMotor[i], OUTPUT); digitalWrite(suctionMotor[i], LOW);
  }

  // Buttons with pullup
  pinMode(BTN1, INPUT_PULLUP);
  pinMode(BTN2, INPUT_PULLUP);
  pinMode(BTN3, INPUT_PULLUP);
  delay(100); // let pullups charge

  // LED
  pinMode(RED_LED, OUTPUT);
  digitalWrite(RED_LED, LOW);

  // OLED init
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();

  // Splash screen
  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(15, 14);
  display.print(F("THE NERS"));
  display.setTextSize(1);
  display.setCursor(12, 40);
  display.print(F("Neonatal Response"));
  display.setCursor(22, 52);
  display.print(F("Swaddle v1.0"));
  display.display();
  delay(5000);

  // Seed timers AFTER splash
  unsigned long t = millis();
  lastPress[0] = lastPress[1] = lastPress[2] = t;
  lastTempTick      = t;
  lastMassageStep   = t;
  lastSuctionStep   = t;

  drawHome();
}

// ── Loop ─────────────────────────────────────────────────
void loop() {
  unsigned long now = millis();

  // ── Button 1: Warming toggle ──────────────────────────
  if (btnPressed(BTN1, 0)) {
    warmingOn = !warmingOn;
    digitalWrite(RED_LED, warmingOn ? HIGH : LOW);
    if (warmingOn) {
      display.clearDisplay();
      display.setTextSize(2);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(8, 10);
      display.print(F("Baby"));
      display.setCursor(8, 34);
      display.print(F("Warming"));
      display.display();
      delay(3000);
      lastTempTick = millis();
    }
    drawHome();
  }

  // ── Button 2: Suction toggle ──────────────────────────
  if (btnPressed(BTN2, 1)) {
    suctionOn = !suctionOn;
    if (!suctionOn) stopMotor(suctionMotor);
    drawHome();
  }

  // ── Button 3: Massage toggle ──────────────────────────
  if (btnPressed(BTN3, 2)) {
    massageOn = !massageOn;
    if (!massageOn) {
      stopMotor(leftMotor);
      stopMotor(rightMotor);
    }
    drawHome();
  }

  // ── Timed motor stepping ──────────────────────────────

  // Suction motor — step every SUCTION_STEP_MS
  if (suctionOn && now - lastSuctionStep >= SUCTION_STEP_MS) {
    lastSuctionStep = now;
    stepMotor(suctionMotor, stepIdx[2]);
  }

  // Chest massage motors — step every MASSAGE_STEP_MS
  if (massageOn && now - lastMassageStep >= MASSAGE_STEP_MS) {
    lastMassageStep = now;
    stepMotor(leftMotor,  stepIdx[0]);
    stepMotor(rightMotor, stepIdx[1]);
  }

  // ── Temperature simulation every 4s ──────────────────
  if (warmingOn && now - lastTempTick > 4000) {
    lastTempTick = now;
    simTemp += (random(-3, 4)) * 0.1;
    simTemp = constrain(simTemp, 35.5, 38.2);
    drawHome();
  }
}
