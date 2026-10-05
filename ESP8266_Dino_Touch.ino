/*
  ESP8266 Dino Touch
  Board : NodeMCU V3 ESP8266 12E CH340
  Display: 128x64 SH1106 I2C OLED
  Input  : TTP223 capacitive touch sensor
  Buzzer : Not used

  Connections
  -----------
  OLED GND -> GND
  OLED VCC -> 3V3
  OLED SCL -> D1 / GPIO5
  OLED SDA -> D2 / GPIO4

  TTP223 GND -> GND
  TTP223 VCC -> 3V3
  TTP223 OUT -> D5 / GPIO14
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

// ======================================================
// DISPLAY
// ======================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_ADDRESS 0x3C

Adafruit_SH1106G display(
  SCREEN_WIDTH,
  SCREEN_HEIGHT,
  &Wire,
  OLED_RESET
);

// ======================================================
// PIN
// ======================================================

#define TOUCH_PIN D5

// ======================================================
// GAME SETTINGS
// ======================================================

const int GROUND_Y = 54;

const int DINO_X = 13;
const int DINO_W = 12;
const int DINO_H = 15;

const int CACTUS_W = 7;
const int CACTUS_H = 16;

float dinoY;
float dinoVelocity = 0;

float gravity = 0.78;
float jumpPower = -8.4;

float cactusX = 135;
float gameSpeed = 2.6;

unsigned long score = 0;
unsigned long highScore = 0;

unsigned long lastFrameTime = 0;
const unsigned long FRAME_TIME = 33;

// Touch debounce
bool oldTouchState = LOW;
unsigned long lastTouchTime = 0;
const unsigned long TOUCH_DEBOUNCE = 120;

enum GameState {
  START_SCREEN,
  PLAYING,
  GAME_OVER
};

GameState gameState = START_SCREEN;

// ======================================================
// DINO
// ======================================================

void drawDino(int x, int y, bool running) {
  display.fillRect(x + 5, y, 7, 6, SH110X_WHITE);
  display.fillRect(x + 10, y + 4, 3, 3, SH110X_WHITE);
  display.drawPixel(x + 9, y + 2, SH110X_BLACK);

  display.fillRect(x + 3, y + 6, 7, 6, SH110X_WHITE);

  display.drawLine(x + 3, y + 7, x, y + 5, SH110X_WHITE);
  display.drawLine(x, y + 5, x + 2, y + 9, SH110X_WHITE);

  display.drawLine(x + 8, y + 8, x + 11, y + 9, SH110X_WHITE);

  if (running) {
    if ((millis() / 120) % 2 == 0) {
      display.drawLine(x + 4, y + 11, x + 3, y + 14, SH110X_WHITE);
      display.drawLine(x + 8, y + 11, x + 9, y + 14, SH110X_WHITE);
    } else {
      display.drawLine(x + 4, y + 11, x + 5, y + 14, SH110X_WHITE);
      display.drawLine(x + 8, y + 11, x + 7, y + 14, SH110X_WHITE);
    }
  } else {
    display.drawLine(x + 4, y + 11, x + 4, y + 14, SH110X_WHITE);
    display.drawLine(x + 8, y + 11, x + 8, y + 14, SH110X_WHITE);
  }
}

// ======================================================
// CACTUS
// ======================================================

void drawCactus(int x) {
  int y = GROUND_Y - CACTUS_H;

  display.fillRect(x + 2, y, 3, CACTUS_H, SH110X_WHITE);

  display.fillRect(x, y + 6, 2, 6, SH110X_WHITE);
  display.drawLine(x, y + 11, x + 3, y + 11, SH110X_WHITE);

  display.fillRect(x + 5, y + 4, 2, 6, SH110X_WHITE);
  display.drawLine(x + 4, y + 9, x + 6, y + 9, SH110X_WHITE);
}

// ======================================================
// GROUND
// ======================================================

void drawGround() {
  display.drawLine(0, GROUND_Y, 127, GROUND_Y, SH110X_WHITE);

  int offset = (millis() / 35) % 18;

  for (int x = -offset; x < 128; x += 18) {
    display.drawPixel(x, GROUND_Y + 4, SH110X_WHITE);

    if (x + 5 < 128) {
      display.drawPixel(x + 5, GROUND_Y + 6, SH110X_WHITE);
    }
  }
}

// ======================================================
// TOUCH
// ======================================================

bool newTouch() {
  bool currentTouch = digitalRead(TOUCH_PIN);
  bool pressed = false;

  if (
    currentTouch == HIGH &&
    oldTouchState == LOW &&
    millis() - lastTouchTime > TOUCH_DEBOUNCE
  ) {
    pressed = true;
    lastTouchTime = millis();
  }

  oldTouchState = currentTouch;
  return pressed;
}

// ======================================================
// COLLISION
// ======================================================

bool checkCollision() {
  int dinoLeft = DINO_X + 2;
  int dinoRight = DINO_X + DINO_W - 1;

  int dinoTop = (int)dinoY + 1;
  int dinoBottom = (int)dinoY + DINO_H;

  int cactusLeft = (int)cactusX + 1;
  int cactusRight = (int)cactusX + CACTUS_W;

  int cactusTop = GROUND_Y - CACTUS_H;
  int cactusBottom = GROUND_Y;

  return (
    dinoRight >= cactusLeft &&
    dinoLeft <= cactusRight &&
    dinoBottom >= cactusTop &&
    dinoTop <= cactusBottom
  );
}

// ======================================================
// NEW GAME
// ======================================================

void resetGame() {
  dinoY = GROUND_Y - DINO_H;
  dinoVelocity = 0;

  cactusX = 140 + random(10, 45);

  gameSpeed = 2.6;
  score = 0;
}

// ======================================================
// JUMP
// ======================================================

void jump() {
  float groundPosition = GROUND_Y - DINO_H;

  if (dinoY >= groundPosition - 1) {
    dinoVelocity = jumpPower;
  }
}

// ======================================================
// UPDATE GAME
// ======================================================

void updateGame() {
  dinoVelocity += gravity;
  dinoY += dinoVelocity;

  float groundPosition = GROUND_Y - DINO_H;

  if (dinoY >= groundPosition) {
    dinoY = groundPosition;
    dinoVelocity = 0;
  }

  cactusX -= gameSpeed;

  if (cactusX < -CACTUS_W) {
    cactusX = 128 + random(25, 80);
    score++;

    if (gameSpeed < 6.4) {
      gameSpeed += 0.10;
    }
  }

  if (checkCollision()) {
    gameState = GAME_OVER;

    if (score > highScore) {
      highScore = score;
    }
  }
}

// ======================================================
// START SCREEN
// ======================================================

void drawStartScreen() {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  display.setTextSize(2);
  display.setCursor(34, 4);
  display.print("DINO");

  display.setTextSize(1);
  display.setCursor(23, 24);
  display.print("Dokun ve basla");

  drawGround();

  drawDino(
    15,
    GROUND_Y - DINO_H,
    true
  );

  drawCactus(100);

  display.display();
}

// ======================================================
// GAME SCREEN
// ======================================================

void drawGame() {
  display.clearDisplay();

  drawGround();

  bool grounded =
    dinoY >=
    (GROUND_Y - DINO_H - 1);

  drawDino(
    DINO_X,
    (int)dinoY,
    grounded
  );

  drawCactus(
    (int)cactusX
  );

  display.setTextColor(SH110X_WHITE);
  display.setTextSize(1);

  display.setCursor(92, 2);
  display.print(score);

  display.display();
}

// ======================================================
// GAME OVER
// ======================================================

void drawGameOver() {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  display.setTextSize(2);
  display.setCursor(10, 5);
  display.print("GAME OVER");

  display.setTextSize(1);

  display.setCursor(34, 28);
  display.print("SKOR: ");
  display.print(score);

  display.setCursor(31, 39);
  display.print("REKOR: ");
  display.print(highScore);

  display.setCursor(15, 54);
  display.print("Tekrar dokun");

  display.display();
}

// ======================================================
// SETUP
// ======================================================

void setup() {
  Serial.begin(115200);

  pinMode(TOUCH_PIN, INPUT);

  // ESP8266 NodeMCU:
  // SDA = D2 / GPIO4
  // SCL = D1 / GPIO5
  Wire.begin(D2, D1);

  delay(250);

  if (!display.begin(OLED_ADDRESS, true)) {
    Serial.println("OLED bulunamadi!");

    while (true) {
      delay(100);
    }
  }

  display.clearDisplay();
  display.display();

  randomSeed(micros());

  resetGame();
  drawStartScreen();
}

// ======================================================
// LOOP
// ======================================================

void loop() {
  if (newTouch()) {
    if (gameState == START_SCREEN) {
      resetGame();
      gameState = PLAYING;
    }
    else if (gameState == PLAYING) {
      jump();
    }
    else if (gameState == GAME_OVER) {
      resetGame();
      gameState = PLAYING;
    }
  }

  if (millis() - lastFrameTime >= FRAME_TIME) {
    lastFrameTime = millis();

    if (gameState == START_SCREEN) {
      drawStartScreen();
    }
    else if (gameState == PLAYING) {
      updateGame();
      drawGame();
    }
    else if (gameState == GAME_OVER) {
      drawGameOver();
    }
  }

  yield();
}
