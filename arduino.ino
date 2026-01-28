#include "esp_task_wdt.h"

// ---------------- SAFE PIN DEFINITIONS ----------------
#define UVLED            18
#define TIPWHITELED      19
#define CONVEYORPAUSE    23
#define DEFECTCONE       15
#define UVWHITELED       22
#define LED_BUILTIN      2

// Sensor pins
const int sensor1Pin = 35;
const int sensor2Pin = 36;
const int sensor3Pin = 25;

// ---------------- FLAGS ----------------
bool debugFlag = false;
bool coneDefect = false;
bool flag_hazard = true;
bool connectionLost = false;

const unsigned long connectionTimeout = 10000;

// ---------------- SENSOR STATES ----------------
int sensor1Value = HIGH;
int sensor2Value = HIGH;
int sensor3Value = HIGH;
int prevSensor2 = HIGH;

// ---------------- DEBOUNCE ----------------
const unsigned long debounceDelay = 30;
unsigned long lastDebounce1 = 0;
unsigned long lastDebounce2 = 0;
unsigned long lastDebounce3 = 0;
unsigned long lastHeartbeat = 0;

// ---------------- FAST DEBUG PRINT ----------------
inline void debugPrint(const char *msg) {
  if (debugFlag) Serial.println(msg);
}

// ---------------- DEBOUNCE FUNCTION ----------------
int debounceRead(int pin, int lastValue, unsigned long &lastTime) {
  int reading = digitalRead(pin);
  if (reading != lastValue && millis() - lastTime > debounceDelay) {
    lastTime = millis();
    return reading;
  }
  return lastValue;
}

// ---------------- SETUP ----------------
void setup() {
  Serial.begin(115200);
  delay(2000);   // <<< IMPORTANT: wait for Serial Monitor

  pinMode(UVLED, OUTPUT);
  pinMode(TIPWHITELED, OUTPUT);
  pinMode(CONVEYORPAUSE, OUTPUT);
  pinMode(DEFECTCONE, OUTPUT);
  pinMode(UVWHITELED, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(sensor1Pin, INPUT_PULLUP);
  pinMode(sensor2Pin, INPUT_PULLUP);
  pinMode(sensor3Pin, INPUT_PULLUP);

  // Fail-safe outputs
  digitalWrite(DEFECTCONE, LOW);
  digitalWrite(CONVEYORPAUSE, LOW);

  // Watchdog (safe)
  esp_task_wdt_init(2, true);
  esp_task_wdt_add(NULL);

  Serial.println("SYSTEM READY");
}

// ---------------- LOOP ----------------
void loop() {
  esp_task_wdt_reset();

  // -------- SENSOR READ --------
  sensor1Value = debounceRead(sensor1Pin, sensor1Value, lastDebounce1);
  sensor2Value = debounceRead(sensor2Pin, sensor2Value, lastDebounce2);
  sensor3Value = debounceRead(sensor3Pin, sensor3Value, lastDebounce3);

  // -------- SENSOR EVENT --------
  if (sensor2Value == LOW && prevSensor2 == HIGH) {
    Serial.println("1");
    debugPrint("Sensor2 Triggered");
  }
  prevSensor2 = sensor2Value;

  // -------- SERIAL COMMAND --------
  if (Serial.available()) {
    char cmd = Serial.read();

    // Ignore newline / carriage return
    if (cmd == '\n' || cmd == '\r') return;

    switch (cmd) {
//      case 'L':
//        lastHeartbeat = millis();
//        if (connectionLost) {
//          connectionLost = false;
//          digitalWrite(DEFECTCONE, LOW);
//          digitalWrite(CONVEYORPAUSE, LOW);
//          debugPrint("Connection Restored");
//        }
//        break;

      case '1':
        coneDefect = true;
        digitalWrite(DEFECTCONE, HIGH);
        debugPrint("Defect ON");
        break;

      case '2':
        coneDefect = false;
        digitalWrite(DEFECTCONE, LOW);
        debugPrint("Defect OFF");
        break;

      case 'D':
        debugFlag = true;
        Serial.println("DEBUG ENABLED");
        break;

      case 'C':
        debugFlag = false;
        Serial.println("DEBUG DISABLED");
        break;

      case '3':
        digitalWrite(LED_BUILTIN, HIGH);
        digitalWrite(CONVEYORPAUSE, HIGH);
        debugPrint("Conveyor Paused");
        break;

      case '4':
        digitalWrite(LED_BUILTIN, LOW);
        digitalWrite(CONVEYORPAUSE, LOW);
        debugPrint("Conveyor Running");
        break;

      case 'A':
        if (!coneDefect) {
          digitalWrite(DEFECTCONE, HIGH);
          digitalWrite(CONVEYORPAUSE, HIGH);
          flag_hazard = false;
          debugPrint("Hazard Detected");
        }
        break;

      case 'B':
        if (!coneDefect) {
          digitalWrite(DEFECTCONE, LOW);
          digitalWrite(CONVEYORPAUSE, LOW);
          flag_hazard = true;
          debugPrint("Hazard Cleared");
        }
        break;
    }
  }

//  // -------- CONNECTION TIMEOUT --------
//  if (millis() - lastHeartbeat > connectionTimeout && !connectionLost) {
//    connectionLost = true;
//    digitalWrite(DEFECTCONE, HIGH);
//    digitalWrite(CONVEYORPAUSE, HIGH);
//    debugPrint("Connection Lost");
//  }
}