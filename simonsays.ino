
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Preferences.h>

// ---------------- PIN ASSIGNMENTS ----------------

const int LED_PINS[3] = {13, 14, 16};
const int BUTTON_PINS[3] = {10, 11, 12};

const int BUZZER_PIN = 15;

// OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
Preferences preferences;

// ---------------- GAME SETTINGS ----------------

const int MAX_SEQUENCE = 100;

int sequence[MAX_SEQUENCE];

int level = 0;
int bestScore = 0;

bool gameRunning = false;

// Different tones for the three colours
const int tones[3] = {
  440, 554, 659
};

// ---------------- OLED DISPLAY ----------------

void showMessage(String line1, String line2, String line3 = "") {
  display.clearDisplay();

  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);

  display.setCursor(0, 5);
  display.println("SIMON SAYS 2.0");

  display.drawLine(0, 16, 127, 16, SSD1306_WHITE);

  display.setCursor(0, 24);
  display.println(line1);

  display.setCursor(0, 38);
  display.println(line2);

  display.setCursor(0, 52);
  display.println(line3);

  display.display();
}

void showScore() {
  showMessage(
    "LEVEL: " + String(level),
    "BEST: " + String(bestScore),
    "Repeat the sequence!"
  );
}

// ---------------- LED AND SOUND ----------------

void playColor(int color, int duration) {

  digitalWrite(LED_PINS[color], HIGH);

  tone(BUZZER_PIN, tones[color]);

  delay(duration);

  digitalWrite(LED_PINS[color], LOW);

  noTone(BUZZER_PIN);

  delay(100);
}

void allLEDsOff() {
  for (int i = 0; i < 3; i++) {
    digitalWrite(LED_PINS[i], LOW);
  }

  noTone(BUZZER_PIN);
}

// ---------------- BUTTON INPUT ----------------

int readButton() {

  while (true) {

    for (int i = 0; i < 3; i++) {

      if (digitalRead(BUTTON_PINS[i]) == LOW) {

        delay(30);

        if (digitalRead(BUTTON_PINS[i]) == LOW) {

          while (digitalRead(BUTTON_PINS[i]) == LOW) {
            delay(5);
          }

          return i;
        }
      }
    }
  }
}

// ---------------- SHOW SEQUENCE ----------------

void showSequence() {

  int speed = 600 - (level * 25);

  if (speed < 180) {
    speed = 180;
  }

  showMessage(
    "MEMORIZE!",
    "Level: " + String(level),
    "Watch carefully..."
  );

  delay(800);

  for (int i = 0; i < level; i++) {

    playColor(sequence[i], speed);

    delay(150);
  }

  showScore();
}

// ---------------- GAME OVER ----------------

void gameOver() {

  gameRunning = false;

  allLEDsOff();

  // Save new record
  if (level - 1 > bestScore) {

    bestScore = level - 1;

    preferences.putInt("best", bestScore);
  }

  // Game over sound
  tone(BUZZER_PIN, 180);
  delay(400);

  noTone(BUZZER_PIN);
  delay(150);

  tone(BUZZER_PIN, 130);
  delay(500);

  noTone(BUZZER_PIN);

  showMessage(
    "GAME OVER!",
    "Your score: " + String(level - 1),
    "Best: " + String(bestScore)
  );

  delay(2500);

  showMessage(
    "SIMON SAYS 2.0",
    "Press any button",
    "to play again!"
  );
}

// ---------------- START GAME ----------------

void startGame() {

  level = 0;
  gameRunning = true;

  randomSeed(esp_random());

  showMessage(
    "NEW GAME!",
    "Get ready...",
    "Remember the colours"
  );

  delay(1200);
}

// ---------------- SETUP ----------------

void setup() {

  Serial.begin(115200);

  // LED setup
  for (int i = 0; i < 3; i++) {

    pinMode(LED_PINS[i], OUTPUT);

    digitalWrite(LED_PINS[i], LOW);
  }

  // Button setup
  for (int i = 0; i < 3; i++) {

    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
  }

  pinMode(BUZZER_PIN, OUTPUT);

  noTone(BUZZER_PIN);

  // OLED initialization
  Wire.begin(8, 9);

  if (!display.begin(
    SSD1306_SWITCHCAPVCC,
    OLED_ADDRESS
  )) {

    Serial.println("OLED ERROR!");

    while (true) {
      delay(100);
    }
  }

  // High score storage
  preferences.begin("SimonGame", false);

  bestScore = preferences.getInt("best", 0);

  showMessage(
    "SIMON SAYS 2.0",
    "Best: " + String(bestScore),
    "Press any button!"
  );

  Serial.println("Simon Says Ready!");
}

// ---------------- MAIN LOOP ----------------

void loop() {

  // Wait for a button to start
  if (!gameRunning) {

    readButton();

    startGame();
  }

  // Add another random colour
  if (level < MAX_SEQUENCE) {

    sequence[level] = random(0, 3);

    level++;

  } else {

    gameOver();
    return;
  }

  // Show the sequence
  showSequence();

  // Wait for player input
  for (int i = 0; i < level; i++) {

    int playerInput = readButton();

    // Flash the selected colour
    playColor(playerInput, 180);

    // Check answer
    if (playerInput != sequence[i]) {

      gameOver();

      return;
    }
  }

  // Successful round
  showMessage(
    "CORRECT!",
    "Level: " + String(level),
    "Next round..."
  );

  tone(BUZZER_PIN, 1000);
  delay(100);

  noTone(BUZZER_PIN);

  delay(700);
}
