#include <Watchdog.h>

// Watchdog object
Watchdog watchdog;

#define FORWARD      10
#define REVERSE      11
#define DEFECTCONE   12
#define UVLED        14
#define TOWERLAMP    15

const int ledPin = LED_BUILTIN;
bool coneDefect = false;
bool sensorsignalflag = false;
bool debugging_led = false;  // Existing debug LED flag
bool debugFlag = false;      // <-- New global debug flag

// Sensor pins
const int sensor1Pin = 26;
const int sensor2Pin = 27;
const int sensor3Pin = 25;

// Stable sensor values
int sensor1Value = HIGH;
int sensor2Value = HIGH;
int sensor3Value = HIGH;

// For detecting changes
int PreviousSensorValue = HIGH;
int Previous3SensorValue = LOW;
int PreviousTipSensorValue = LOW;

// Timing variables
unsigned long LEDOnDelayTime = 0;
unsigned long coneDefectTime = 0;
unsigned long lastCallTime = 0;
unsigned long LEDDelayTime = 0;


// Debounce settings
const unsigned long debounceDelay = 0.5; // 30 ms debounce delay
unsigned long lastDebounceTime1 = 0;
unsigned long lastDebounceTime2 = 0;
unsigned long lastDebounceTime3 = 0;

// For measuring debounce duration
unsigned long debounceStartTime1 = 0;
unsigned long debounceStartTime2 = 0;
unsigned long debounceStartTime3 = 0;

int debounceRead(int pin, int &lastStableValue, unsigned long &lastDebounceTime, unsigned long &debounceStartTime) {
    int reading = digitalRead(pin);

    // If pin reading has changed, start timing
    if (reading != lastStableValue) {
        lastDebounceTime = millis();
        debounceStartTime = millis();   // Record when instability started
    }

    // If debounce time has passed, confirm stable state
    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (lastStableValue != reading) {
            // Only log when the stable value actually changes
            unsigned long debounceDuration = millis() - debounceStartTime;
        }
        lastStableValue = reading;
    }
    return lastStableValue;
}

// ---------- FUNCTION TO PRINT DEBUG MESSAGES ----------
void debugPrint(const String &msg) {
  if (debugFlag) {
    Serial.println(msg);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(10);
  

  // Output pins
  pinMode(UVLED, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(FORWARD, OUTPUT);
  pinMode(REVERSE, OUTPUT);
  pinMode(TOWERLAMP, OUTPUT);
  pinMode(DEFECTCONE, OUTPUT);


  // Input pins with pull-ups
  pinMode(sensor1Pin, INPUT_PULLUP);
  pinMode(sensor2Pin, INPUT_PULLUP);
  pinMode(sensor3Pin, INPUT_PULLUP);

  digitalWrite(DEFECTCONE, LOW);
  digitalWrite(REVERSE, HIGH);

  watchdog.enable(Watchdog::TIMEOUT_1S);

  debugPrint("System Initialized");
}

void loop() {
  // ---------- DEBOUNCED SENSOR READS ----------
  sensor1Value = debounceRead(sensor1Pin, sensor1Value, lastDebounceTime1, debounceStartTime1);
  sensor2Value = debounceRead(sensor2Pin, sensor2Value, lastDebounceTime2, debounceStartTime2);
  sensor3Value = debounceRead(sensor3Pin, sensor3Value, lastDebounceTime3, debounceStartTime3);

  // ---------- SERIAL COMMANDS ----------
  if (Serial.available()) {
    unsigned long startTime = millis();   // Start time before reading
    Serial.setTimeout(5);
    // Read data until timeout or complete message
    String temp1 = Serial.readStringUntil('\n');
    temp1.trim();
//    Serial.println(millis());
    unsigned long endTime = millis();     // Time after reading

    if (temp1 == "1") {
      digitalWrite(DEFECTCONE, HIGH);
      digitalWrite(TOWERLAMP, HIGH);
      coneDefect = true;
      debugPrint("Defect");
    }
    else if (temp1 == "2") {
      digitalWrite(DEFECTCONE, LOW);
      digitalWrite(TOWERLAMP, LOW);
      coneDefect = false;
      debugPrint("Clear");
    }
    else if (temp1 == "3") {
      digitalWrite(TOWERLAMP, HIGH);
      debugPrint("TOWERLAMP");
    }
    else if (temp1 == "-3") {
      digitalWrite(TOWERLAMP, LOW);
      debugPrint("go");
    }
    else if (temp1 == "4") {
      digitalWrite(UVLED, HIGH);
      debugPrint("UVLEDon");
    }
    else if (temp1 == "-4") {
      digitalWrite(UVLED, LOW);
      debugPrint("UVLEDoff");
    }
    else if (temp1 == "5") {
      digitalWrite(FORWARD, HIGH);
      debugPrint("FORWARDon");
    }
    else if (temp1 == "-5") {
      digitalWrite(FORWARD, LOW);
      debugPrint("FORWARDoff");
    }
    else if (temp1 == "D") {
      debugging_led = true;
      debugFlag = true; // Enable debug mode
      digitalWrite(ledPin, HIGH);
      debugPrint("Debug ON");
    }
    else if (temp1 == "C") {
      debugging_led = false;
      debugFlag = false; // Disable debug mode
      digitalWrite(ledPin, LOW);
      // No debugPrint here since we just turned it off
    }
  }

  // ---------- SENSOR-BASED CONTROL ----------
  if (sensor1Value == LOW && PreviousTipSensorValue == HIGH) {
    digitalWrite(FORWARD, HIGH);
    digitalWrite(REVERSE, LOW);
    debugPrint("Sensor1 triggered - Stopping FORWARD");
  }

  if (sensor2Value == LOW && PreviousSensorValue == HIGH) {
    digitalWrite(FORWARD, LOW);
    digitalWrite(REVERSE, HIGH);
    debugPrint("Sensor2 triggered - Moving FORWARD");
  }

  if (sensor3Value == LOW && Previous3SensorValue == HIGH) {
    debugPrint("Sensor3 triggered - Sending 0");
    Serial.println("0");
  }

  if ((millis() - LEDOnDelayTime) >= 10 && sensorsignalflag) {
    sensorsignalflag = false;
  }

  // ---------- UPDATE PREVIOUS STATES ----------
  PreviousSensorValue = sensor2Value;
  Previous3SensorValue = sensor3Value;
  PreviousTipSensorValue = sensor1Value;

  // Watchdog reset
  watchdog.reset();
}