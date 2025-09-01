#include <Watchdog.h>

// Watchdog object
Watchdog watchdog;
#define led 11
#define uv 10
#define defectCone 14
#define pause 12
#define uvled 15

bool coneDefect = false;
bool sensorsignalflag = false;
bool ledflag = false;
bool flag_hazard = true;
bool debugging_led = false;  // Existing debug LED flag
bool debugFlag = false;      // <-- New global debug flag

int sensor1Pin =27;
int sensor2Pin = 26;
//int PreviousSensorValue = HIGH;
unsigned long LEDOnDelayTime = 0;
//int PreviousTipSensorValue = LOW;
unsigned long coneDefectTime = 0;
unsigned long lastCallTime = 0;
unsigned long LEDDelayTime = 0;
short LED_Delay     = 140;
int sensor1Value = HIGH;
int sensor2Value = HIGH;
int sensor3Value = HIGH;

// For detecting changes
int PreviousSensorValue = HIGH;
int Previous3SensorValue = LOW;
int PreviousTipSensorValue = LOW;
unsigned long lastLReceivedTime = 0;
bool connectionLost = false;
const unsigned long connectionTimeout = 10000; // 10 seconds

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

  
  //Output Pin
  pinMode(led,OUTPUT);
  pinMode(uv,OUTPUT);
  pinMode(defectCone, OUTPUT);
  pinMode(pause, OUTPUT);
  pinMode(uvled, OUTPUT);
  pinMode(sensor2Pin, INPUT_PULLUP);
  pinMode(sensor1Pin, INPUT_PULLUP);
  digitalWrite(led,HIGH);
  digitalWrite(uv,HIGH);
  digitalWrite(defectCone, LOW);
  watchdog.enable(Watchdog::TIMEOUT_1S);

  debugPrint("System Initialized");
}

void loop() {
  int sensor2Value = digitalRead(sensor2Pin);
  int sensor1Value = digitalRead(sensor1Pin);
  if (Serial.available()) {
    unsigned long startTime = millis();   // Start time before reading
    Serial.setTimeout(5);
    // Read data until timeout or complete message
    String temp1 = Serial.readStringUntil('\n');
    temp1.trim();
//    Serial.println(millis());
    unsigned long endTime = millis();  
    //  Serial.println(temp1[0]);
    if (temp1 == "1") {
      digitalWrite(defectCone, HIGH);
      coneDefect = true;
      debugPrint("Defect");
    }
    else if (temp1 == "2") {
      digitalWrite(defectCone, LOW);
      coneDefect = false;
      debugPrint("Clear");
    }
    else if (temp1 == "3") {
    digitalWrite(pause, HIGH);
    coneDefect = false;
        debugPrint("pause");
  }
  else if (temp1 == "4") {
    digitalWrite(pause, LOW);
    coneDefect = false;
        debugPrint("go");
  }
  else if (temp1 == "5") {
    digitalWrite(uvled, HIGH);
    coneDefect = false;
        debugPrint("uvledon");
  }
  else if (temp1 == "6") {
    digitalWrite(uvled, LOW);
    coneDefect = false;
        debugPrint("uvledoff");
  }
    else if (temp1 == "D") {
      debugging_led = true;
      debugFlag = true; // Enable debug mode
      debugPrint("Debug ON");
    }
    else if (temp1 == "C") {
      debugging_led = false;
      debugFlag = false; // Disable debug mode
      Serial.println("debugging_off");
    }
    else if(coneDefect == false && temp1 == "A"){
        Serial.println("Hazard detected");
       digitalWrite(defectCone, HIGH);
        digitalWrite(pause, HIGH);
        flag_hazard = false;
      }
      else if(coneDefect == false && temp1 == "B"){
        Serial.println("Hazard cleared");
        digitalWrite(defectCone, LOW);
        digitalWrite(pause, LOW);
        flag_hazard = true;
      }   
  }


  if (sensor1Value == HIGH and PreviousTipSensorValue == LOW)
  {
    digitalWrite(led,HIGH);
    sensorsignalflag = true;
    ledflag = true;
    LEDOnDelayTime = millis();
    LEDDelayTime = millis();
  }

  if (sensor2Value == HIGH and PreviousSensorValue == LOW )
  {
    Serial.println("1");
  }

  if((millis() - LEDOnDelayTime)>=10 and sensorsignalflag == true ){
     Serial.println("0");
     sensorsignalflag = false;
  }

  if((millis() - LEDDelayTime)>=LED_Delay and ledflag == true ){
     LEDDelayTime = 0;
     ledflag = false;
  }
  
  PreviousSensorValue = sensor2Value;
  PreviousTipSensorValue = sensor1Value;
  watchdog.reset();
  
}