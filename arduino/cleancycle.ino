/*
 * ============================================================
 *  CleanCycle – Smart Plastic Waste Collection System
 *  Arduino UNO | HC-SR04 Ultrasonic Sensor | SG90 Servo
 * ============================================================
 *  WIRING SUMMARY:
 *   HC-SR04:
 *     VCC  → 5V
 *     GND  → GND
 *     TRIG → Pin 9
 *     ECHO → Pin 10
 *
 *   SG90 Servo:
 *     Red   (VCC)  → 5V
 *     Brown (GND)  → GND
 *     Orange(SIG)  → Pin 6
 * ============================================================
 */

#include <Servo.h>

// ---------- PIN DEFINITIONS ----------
const int TRIG_PIN   = 9;
const int ECHO_PIN   = 10;
const int SERVO_PIN  = 6;
const int LED_PIN    = 13;   // onboard LED as status indicator

// ---------- TUNABLE PARAMETERS ----------
const int DETECT_DISTANCE_CM = 15;   // trigger if object closer than 15 cm
const int FLAP_OPEN_ANGLE    = 90;   // degrees – flap open position
const int FLAP_CLOSED_ANGLE  = 0;    // degrees – flap closed position
const unsigned long OPEN_DURATION_MS = 3000;  // keep flap open 3 seconds
const unsigned long DEBOUNCE_MS      = 5000;  // wait 5 s before next detection

// ---------- GLOBALS ----------
Servo flapServo;

int  totalCollected    = 0;
float totalWeightGrams = 0.0;
const float WEIGHT_PER_BOTTLE_G = 25.0;  // average PET bottle ~25 g

bool flapIsOpen      = false;
unsigned long flapOpenedAt = 0;
unsigned long lastCollectedAt = 0;

// ---------- HELPER: measure distance ----------
long measureDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30 ms timeout
  if (duration == 0) return 999;                  // no echo → nothing nearby
  return duration / 58;                           // convert to cm
}

// ---------- HELPER: open flap ----------
void openFlap() {
  flapServo.write(FLAP_OPEN_ANGLE);
  digitalWrite(LED_PIN, HIGH);
  flapIsOpen    = true;
  flapOpenedAt  = millis();

  totalCollected++;
  totalWeightGrams += WEIGHT_PER_BOTTLE_G;
  lastCollectedAt = millis();

  Serial.println(F("------------------------------"));
  Serial.println(F("[STATUS] Object Detected!"));
  Serial.println(F("[ACTION] Collection Activated – Flap OPEN"));
  Serial.print  (F("[INFO]   Total collected so far: "));
  Serial.println(totalCollected);
  Serial.print  (F("[INFO]   Estimated total weight: "));
  Serial.print  (totalWeightGrams / 1000.0, 3);
  Serial.println(F(" kg"));
  Serial.println(F("------------------------------"));
}

// ---------- HELPER: close flap ----------
void closeFlap() {
  flapServo.write(FLAP_CLOSED_ANGLE);
  digitalWrite(LED_PIN, LOW);
  flapIsOpen = false;

  Serial.println(F("[STATUS] Collection Completed – Flap CLOSED"));
  Serial.println(F("[INFO]   Ready for next item."));
  Serial.println();
}

// ---------- SETUP ----------
void setup() {
  Serial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(LED_PIN,  OUTPUT);

  flapServo.attach(SERVO_PIN);
  flapServo.write(FLAP_CLOSED_ANGLE);
  delay(500);

  Serial.println(F("============================================================"));
  Serial.println(F("   CleanCycle – Smart Plastic Waste Collection System"));
  Serial.println(F("   Arduino UNO | HC-SR04 | SG90 Servo"));
  Serial.println(F("============================================================"));
  Serial.println(F("[BOOT] System initialized. Waiting for plastic items..."));
  Serial.println();
}

// ---------- MAIN LOOP ----------
void loop() {
  unsigned long now = millis();

  // ---- Close flap after OPEN_DURATION_MS ----
  if (flapIsOpen && (now - flapOpenedAt >= OPEN_DURATION_MS)) {
    closeFlap();
  }

  // ---- Don't scan during debounce window ----
  if (!flapIsOpen && (now - lastCollectedAt < DEBOUNCE_MS)) {
    return;
  }

  // ---- Measure distance ----
  long dist = measureDistanceCM();

  // ---- Optional: print distance every second for debugging ----
  static unsigned long lastPrint = 0;
  if (now - lastPrint >= 1000) {
    lastPrint = now;
    Serial.print(F("[SCAN]  Distance: "));
    Serial.print(dist);
    Serial.println(F(" cm"));
  }

  // ---- Detect object & open flap ----
  if (!flapIsOpen && dist < DETECT_DISTANCE_CM) {
    openFlap();
  }
}
