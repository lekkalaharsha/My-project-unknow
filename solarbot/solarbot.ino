#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// ==== WiFi & Firebase Credentials ====
#define FIREBASE_HOST "harsha2-7efc2-default-rtdb.firebaseio.com/"
#define FIREBASE_AUTH "kwBJuiP1UMCx14zLC1WLYjrxf2Li6QvozgkATVVE"
const char* ssid     = "H";
const char* password = "12345678";

// ==== Motor & Sensor Pins ====
const int ENA = D8;
const int ENB = A0;
const int a1 = D0, a2 = D1, b1 = D2, b2 = D3;
const int trig1 = D4, echo1 = D5;      // Sensor 1 (Front)
const int trig2 = D6, echo2 = D7;      // Sensor 2 (Back)

// ==== Other Constants ====
#define SOUND_VELOCITY 0.034  // cm/us

FirebaseData firebaseData;
FirebaseData commandData, speedData;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "in.pool.ntp.org", 19800, 60000); // +5:30 IST

unsigned long lastReadMillis = 0;
const unsigned long readInterval = 200; // ms

// ==== Function Prototypes ====
float readDistanceCM(int trigPin, int echoPin);
void moveForward(int speed);
void moveBackward(int speed);
void stopMotors();

void setup() {
  Serial.begin(115200);

  pinMode(ENA, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(a1, OUTPUT);
  pinMode(a2, OUTPUT);
  pinMode(b1, OUTPUT);
  pinMode(b2, OUTPUT);
  pinMode(trig1, OUTPUT);
  pinMode(echo1, INPUT);
  pinMode(trig2, OUTPUT);
  pinMode(echo2, INPUT);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected!");

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  timeClient.begin();
}

void loop() {
  timeClient.update();
  String nowTime = timeClient.getFormattedTime();

  // ---- Periodic Firebase Read (every readInterval ms) ----
  if (millis() - lastReadMillis > readInterval) {
    lastReadMillis = millis();

    // Fetch command and speed from Firebase
    if (!Firebase.getString(commandData, "/FirebaseIOT/man")) {
      Serial.println("Failed to get /man");
      return;
    }
    if (!Firebase.getInt(speedData, "/FirebaseIOT/speed")) {
      Serial.println("Failed to get /speed");
      return;
    }
    String command = commandData.stringData();
    int carSpeed = speedData.intData();
    carSpeed = constrain(carSpeed, 0, 255);

    // Read front and back ultrasonic sensors
    float frontDist = readDistanceCM(trig1, echo1);
    float backDist = readDistanceCM(trig2, echo2);

    Firebase.setFloat(firebaseData, "/FirebaseIOT/front_distance", frontDist);
    Firebase.setFloat(firebaseData, "/FirebaseIOT/back_distance", backDist);

    // ---- Car Logic ----
    // Manual control: "1"=forward, "2"=backward, "0"=stop
    // Auto-mode (if you want: use a specific time window)
    bool autoMode = (nowTime > "01:00:20" && nowTime <= "05:34:20");

    String status;
    if ((command == "1" || autoMode) && frontDist > 10.0) {
      moveForward(carSpeed);
      status = "forward";
    } else if ((command == "2" || autoMode) && backDist > 5.0) {
      moveBackward(carSpeed);
      status = "backward";
    } else {
      stopMotors();
      status = "stopped";
    }

    Firebase.setString(firebaseData, "/FirebaseIOT/status", status);

    // ---- Debug ----
    Serial.printf("Status: %s, Cmd: %s, Spd: %d, F: %.1fcm, B: %.1fcm, Time: %s\n",
      status.c_str(), command.c_str(), carSpeed, frontDist, backDist, nowTime.c_str());
  }
}

// ==== Utility Functions ====
float readDistanceCM(int trigPin, int echoPin) {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  long duration = pulseIn(echoPin, HIGH, 25000); // Timeout for stability
  float distance = duration * SOUND_VELOCITY / 2;
  return (duration == 0) ? 999.0 : distance; // 999cm = no reading
}

void moveForward(int speed) {
  digitalWrite(a1, HIGH); digitalWrite(a2, LOW);
  digitalWrite(b1, HIGH); digitalWrite(b2, LOW);
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}

void moveBackward(int speed) {
  digitalWrite(a1, LOW); digitalWrite(a2, HIGH);
  digitalWrite(b1, LOW); digitalWrite(b2, HIGH);
  analogWrite(ENA, speed);
  analogWrite(ENB, speed);
}

void stopMotors() {
  digitalWrite(a1, LOW); digitalWrite(a2, LOW);
  digitalWrite(b1, LOW); digitalWrite(b2, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}
