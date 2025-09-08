#include <Watchdog.h>

// Watchdog object
Watchdog watchdog;

#define UVLED 10
#define TIPWHITELED 11
#define CONVEYORPAUSE 12
#define DEFECTCONE 14
#define UVWHITELED 15

// Sensor pins
const int sensor1Pin = 26;
const int sensor2Pin = 27;
const int sensor3Pin = 25;
const int ledPin = LED_BUILTIN;

// flag 
bool debugFlag = false;      // <-- New global debug flag
bool coneDefect = false;
bool flag_hazard = true;
bool debugging_led = false;  // Existing debug LED flag
bool sensorsignalflag = false;
bool connectionLost = false;
const unsigned long connectionTimeout = 10000; // 10 seconds


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
  //Serial Communication
  Serial.begin(115200);
  Serial.setTimeout(10);
  

  // Output pins  
  pinMode(UVLED, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(UVWHITELED, OUTPUT);
  pinMode(DEFECTCONE, OUTPUT); 
  pinMode(TIPWHITELED, OUTPUT); 
  pinMode(CONVEYORPAUSE, OUTPUT);


  // Input pins with pull-ups
  pinMode(sensor1Pin, INPUT_PULLUP);
  pinMode(sensor2Pin, INPUT_PULLUP);
  pinMode(sensor3Pin, INPUT_PULLUP);


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

    // ------------ HEART BEAT CONNECTED---------------
    if (temp1 == "L") {
      lastLReceivedTime = millis();  // Update the time when "L" is received
      if (connectionLost) {
        debugPrint("Connected");
        connectionLost = false;
        digitalWrite(DEFECTCONE, LOW);
        digitalWrite(CONVEYORPAUSE, LOW);
      }
    }

    if (temp1 == "1") {
      digitalWrite(DEFECTCONE, HIGH);
      coneDefect = true;
      debugPrint("Defect");
    }
    else if (temp1 == "2") {
      digitalWrite(DEFECTCONE, LOW);
      coneDefect = false;
      debugPrint("Clear");
    }
    else if (temp1 == "D") {
      debugging_led = true;
      debugFlag = true; // Enable debug mode
      // digitalWrite(ledPin, HIGH);
      debugPrint("Debug ON");
    }
    else if (temp1 == "C") {
      debugging_led = false;
      debugFlag = false; // Disable debug mode
      // digitalWrite(ledPin, LOW);
      debugPrint("Debug OFF");
      // No debugPrint here since we just turned it off
    }else if (temp1 == "3") {
      digitalWrite(CONVEYORPAUSE, HIGH);
      debugPrint("CONVEYORPAUSE ON");
    }
    else if (temp1 == "-3") {
      digitalWrite(CONVEYORPAUSE, LOW);
      debugPrint("CONVEYORPAUSE OFF");
      // No debugPrint here since we just turned it off
    }
    else if(coneDefect == false && temp1 == "A"){
        debugPrint("Hazard detected");
       digitalWrite(DEFECTCONE, HIGH);
        digitalWrite(CONVEYORPAUSE, HIGH);
        flag_hazard = false;
      }
      else if(coneDefect == false && temp1 == "B"){
        debugPrint("Hazard cleared");
        digitalWrite(DEFECTCONE, LOW);
        digitalWrite(CONVEYORPAUSE, LOW);
        flag_hazard = true;
      }   
  }

  // ---------- SENSOR-BASED CONTROL ----------
  if (sensor2Value == LOW && PreviousSensorValue == HIGH) {
    debugPrint("Sensor2 triggered - Sending 0");
    Serial.println("0");
  }

  // ---------- DEBOUNCED SENSOR VALUE CHANGE ----------
  if ((millis() - LEDOnDelayTime) >= 10 && sensorsignalflag) {
    sensorsignalflag = false;
  }

  // ------------ HEART BEAT TERMINATED---------------
  if (millis() - lastLReceivedTime > connectionTimeout && !connectionLost) {
    Serial.println("Connection terminated");
    connectionLost = true;
    digitalWrite(DEFECTCONE, HIGH);
    digitalWrite(CONVEYORPAUSE, HIGH);
  }

  // ---------- UPDATE PREVIOUS STATES ----------
  PreviousSensorValue = sensor2Value;
  Previous3SensorValue = sensor3Value;
  PreviousTipSensorValue = sensor1Value;

  // Watchdog reset
  watchdog.reset();
}