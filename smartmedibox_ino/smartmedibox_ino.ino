#define BLYNK_TEMPLATE_ID "TMPL64urKgRIj"
#define BLYNK_TEMPLATE_NAME "Smart Medi Box"
#define BLYNK_AUTH_TOKEN "pqPlhaC8HSsMY6Ig6VgIi4ToAeea0VI9"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "SLT_FIBRE_njCk6";
char pass[] = "uka191721";

const int buzzerPin = 25;
const int fanPin = 14;
const int doorSensorPin = 27;
const int servoPin = 23;

Servo myservo;
Adafruit_SSD1306 display(128, 64, &Wire, -1);
BlynkTimer timer;

// --- Functions ---

void updateOLED(String line1, String line2) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 0);
    display.println(line1);
    display.println(line2);
    display.display();
}

// Logic to send sensor data to Blynk Dashboard widgets
void sendSensorData() {
    // Replace these with actual sensor readings (e.g., DHT.readTemperature())
    float t = 25.0; 
    float h = 60.0;
    
    Blynk.virtualWrite(V4, t); // Temperature Widget
    Blynk.virtualWrite(V5, h); // Humidity Widget
}

// Function handling the physical pill dispensing mechanical motion
void dispensePillSequence() {
    updateOLED("Dispensing...", "Please wait...");
    Serial.println("Dispense initiated.");

    // 1. Move servo to the open/drop position
    myservo.write(90); 
    
    // 2. Beep buzzer to catch user attention
    digitalWrite(buzzerPin, HIGH);
    delay(1000); // Wait 1 second to let the pill slide out
    digitalWrite(buzzerPin, LOW);

    // 3. Return servo to secure/closed position 
    myservo.write(0); 
    
    // 4. Update interface and notify smartphone
    updateOLED("Take Medicine!", "Slot Closed");
    Blynk.logEvent("medication_alert", "Pill has been successfully dispensed!"); 
}

void setup() {
    Serial.begin(115200);
    
    pinMode(buzzerPin, OUTPUT);
    pinMode(fanPin, OUTPUT);
    pinMode(doorSensorPin, INPUT_PULLUP);
    
    myservo.attach(servoPin);
    myservo.write(0); // Ensure servo starts at home/closed position
    
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("OLED Failed");
    }
    
    Blynk.begin(auth, ssid, pass);
    
    // Setup timer: Send sensor data every 5 seconds
    timer.setInterval(5000L, sendSensorData);
    
    updateOLED("MediBox Online", "Waiting for data...");    
}

// --- Blynk Handlers ---

BLYNK_WRITE(V0) { digitalWrite(buzzerPin, param.asInt()); } // Manual Buzzer
BLYNK_WRITE(V1) { myservo.write(param.asInt()); }          // Manual Servo control
BLYNK_WRITE(V3) { updateOLED("Terminal:", param.asString()); } // Terminal to OLED

// V6: Trigger Auto Pill Dispense Routine from Blynk Dashboard Button
BLYNK_WRITE(V6) {
    int trigger = param.asInt();
    if (trigger == 1) { // Triggers when button is pressed down
        dispensePillSequence();
    }
}

void loop() {
    Blynk.run();
    timer.run();
    
    // Door Logic
    static int lastState = -1;
    int currentState = digitalRead(doorSensorPin);
    if (currentState != lastState) {
        Blynk.virtualWrite(V2, currentState);
        if(currentState == LOW) {
            updateOLED("Alert!", "Door Opened");
        }
        lastState = currentState;
    }
}
