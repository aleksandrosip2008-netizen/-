#include <Stepper.h>

#include <WiFi.h>
#include <WebServer.h>

// WiFi AP credentials
const char* ssid = "ESP32-AP";
const char* password = "12345678";
#include "DFRobotDFPlayerMini.h"
//#include <HardwareSerial.h>
//HardwareSerial Serial2(2); // RX:gpio16 TX:gpio17
int obezd = 0;
int t = 24;
#define HC_TRIG 4
#define HC_ECHO 2

Stepper stepper(510, 32, 0, 33, 15);

DFRobotDFPlayerMini myDFPlayer;
// Create web server on port 80
WebServer server(80);

// L298N motor driver pins
const int ENA = 26;  // Left motor PWM (speed)
const int IN1 = 27;  // Left motor direction 1
const int IN2 = 25;  // Left motor direction 2
const int ENB = 14;  // Right motor PWM (speed)
const int IN3 = 12;  // Right motor direction 1
const int IN4 = 13;  // Right motor direction 2

// Variables to store command and speed
String currentCommand = "";
int currentSpeed = 0;
const int defaultSpeed = 100; // Default speed when no value is provided

int getMm(uint8_t trig, uint8_t echo, int t) {
  // импульс 10 мкс
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);

  // измеряем время ответного импульса
  uint32_t us = pulseIn(echo, HIGH);

  // считаем расстояние и возвращаем
  return us * (t * 6 / 10 + 330) / 2000ul;
}

void setup() {
  // Start Serial
  Serial.begin(115200);
  delay(1000);
  Serial2.begin(9600, SERIAL_8N1, 16, 17);
  myDFPlayer.begin(Serial2);
  myDFPlayer.volume(25);
  pinMode(HC_TRIG, OUTPUT); // trig выход
  pinMode(HC_ECHO, INPUT);  // echo вход
  stepper.setSpeed(60);
  // Set up motor pins
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENB, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  myDFPlayer.play(1);
  // Set up Access Point
  WiFi.softAP(ssid, password);

  Serial.println();
  Serial.print("AP IP address: ");
  Serial.println(WiFi.softAPIP());

  // Define server routes
  server.onNotFound(handleCommand);

  // Start server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  int dist = getMm(HC_TRIG, HC_ECHO, t);    // получаем расстояние в мм
  delay(50);
  if (dist < 300 and obezd == 1){
    digitalWrite(IN1, LOW);   // Left motor backward
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);   // Right motor backward
    digitalWrite(IN4, LOW);
    analogWrite(ENA, currentSpeed);
    analogWrite(ENB, currentSpeed);
    stepper.step(255);
    int dist1 = getMm(HC_TRIG, HC_ECHO, t);    // получаем расстояние в мм
    delay(50);
    stepper.step(-510);
    int dist2 = getMm(HC_TRIG, HC_ECHO, t);    // получаем расстояние в мм
    delay(50);
    stepper.step(255);
    digitalWrite(32, LOW);
    digitalWrite(0, LOW);
    digitalWrite(33, LOW);
    digitalWrite(15, LOW);
    if (dist1 < dist2){
      digitalWrite(IN1, LOW);   // Left motor backward
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, LOW);   // Right motor backward
      digitalWrite(IN4, HIGH);
      analogWrite(ENA, currentSpeed);
      analogWrite(ENB, currentSpeed);
      delay(1000);
      digitalWrite(IN1, HIGH);  // Left motor forward
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, LOW);   // Right motor backward
      digitalWrite(IN4, HIGH);
      analogWrite(ENA, currentSpeed);
      analogWrite(ENB, currentSpeed);
      delay(1500);
      digitalWrite(IN1, HIGH);  // Left motor forward
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH);  // Right motor forward
      digitalWrite(IN4, LOW);
      analogWrite(ENA, currentSpeed);
      analogWrite(ENB, currentSpeed);
    }
    else {
      digitalWrite(IN1, LOW);   // Left motor backward
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, LOW);   // Right motor backward
      digitalWrite(IN4, HIGH);
      analogWrite(ENA, currentSpeed);
      analogWrite(ENB, currentSpeed);
      delay(1000);
      digitalWrite(IN1, LOW);  // Left motor forward
      digitalWrite(IN2, HIGH);
      digitalWrite(IN3, HIGH);   // Right motor backward
      digitalWrite(IN4, LOW);
      analogWrite(ENA, currentSpeed);
      analogWrite(ENB, currentSpeed);
      delay(1500);
      digitalWrite(IN1, HIGH);  // Left motor forward
      digitalWrite(IN2, LOW);
      digitalWrite(IN3, HIGH);  // Right motor forward
      digitalWrite(IN4, LOW);
      analogWrite(ENA, currentSpeed);
      analogWrite(ENB, currentSpeed);
    }
  }
}

void handleCommand() {
  String request = server.uri();

  if (request.startsWith("/")) {
    request = request.substring(1);
  }

  // Parse command and speed (if provided)
  if (request.indexOf(":") != -1) {
    int colonIndex = request.indexOf(":");
    currentCommand = request.substring(0, colonIndex);
    String speedStr = request.substring(colonIndex + 1);
    currentSpeed = speedStr.toInt();
  } else {
    currentCommand = request;
    currentSpeed = (currentCommand == "S") ? 0 : defaultSpeed; // Default speed unless "S"
  }

  // Process command and control robot
  controlRobot();
  displayCommand();

  // Send response back to client
  server.send(200, "text/plain", "Command received: " + request);
}

void controlRobot() {
  // Stop motors by default
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  if (currentCommand == "F") { // Forward
    digitalWrite(IN1, HIGH);  // Left motor forward
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);  // Right motor forward
    digitalWrite(IN4, LOW);
    analogWrite(ENA, currentSpeed);
    analogWrite(ENB, currentSpeed);
  } 
  else if (currentCommand == "B") { // Backward
    digitalWrite(IN1, LOW);   // Left motor backward
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);   // Right motor backward
    digitalWrite(IN4, HIGH);
    analogWrite(ENA, currentSpeed);
    analogWrite(ENB, currentSpeed);
  } 
  else if (currentCommand == "L") { // Left
    digitalWrite(IN1, LOW);   // Left motor backward
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, HIGH);  // Right motor forward
    digitalWrite(IN4, LOW);
    analogWrite(ENA, currentSpeed);
    analogWrite(ENB, currentSpeed);
  } 
  else if (currentCommand == "next") {
    myDFPlayer.next();
    delay(1000);
  }
  else if (currentCommand == "last") {
    myDFPlayer.previous();
    delay(1000);
  }
  else if (currentCommand == "+") {
    myDFPlayer.volumeUp();
    delay(1000);
  }
  else if (currentCommand == "-") {
    myDFPlayer.volumeDown();
    delay(1000);
  }
  else if (currentCommand == "h+") {
    obezd = 1;
  }
  else if (currentCommand == "h-") {
    obezd = 0;
  }
  else if (currentCommand == "R") { // Right
    digitalWrite(IN1, HIGH);  // Left motor forward
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, LOW);   // Right motor backward
    digitalWrite(IN4, HIGH);
    analogWrite(ENA, currentSpeed);
    analogWrite(ENB, currentSpeed);
  } 
  else if (currentCommand == "S") { // Stop
    analogWrite(ENA, 0);
    analogWrite(ENB, 0);
  }
}

void displayCommand() {
  Serial.println("-------------------");
  Serial.print("Received Command: ");
  Serial.println(currentCommand);

  if (currentSpeed > 0) {
    Serial.print("Speed Value: ");
    Serial.println(currentSpeed);
  }

  if (currentCommand == "F") {
    Serial.println("Action: Moving Forward");
  } else if (currentCommand == "B") {
    Serial.println("Action: Moving Backward");
  } else if (currentCommand == "L") {
    Serial.println("Action: Turning Left");
  } else if (currentCommand == "R") {
    Serial.println("Action: Turning Right");
  } else if (currentCommand == "S") {
    Serial.println("Action: Stop");
  }
  Serial.println("-------------------");
