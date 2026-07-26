#include <WiFi.h>
#include <WebServer.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

// ==========================================
// 1. PIN DEFINITIONS 
// ==========================================
// Motors (1x L298N driving 4 wheels)
const int ENA_LEFT = 25; 
const int IN1_LEFT = 26; 
const int IN2_LEFT = 27; 
const int IN3_RIGHT = 14; 
const int IN4_RIGHT = 5; 
const int ENB_RIGHT = 13;

// Sensors
const int TRIG_PIN = 4;
const int ECHO_FRONT = 39;
const int SOUND_PIN = 35; // Digital Out from Sound Sensor

// GPS (Using Hardware Serial 2 on ESP32)
const int RXD2 = 16;
const int TXD2 = 17;

// ==========================================
// 2. OBJECTS & STATE VARIABLES
// ==========================================
WebServer server(80);
Adafruit_MPU6050 mpu;
TinyGPSPlus gps;
HardwareSerial gpsSerial(2); // Explicitly create our own Serial port for the GPS

// Sensor State Variables
float currentLat = 0.0;
float currentLng = 0.0;
long currentDistance = 0;
bool soundDetected = false;
float currentTilt = 0.0;
String robotState = "BOOTING...";

// Speed Variables
int baseSpeed = 150;  // Normal driving speed
int boostSpeed = 255; // Max speed for climbing over rubble

// ==========================================
// 3. WEBSERVER HTML DESIGN
// ==========================================
void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
  html += "<meta http-equiv=\"refresh\" content=\"2\">"; // Refresh every 2 seconds
  html += "<style>body{background-color:#1e1e1e;color:#00ffcc;font-family:Arial,sans-serif;text-align:center;padding:20px;}";
  html += "h1{color:#ffcc00;} .card{background:#2d2d2d;border:2px solid #00ffcc;padding:15px;margin:15px auto;width:90%;border-radius:10px;box-shadow:0px 0px 15px #00ffcc;}";
  html += ".alert{color:#ff3333;font-weight:bold;font-size:1.2em;} .safe{color:#33ff33;} ";
  html += "a.btn{background:#00BFFF;color:#fff;padding:10px 20px;text-decoration:none;border-radius:5px;display:inline-block;margin-top:10px;}</style></head><body>";
  
  html += "<h1>🚨 ResQBot Dashboard 🚨</h1>";
  
  // GPS Section
  html += "<div class='card'><h2>📍 Live Location</h2>";
  if (currentLat != 0.0) {
    html += "<p>LAT: " + String(currentLat, 6) + "<br>LON: " + String(currentLng, 6) + "</p>";
    html += "<a class='btn' href=\"https://www.google.com/maps/search/?api=1&query=" + String(currentLat, 6) + "," + String(currentLng, 6) + "\" target=\"_blank\">🗺️ OPEN IN MAPS</a>";
  } else {
    html += "<p class='alert'>Searching for GPS Satellites...</p>";
  }
  html += "</div>";

  // Robot Status Section
  html += "<div class='card'><h2>🤖 Systems Status</h2>";
  html += "<p>Current Action: <strong>" + robotState + "</strong></p>";
  html += "<p>Obstacle Distance: " + String(currentDistance) + " cm</p>";
  
  if (soundDetected) {
    html += "<p class='alert'>⚠️ SURVIVOR SOUND DETECTED!</p>";
  } else {
    html += "<p class='safe'>✅ Audio Levels Normal</p>";
  }
  
  if (abs(currentTilt) > 3.0) {
    html += "<p class='alert'>⚠️ ROUGH TERRAIN (Torque Boost Active)</p>";
  }
  html += "</div></body></html>";

  server.send(200, "text/html", html);
}

// ==========================================
// 4. MOTOR CONTROL FUNCTIONS
// ==========================================
void driveForward(int speed) {
  digitalWrite(IN1_LEFT, HIGH); digitalWrite(IN2_LEFT, LOW);
  digitalWrite(IN3_RIGHT, HIGH); digitalWrite(IN4_RIGHT, LOW);
  analogWrite(ENA_LEFT, speed); analogWrite(ENB_RIGHT, speed);
}

void stopMotors() {
  digitalWrite(IN1_LEFT, LOW); digitalWrite(IN2_LEFT, LOW);
  digitalWrite(IN3_RIGHT, LOW); digitalWrite(IN4_RIGHT, LOW);
  analogWrite(ENA_LEFT, 0); analogWrite(ENB_RIGHT, 0);
}

void turnRight(int speed) {
  digitalWrite(IN1_LEFT, HIGH); digitalWrite(IN2_LEFT, LOW);
  digitalWrite(IN3_RIGHT, LOW); digitalWrite(IN4_RIGHT, HIGH); // Right side reverse
  analogWrite(ENA_LEFT, speed); analogWrite(ENB_RIGHT, speed);
}

// ==========================================
// 5. SETUP FUNCTION
// ==========================================
void setup() {
  Serial.begin(115200);
  gpsSerial.begin(9600, SERIAL_8N1, RXD2, TXD2); // Init GPS

  // Pin Modes
  pinMode(ENA_LEFT, OUTPUT); pinMode(IN1_LEFT, OUTPUT); pinMode(IN2_LEFT, OUTPUT);
  pinMode(ENB_RIGHT, OUTPUT); pinMode(IN3_RIGHT, OUTPUT); pinMode(IN4_RIGHT, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT); pinMode(ECHO_FRONT, INPUT); pinMode(SOUND_PIN, INPUT);

  stopMotors(); // Ensure motors are off initially

  // Init MPU6050
  Serial.println("Initializing MPU6050...");
  Wire.begin(21, 22);
  if (!mpu.begin()) {
    Serial.println("MPU6050 NOT FOUND! Check wiring.");
  } else {
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  }

  // Create WiFi Access Point
  Serial.println("Creating Rescue WiFi Network...");
  WiFi.softAP("ResQBot_Emergency", "rescue1234"); 
  IPAddress IP = WiFi.softAPIP();
  Serial.print("Web Server Online at IP: ");
  Serial.println(IP); // Should be 192.168.4.1

  server.on("/", handleRoot);
  server.begin();
  
  robotState = "SYSTEMS ONLINE";
  delay(2000);
}

// ==========================================
// 6. MAIN LOOP
// ==========================================
void loop() {
  // 1. Keep the Web Server Alive
  server.handleClient();

  // 2. Read GPS Data Non-Blocking
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      if (gps.location.isValid()) {
        currentLat = gps.location.lat();
        currentLng = gps.location.lng();
      }
    }
  }

  // 3. Read Ultrasonic Sensor
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10); digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_FRONT, HIGH, 30000); // 30ms timeout
  if (duration == 0) {
    currentDistance = 999; // No obstacle
  } else {
    currentDistance = (duration * 0.034 / 2);
  }

  // 4. Read Sound Sensor
  soundDetected = (digitalRead(SOUND_PIN) == HIGH); 

  // 5. Read MPU6050 Tilt
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  currentTilt = a.acceleration.x; 
  
  int activeSpeed = baseSpeed;

  // 6. Navigation & AI Logic
  if (abs(currentTilt) > 3.0) {
    activeSpeed = boostSpeed; 
  }

  if (currentDistance < 25 && currentDistance > 0) { 
    robotState = "EVADING OBSTACLE";
    stopMotors();
    delay(200);
    turnRight(activeSpeed);
    delay(450); 
  } else {
    robotState = "PATROLLING";
    driveForward(activeSpeed);
  }
}
