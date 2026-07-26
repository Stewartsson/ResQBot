/*
  Fire Fighting Subsystem (ESP32-C3 Mini) - LITE VERSION
  Features: Servo sweeping, Flame detection, DHT telemetry, Tactical Web HUD.
  No pump/relay included to prevent power-draw crashes.
  NOTE: All emojis removed for pure English ASCII compatibility.
*/

#include <ESP32Servo.h>
#include "DHT.h"
#include <WiFi.h>
#include <WebServer.h>

// --- Offline Wireless Dashboard Configuration ---
const char* ssid = "ResQBot_FireNode";
const char* password = "rescueadmin"; // Password must be at least 8 characters
WebServer server(80); // Create web server on port 80

// --- Pin Definitions ---
const int SERVO_PIN = 4;
const int FLAME_SENSOR_PIN = 5;  
const int DHT_PIN = 7;

// --- Sensor Configurations ---
#define DHTTYPE DHT11   
DHT dht(DHT_PIN, DHTTYPE);

// --- Global Variables ---
Servo myServo;
int servoPos = 0;           
int servoDirection = 1;     
const int servoDelay = 15;  
unsigned long lastServoMoveTime = 0;

bool fireDetected = false;
unsigned long hazardPauseTime = 0;
const unsigned long hazardPauseDuration = 5000; // Stop and alert for 5 seconds

// Timer for DHT readings
unsigned long lastDHTReadTime = 0;
const unsigned long dhtInterval = 2000; // Read every 2 seconds

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("--- Starting TACTICAL Fire Subsystem (LITE) ---");

  // Initialize Pins
  pinMode(FLAME_SENSOR_PIN, INPUT);

  // Initialize Servo
  ESP32PWM::allocateTimer(0);
  myServo.setPeriodHertz(50); 
  myServo.attach(SERVO_PIN, 500, 2400); 

  // Initialize DHT Sensor
  dht.begin();
  
  // Initialize Local Wi-Fi Access Point
  Serial.println("\n--- Starting Local Tactical Dashboard ---");
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  
  Serial.print("1. Connect smartphone to Wi-Fi: ");
  Serial.println(ssid);
  Serial.print("2. Open browser and go to: http://");
  Serial.println(IP);

  // Link the web server to our dashboard page function
  server.on("/", handleRoot);
  server.begin(); // START THE SERVER
  Serial.println("Tactical HUD Online!");
  
  Serial.println("Setup Complete. Commencing radar sweep...");
}

void loop() {
  // Listen for incoming smartphone connections to update the HUD
  server.handleClient(); 

  // 1. Read DHT Sensor every 2 seconds without blocking the robot
  if (millis() - lastDHTReadTime >= dhtInterval) {
    lastDHTReadTime = millis();
    readSensorData();
  }

  // 2. Check for fire continuously
  checkFire();

  // 3. Handle physical actions
  if (!fireDetected) {
    sweepServo();
  } else {
    handleHazardPause();
  }
}

void readSensorData() {
  float h = dht.readHumidity();
  float t = dht.readTemperature(); 
  
  int flameRaw = digitalRead(FLAME_SENSOR_PIN);

  if (isnan(h) || isnan(t)) {
    Serial.println("[ERROR] Failed to read from DHT sensor!");
    return;
  }

  // Print data directly to the Arduino Serial Monitor for debugging
  Serial.print("[TELEMETRY] Temp: ");
  Serial.print(t);
  Serial.print(" C | Humidity: ");
  Serial.print(h);
  Serial.print(" % | Flame Status: ");
  
  // Clean English output without symbols
  if (flameRaw == HIGH) {
    Serial.println("FIRE DETECTED");
  } else {
    Serial.println("NO FIRE DETECTED");
  }
}

void sweepServo() {
  if (millis() - lastServoMoveTime >= servoDelay) {
    lastServoMoveTime = millis();
    servoPos += servoDirection;
    myServo.write(servoPos);

    if (servoPos >= 180) {
      servoDirection = -1; 
    } else if (servoPos <= 0) {
      servoDirection = 1;  
    }
  }
}

void checkFire() {
  // Read the flame sensor
  int flameStatus = digitalRead(FLAME_SENSOR_PIN);
  
  // Note: Most flame sensors output LOW when a fire is detected.
  if (flameStatus == HIGH && !fireDetected) { 
    Serial.println("\n====================================");
    Serial.println("*** CRITICAL HAZARD: FIRE DETECTED! ***");
    Serial.println("====================================");
    Serial.println("Halting servo to flag hazard zone!");
    
    fireDetected = true;
    hazardPauseTime = millis(); 
  }
}

void handleHazardPause() {
  if (millis() - hazardPauseTime >= hazardPauseDuration) {
    Serial.println("Hazard flagged. Resuming sector sweep...");
    Serial.println("------------------------------------");
    
    fireDetected = false;        
    delay(1000); // 1-second safety pause before moving again
  }
}

// --- Web Server Page Generation ---
void handleRoot() {
  // Grab the latest sensor readings
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  
  // Build a tactical "Disaster HUD" HTML website
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<meta http-equiv=\"refresh\" content=\"1\">"; // Auto-refresh the page every 1 second
  
  // Tactical CSS Styling
  html += "<style>";
  html += "body { background-color: #050505; color: #00ff00; font-family: 'Courier New', Courier, monospace; margin: 0; padding: 10px; }";
  html += ".hud-container { max-width: 600px; margin: auto; border: 2px solid #333; padding: 10px; background: #0a0a0a; box-shadow: 0 0 15px rgba(0, 255, 0, 0.1); }";
  html += ".hazard-tape { background: repeating-linear-gradient(45deg, #ffc107, #ffc107 10px, #000 10px, #000 20px); height: 15px; border-radius: 2px; margin-bottom: 15px; }";
  
  html += "h1 { text-align: center; color: #ffc107; text-transform: uppercase; font-size: 1.6em; text-shadow: 0 0 5px #ffc107; margin-top: 5px; }";
  html += ".panel { background: #111; border: 1px solid #333; margin-bottom: 15px; padding: 15px; text-align: center; position: relative; }";
  html += ".label { color: #888; font-size: 0.9em; letter-spacing: 2px; text-transform: uppercase; margin-bottom: 10px; }";
  html += ".data-val { font-size: 2.5em; font-weight: bold; margin: 10px 0; }";
  html += ".temp { color: #ff9800; text-shadow: 0 0 10px rgba(255, 152, 0, 0.5); }";
  html += ".hum { color: #03a9f4; text-shadow: 0 0 10px rgba(3, 169, 244, 0.5); }";
  
  html += ".safe-panel { border-left: 5px solid #00ff00; box-shadow: inset 10px 0 20px -10px rgba(0,255,0,0.2); }";
  html += ".danger-panel { border: 2px solid #ff0000; background: #3a0000; animation: strobe 0.5s infinite; box-shadow: 0 0 20px rgba(255,0,0,0.6); }";
  html += ".danger-text { color: #fff; font-size: 2em; font-weight: bold; text-shadow: 0 0 10px #fff; letter-spacing: 1px; }";
  html += "@keyframes strobe { 0% { opacity: 1; } 50% { opacity: 0.6; } 100% { opacity: 1; } }";
  html += "</style></head><body>";
  
  html += "<div class=\"hud-container\">";
  html += "<div class=\"hazard-tape\"></div>";
  html += "<h1>*** TACTICAL RESQ-HUD ***</h1>";
  
  // Display Fire Status (Pure English)
  if (fireDetected) {
    html += "<div class=\"panel danger-panel\">";
    html += "<div class=\"danger-text\">*** FIRE DETECTED ***</div>";
    html += "<div style=\"color: #ffaaaa; margin-top: 10px; font-size: 1.2em; font-weight: bold;\">>>> EMERGENCY HALT TRIGGERED <<<</div>";
    html += "</div>";
  } else {
    html += "<div class=\"panel safe-panel\">";
    html += "<div class=\"label\">Flame Sensor Status</div>";
    html += "<div style=\"color: #00ff00; font-size: 1.8em; font-weight: bold;\">[OK] NO FIRE DETECTED</div>";
    html += "<div style=\"color: #555; margin-top: 10px; font-size: 0.8em;\">RADAR SWEEPING FOR ANOMALIES...</div>";
    html += "</div>";
  }
  
  // Display Environment Data
  html += "<div class=\"panel\">";
  html += "<div class=\"label\">Environmental Telemetry</div>";
  if (isnan(t)) {
     html += "<div style=\"color: #ff0000; font-weight: bold; font-size: 1.5em; animation: strobe 1s infinite;\">*** SENSOR OFFLINE ***</div>";
  } else {
     html += "<div class=\"data-val temp\">" + String(t, 1) + " C</div>";
     html += "<div style=\"color: #555; font-size: 0.8em;\">AMBIENT TEMPERATURE</div><br>";
     html += "<div class=\"data-val hum\">" + String(h, 1) + " %</div>";
     html += "<div style=\"color: #555; font-size: 0.8em;\">RELATIVE HUMIDITY</div>";
  }
  html += "</div>";
  
  html += "<div class=\"hazard-tape\"></div>";
  html += "</div></body></html>";
  
  // Send the website to the phone
  server.send(200, "text/html", html);
}
