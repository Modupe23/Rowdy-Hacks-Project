#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- PIN DEFINITIONS ---
const int TRIG_PIN = 9;   // Ultrasonic Trig
const int ECHO_PIN = 10;  // Ultrasonic Echo
const int SPEAKER_PIN = 8;  // Buzzer

// --- SETTINGS ---

LiquidCrystal_I2C lcd(0x27, 16, 2);  

// Demo values, multiply by 60 for real values
const unsigned long WORK_DURATION  = 25 * 1000 *60;  // 25 Seconds
const unsigned long BREAK_DURATION = 5 * 1000 *60; // 5 Seconds

// Posture Settings (in Centimeters)
const int POSTURE_MIN_CM = 30; // Too Close!
const int POSTURE_MAX_CM = 55; // Too Far!

// --- GLOBAL VARS ---

// This is a "state machine"
enum State { STATE_WORK, STATE_BREAK };
State currentState = STATE_WORK;

// Non-blocking timers (using millis)
unsigned long stateStartTime;      // When did the current work/break state start?
unsigned long lastLcdUpdateTime;   // When did we last update the LCD timer?
unsigned long lastPostureCheckTime; // When did we last check posture?

// Store the remaining time to display it
unsigned long timeRemaining;

// Keep track of posture to avoid constant beeping
enum PostureState { POSTURE_GOOD, POSTURE_CLOSE, POSTURE_FAR };
PostureState currentPosture = POSTURE_GOOD;


// --- SETUP: Runs once at the beginning ---
void setup() {
  // Initialize Serial for debugging
  Serial.begin(9600);

  // Initialize hardware pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(SPEAKER_PIN, OUTPUT);

  // Initialize the LCD
  lcd.init();
  lcd.backlight();
  lcd.print("Posture Pomodoro");
  lcd.setCursor(0, 1);
  lcd.print("Starting up...");
  delay(2000);

  // Start the first work session
  startWorkSession();
}


// --- LOOP: Runs forever, as fast as possible ---
// We avoid all delay() functions here!
void loop() {
  // Get the current time once per loop
  unsigned long currentTime = millis();

  // --- 1. State Timer Management ---
  // Check if the current work/break session is over
  if (currentTime - stateStartTime >= timeRemaining) {
    if (currentState == STATE_WORK) {
      startBreakSession();
    } else {
      startWorkSession();
    }
  }

  // --- LCD Timer Update ---
  // Update the countdown on the LCD only once per second
  if (currentTime - lastLcdUpdateTime >= 1000) {
    lastLcdUpdateTime = currentTime;
    updateLcdTimer(currentTime);
  }

  // ---  Posture Check ---
  // Only check posture during the WORK state
  // Check every 500ms (half a second)
  if (currentState == STATE_WORK) {
    if (currentTime - lastPostureCheckTime >= 500) {
      lastPostureCheckTime = currentTime;
      checkPosture();
    }
  }
}


// --- HELPER FUNCTIONS ---

void startWorkSession() {
  Serial.println("Starting WORK session.");
  currentState = STATE_WORK;
  stateStartTime = millis();
  timeRemaining = WORK_DURATION;
  currentPosture = POSTURE_GOOD; // Reset posture at start
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WORK SESH");

  // Play a start sound
  tone(SPEAKER_PIN, 523, 150); // C5
  delay(150);
  tone(SPEAKER_PIN, 783, 200); // G5
}

void startBreakSession() {
  Serial.println("Starting BREAK session.");
  currentState = STATE_BREAK;
  stateStartTime = millis();
  timeRemaining = BREAK_DURATION;

  // Stop any lingering posture warning sounds
  noTone(SPEAKER_PIN); 

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("BREAK TIME!");
  lcd.setCursor(0, 1);
  lcd.print("Relax your eyes.");

  // Play a "break" melody
  tone(SPEAKER_PIN, 783, 150); // G5
  delay(150);
  tone(SPEAKER_PIN, 523, 150); // C5
  delay(150);
  tone(SPEAKER_PIN, 783, 150); // G5
}

void updateLcdTimer(unsigned long currentTime) {
  // Calculate remaining time
  unsigned long elapsed = currentTime - stateStartTime;
  unsigned long remainingSeconds = (timeRemaining - elapsed) / 1000;

  int minutes = remainingSeconds / 60;
  int seconds = remainingSeconds % 60;

  // Position cursor on the top line, after the title
  lcd.setCursor(11, 0);

  // Print minutes
  if (minutes < 10) lcd.print("0");
  lcd.print(minutes);
  lcd.print(":");
  
  // Print seconds
  if (seconds < 10) lcd.print("0");
  lcd.print(seconds);
}

void checkPosture() {
  long distance = getDistanceCm();
  Serial.print("Distance: ");
  Serial.println(distance);

  // Check against our posture thresholds
  if (distance > 0 && distance < POSTURE_MIN_CM) {
    // Only update if the state has changed
    if (currentPosture != POSTURE_CLOSE) {
      currentPosture = POSTURE_CLOSE;
      lcd.setCursor(0, 1);
      lcd.print("! TOO CLOSE !   "); // Extra spaces to clear line
      tone(SPEAKER_PIN, 200, 300); // Low, annoying tone
    }
    // We stay beeping until posture is good.
    // Re-trigger the tone
    tone(SPEAKER_PIN, 200, 300); 

  } else if (distance > POSTURE_MAX_CM) {
    if (currentPosture != POSTURE_FAR) {
      currentPosture = POSTURE_FAR;
      lcd.setCursor(0, 1);
      lcd.print("! TOO FAR !     ");
      tone(SPEAKER_PIN, 1000, 300); // High, annoying tone
    }
    // Re-trigger the tone
    tone(SPEAKER_PIN, 1000, 300); 

  } else {
    // Posture is good
    if (currentPosture != POSTURE_GOOD) {
      currentPosture = POSTURE_GOOD;
      lcd.setCursor(0, 1);
      lcd.print("Posture: Good  ");
      noTone(SPEAKER_PIN); // Stop warning sounds
    }
  }
}

// Function to get distance from ultrasonic sensor
long getDistanceCm() {
  // Clear the trigPin
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  
  // Set the trigPin on HIGH state for 10 micro seconds
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Read the echoPin, returns the sound wave travel time in microseconds
  long duration = pulseIn(ECHO_PIN, HIGH);
  
  // Calculate the distance
  // Speed of sound = 0.034 cm/uS. Divide by 2 for back and forth distance
  return duration * 0.034 / 2; 
}