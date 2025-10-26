#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int btnPlus = 6;
const int btnMinus = 7;
const int btnNext = 8;
const int btnStart = 9;

int studyTime = 25;
int breakTime = 5;
int rounds = 4;

int menuState = 0;
bool running = false;

void setup() {
  Serial.begin(9600);
  
  lcd.init();
  lcd.backlight();

  pinMode(btnPlus, INPUT_PULLUP);
  pinMode(btnMinus, INPUT_PULLUP);
  pinMode(btnNext, INPUT_PULLUP);
  pinMode(btnStart, INPUT_PULLUP);

  lcd.print("Pomodoro Timer");
  delay(1500);
  showMenu();
}

void loop() {
  if (!running) {
    handleMenuInput();
  } else {
    runPomodoro();
  }
}

void showMenu() {
  lcd.clear();
  if (menuState == 0) {
    lcd.print("Study: ");
    lcd.print(studyTime);
    lcd.print(" min");
    lcd.setCursor(0, 1);
    lcd.print("Next=Break");
    delay(1500);
  } else if (menuState == 1) {
    lcd.print("Break: ");
    lcd.print(breakTime);
    lcd.print(" min");
    lcd.setCursor(0, 1);
    lcd.print("Next=Rounds");
    delay(1500);
  } else if (menuState == 2) {
    lcd.print("Rounds: ");
    lcd.print(rounds);
    lcd.setCursor(0, 1);
    lcd.print("Next=Start");
    delay(1500);
  } else if (menuState == 3) {
    lcd.print("Press START");
    lcd.setCursor(0, 1);
    lcd.print("to begin!");
    delay(1500);
  }
}

void handleMenuInput() {
  if (digitalRead(btnPlus) == LOW) {
    if (menuState == 0 && studyTime < 60) studyTime++;
    if (menuState == 1 && breakTime < 60) breakTime++;
    if (menuState == 2 && rounds < 10) rounds++;
    showMenu();
    delay(250);
  }
  if (digitalRead(btnMinus) == LOW) {
    if (menuState == 0 && studyTime > 1) studyTime--;
    if (menuState == 1 && breakTime > 1) breakTime--;
    if (menuState == 2 && rounds > 1) rounds--;
    showMenu();
    delay(250);
  }
  if (digitalRead(btnNext) == LOW) {
    menuState = (menuState + 1) % 4;
    showMenu();
    delay(250);
  }
  if (digitalRead(btnStart) == LOW && menuState == 3) {
    running = true;
    lcd.clear();
    lcd.print("Starting...");
    delay(1000);
  }
}

void runPomodoro() {
  for (int r = 1; r <= rounds; r++) {
    lcd.clear();
    lcd.print("Round ");
    lcd.print(r);
    lcd.setCursor(0, 1);
    lcd.print("Study!");
    countdown(studyTime * 60);

    lcd.clear();
    lcd.print("Break!");
    countdown(breakTime * 60);
  }

  lcd.clear();
  lcd.print("All done!");
  tone(10, 1000, 1000);
  delay(3000);
  running = false;
  menuState = 0;
  showMenu();
}

void countdown(int seconds) {
  unsigned long start = millis();
  while (millis() - start < (unsigned long)seconds * 1000) {
    int remaining = seconds - (millis() - start) / 1000;
    lcd.setCursor(0, 1);
    lcd.print("Left: ");
    lcd.print(remaining);
    lcd.print("s   ");
    delay(1000);
  }
}