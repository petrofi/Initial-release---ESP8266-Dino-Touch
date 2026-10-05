/*
 * ESP8266 Dino Run
 * Board  : NodeMCU ESP8266
 * Display: 1.3" SH1106 OLED 128x64
 * Input  : TTP223 Capacitive Touch Sensor
 *
 * OLED:
 * SDA -> D2 / GPIO4
 * SCL -> D1 / GPIO5
 *
 * TTP223:
 * OUT -> D5 / GPIO14
 */

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// ----------------------------------------------------
// OLED
// ----------------------------------------------------

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C

#define OLED_SDA D2
#define OLED_SCL D1

Adafruit_SH1106G display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  -1
);

// ----------------------------------------------------
// TTP223 TOUCH SENSOR
// ----------------------------------------------------

#define TOUCH_PIN D5

// TTP223 normal configuration:
// Not touched = LOW
// Touched     = HIGH

bool lastTouchState = LOW;

// ----------------------------------------------------
// DINO SPRITES
// ----------------------------------------------------

const unsigned char PROGMEM dino_run1[] = {
  0x01, 0xfe,
  0x01, 0xfe,
  0x01, 0xfe,
  0x01, 0xfe,
  0x01, 0xf8,
  0x03, 0xf0,
  0x3f, 0xe0,
  0x7f, 0xe0,
  0xff, 0xe0,
  0xff, 0xe0,
  0x7f, 0xc0,
  0x3f, 0x80,
  0x1e, 0x00,
  0x18, 0x00,
  0x10, 0x00,
  0x10, 0x00
};

const unsigned char PROGMEM dino_run2[] = {
  0x01, 0xfe,
  0x01, 0xfe,
  0x01, 0xfe,
  0x01, 0xfe,
  0x01, 0xf8,
  0x03, 0xf0,
  0x3f, 0xe0,
  0x7f, 0xe0,
  0xff, 0xe0,
  0xff, 0xe0,
  0x7f, 0xc0,
  0x3f, 0x80,
  0x1a, 0x00,
  0x0a, 0x00,
  0x02, 0x00,
  0x02, 0x00
};

const unsigned char PROGMEM cactus_sprite[] = {
  0x00, 0x00,
  0x00, 0x00,
  0x02, 0x00,
  0x03, 0x00,
  0x23, 0x00,
  0x33, 0x00,
  0x33, 0x18,
  0x3b, 0x38,
  0x3f, 0x38,
  0x3f, 0x38,
  0x1f, 0xf8,
  0x0f, 0xf0,
  0x07, 0x00,
  0x07, 0x00,
  0x07, 0x00,
  0x07, 0x00
};

// ----------------------------------------------------
// DINO PHYSICS
// ----------------------------------------------------

float dinoY = 48.0;
float velocityY = 0.0;

float gravity = 1.3;
float jumpPower = -9.5;

bool isGrounded = true;

// ----------------------------------------------------
// CACTUS
// ----------------------------------------------------

float cactusX = 128.0;
float cactusSpeed = 4.0;

int obstacleType = 1;
int cactusWidth = 16;

// ----------------------------------------------------
// GAME
// ----------------------------------------------------

int score = 0;

bool gameOver = false;
bool useFrame1 = true;

// ----------------------------------------------------
// ANIMATION
// ----------------------------------------------------

unsigned long previousMillis = 0;

const unsigned long animationInterval = 100;

// ----------------------------------------------------
// SETUP
// ----------------------------------------------------

void setup() {

  // TTP223 has its own output circuitry.
  // Do NOT use INPUT_PULLUP.
  pinMode(TOUCH_PIN, INPUT);

  // ESP8266 I2C
  Wire.begin(OLED_SDA, OLED_SCL);

  // Random seed
  randomSeed(analogRead(A0) + micros());

  // OLED
  if (!display.begin(OLED_ADDRESS, true)) {
    while (true) {
      delay(100);
    }
  }

  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  showIntro();

  // Prevent the intro touch from becoming a jump
  lastTouchState = digitalRead(TOUCH_PIN);
}

// ----------------------------------------------------
// LOOP
// ----------------------------------------------------

void loop() {

  // --------------------------------------------------
  // GAME OVER
  // --------------------------------------------------

  if (gameOver) {

    showGameOver();

    bool touchState = digitalRead(TOUCH_PIN);

    // New touch detected
    if (touchState == HIGH && lastTouchState == LOW) {

      resetGame();

      // Wait for finger release
      while (digitalRead(TOUCH_PIN) == HIGH) {
        delay(5);
        yield();
      }

      lastTouchState = LOW;
    }
    else {
      lastTouchState = touchState;
    }

    delay(10);
    return;
  }

  // --------------------------------------------------
  // TOUCH INPUT
  // --------------------------------------------------

  bool currentTouchState = digitalRead(TOUCH_PIN);

  // Rising edge:
  // LOW -> HIGH = new touch
  if (
    currentTouchState == HIGH &&
    lastTouchState == LOW &&
    isGrounded
  ) {

    velocityY = jumpPower;
    isGrounded = false;
  }

  lastTouchState = currentTouchState;

  // --------------------------------------------------
  // DINO PHYSICS
  // --------------------------------------------------

  velocityY += gravity;
  dinoY += velocityY;

  // Floor
  if (dinoY >= 48) {

    dinoY = 48;
    velocityY = 0;
    isGrounded = true;
  }

  // --------------------------------------------------
  // MOVE CACTUS
  // --------------------------------------------------

  cactusX -= cactusSpeed;

  // Passed obstacle
  if (cactusX < -cactusWidth) {

    cactusX = 128;

    score++;

    // Increase difficulty
    if (
      score % 5 == 0 &&
      cactusSpeed < 8.0
    ) {

      cactusSpeed += 0.5;
    }

    // Single or double cactus
    obstacleType = random(1, 3);

    if (obstacleType == 1) {
      cactusWidth = 16;
    }
    else {
      cactusWidth = 28;
    }
  }

  // --------------------------------------------------
  // COLLISION DETECTION
  // --------------------------------------------------

  int dinoX = 10;
  int dinoWidth = 16;
  int dinoHeight = 16;

  int cactusY = 48;
  int cactusHeight = 16;

  int hitMargin = 3;

  bool collisionX =
    dinoX + hitMargin <
      cactusX + cactusWidth - hitMargin &&
    dinoX + dinoWidth - hitMargin >
      cactusX + hitMargin;

  bool collisionY =
    (int)dinoY + hitMargin <
      cactusY + cactusHeight - hitMargin &&
    (int)dinoY + dinoHeight - hitMargin >
      cactusY + hitMargin;

  if (collisionX && collisionY) {

    gameOver = true;

    return;
  }

  // --------------------------------------------------
  // DINO LEG ANIMATION
  // --------------------------------------------------

  unsigned long currentMillis = millis();

  if (
    currentMillis - previousMillis >=
    animationInterval
  ) {

    previousMillis = currentMillis;
    useFrame1 = !useFrame1;
  }

  // --------------------------------------------------
  // DRAW
  // --------------------------------------------------

  display.clearDisplay();

  // Ground
  display.drawLine(
    0,
    63,
    127,
    63,
    SH110X_WHITE
  );

  // --------------------------------------------------
  // DINO
  // --------------------------------------------------

  if (!isGrounded) {

    display.drawBitmap(
      10,
      (int)dinoY,
      dino_run1,
      16,
      16,
      SH110X_WHITE
    );
  }

  else if (useFrame1) {

    display.drawBitmap(
      10,
      (int)dinoY,
      dino_run1,
      16,
      16,
      SH110X_WHITE
    );
  }

  else {

    display.drawBitmap(
      10,
      (int)dinoY,
      dino_run2,
      16,
      16,
      SH110X_WHITE
    );
  }

  // --------------------------------------------------
  // CACTUS
  // --------------------------------------------------

  for (int i = 0; i < obstacleType; i++) {

    display.drawBitmap(
      (int)cactusX + (i * 12),
      48,
      cactus_sprite,
      16,
      16,
      SH110X_WHITE
    );
  }

  // --------------------------------------------------
  // SCORE
  // --------------------------------------------------

  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  display.setCursor(68, 2);

  display.print("S:");
  display.print(score);

  // --------------------------------------------------

  display.display();

  // Around 60 FPS
  delay(15);

  yield();
}

// ----------------------------------------------------
// INTRO
// ----------------------------------------------------

void showIntro() {

  display.clearDisplay();

  display.setTextColor(SH110X_WHITE);

  display.setTextSize(2);

  display.setCursor(17, 8);
  display.print("DINO RUN");

  display.setTextSize(1);

  display.setCursor(18, 38);
  display.print("Touch to Start");

  display.setCursor(25, 52);
  display.print("ESP8266");

  display.display();

  // Wait for touch
  while (digitalRead(TOUCH_PIN) == LOW) {
    delay(5);
    yield();
  }

  // Wait until finger is removed
  while (digitalRead(TOUCH_PIN) == HIGH) {
    delay(5);
    yield();
  }

  delay(100);
}

// ----------------------------------------------------
// GAME OVER SCREEN
// ----------------------------------------------------

void showGameOver() {

  display.clearDisplay();

  display.setTextColor(SH110X_WHITE);

  display.setTextSize(2);

  display.setCursor(10, 7);
  display.print("GAME OVER");

  display.setTextSize(1);

  display.setCursor(39, 30);
  display.print("Score:");
  display.print(score);

  display.setCursor(17, 48);
  display.print("Touch to Retry");

  display.display();
}

// ----------------------------------------------------
// RESET
// ----------------------------------------------------

void resetGame() {

  dinoY = 48;
  velocityY = 0;

  isGrounded = true;

  cactusX = 128;
  cactusSpeed = 4.0;

  obstacleType = 1;
  cactusWidth = 16;

  score = 0;

  gameOver = false;

  useFrame1 = true;

  previousMillis = millis();
}
