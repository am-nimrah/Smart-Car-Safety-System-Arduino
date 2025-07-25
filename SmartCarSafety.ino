#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <Keypad.h>

// OLED config
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// DHT Sensor
#define DHTPIN A0
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Pins
#define PIR_PIN 2
#define GAS_PIN A1
#define BUZZER_PIN 3
#define LED_PIN 4
#define BUTTON_PIN 5

// Keypad
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
{'1','2','3','A'},
{'4','5','6','B'},
{'7','8','9','C'},
{'*','0','#','D'}
};
byte rowPins[ROWS] = {6, 7, 8, 9};
byte colPins[COLS] = {10, 11, 12, 13};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Variables
String mode = "None";
bool modeSelected = false;
bool requireMotion = true;
int tempThreshold = 38;
int gasThreshold = 400;
bool alertOn = false;
bool manualAlert = false;
unsigned long lastBlinkTime = 0;
bool ledState = false;

void setup() {
Serial.begin(9600);

pinMode(PIR_PIN, INPUT);
pinMode(GAS_PIN, INPUT);
pinMode(BUZZER_PIN, OUTPUT);
pinMode(LED_PIN, OUTPUT);
pinMode(BUTTON_PIN, INPUT_PULLUP);

dht.begin();
delay(300);

if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
Serial.println(F("OLED not found"));
while (true);
}

display.clearDisplay();
display.setTextSize(1);
display.setTextColor(WHITE);
display.setCursor(0, 0);
display.println(F("Smart Car Safety"));
display.println(F("Select Mode:"));
display.println(F("A: Pet  B: Child"));
display.println(F("C: Full"));
display.display();
}

void loop() {
char key = keypad.getKey();

// Mode selection
if (!modeSelected && key) {
if (key == 'A') {
mode = "Pet";
tempThreshold = 35;
gasThreshold = 300;
requireMotion = false;
} else if (key == 'B') {
mode = "Child";
tempThreshold = 38;
gasThreshold = 400;
requireMotion = true;
} else if (key == 'C') {
mode = "Full";
tempThreshold = 35;
gasThreshold = 300;
requireMotion = true;
}

if (key == 'A' || key == 'B' || key == 'C') {  
  modeSelected = true;  
  display.clearDisplay();  
  display.setCursor(0, 0);  
  display.print("Mode: ");  
  display.println(mode);  
  display.display();  
  delay(1000);  
}  
return;

}

// Toggle manual alert via 'D' key
if (key == 'D') {
manualAlert = !manualAlert;      // Toggle manual mode
alertOn = manualAlert;           // Set alert state
Serial.println(manualAlert ? "Manual Alert ON" : "Manual Alert OFF");
if (!manualAlert) {
digitalWrite(BUZZER_PIN, LOW);
digitalWrite(LED_PIN, LOW);
}
delay(300); // Debounce
}

// Reset alert with button or # key
if (digitalRead(BUTTON_PIN) == LOW || key == '#') {
alertOn = false;
manualAlert = false;
digitalWrite(BUZZER_PIN, LOW);
digitalWrite(LED_PIN, LOW);
Serial.println("Alert reset");
delay(300);
}

// Read sensors
float temperature = dht.readTemperature();
float humidity = dht.readHumidity();
int gasLevel = analogRead(GAS_PIN);
bool motionDetected = digitalRead(PIR_PIN);

if (isnan(temperature) || isnan(humidity)) return;

// Automatic alert trigger
if (!manualAlert &&
(temperature > tempThreshold || gasLevel > gasThreshold) &&
(!requireMotion || motionDetected)) {
alertOn = true;
}

// Alert actions
if (alertOn) {
unsigned long currentMillis = millis();
if (currentMillis - lastBlinkTime >= 500) {
lastBlinkTime = currentMillis;
ledState = !ledState;
digitalWrite(LED_PIN, ledState ? HIGH : LOW);
digitalWrite(BUZZER_PIN, ledState ? HIGH : LOW);
}
} else {
digitalWrite(LED_PIN, LOW);
digitalWrite(BUZZER_PIN, LOW);
}

// OLED display
display.clearDisplay();
display.setCursor(0, 0);
display.print("T: "); display.print(temperature); display.println(" C");
display.print("H: "); display.print(humidity); display.println(" %");
display.print("Gas: "); display.println(gasLevel);
display.print("Motion: "); display.println(motionDetected ? "YES" : "NO");
display.setCursor(0, 56);
display.print(alertOn ? "ALERT!!" : "Safe");
display.display();

delay(100);
}
