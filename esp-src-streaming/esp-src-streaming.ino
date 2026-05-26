#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "secrets.h"
#include "pins.h"
#include <Wire.h>
#include <BH1750.h>

// Declaring sensor variables
// PIR
int motionVal = 0;
int pirState = LOW;
unsigned long motionStartTime = 0;
const unsigned long ON_DURATION = 5000;

// LUX

unsigned long previousMillisLux = 0;
const long intervalLux = 5000;
float lux;

// STATUS

bool lastPresenceSent = false;
bool currentPresence = false;

// LED
#define pwmChannel 0
bool ledState = false;
int brightness = 0;

// MODE
int currentMode = 0;


BH1750 lightMeter;

// Firebase objects
FirebaseData fbdo;
FirebaseData stream;
FirebaseAuth auth;
FirebaseConfig config;

// STREAM CALLBACK
void streamCallback(FirebaseStream data)
{
  String path = data.dataPath();
  if(path == "/mode"){
    currentMode = data.intData();
    Serial.println("Mode: " + String(currentMode));
  }

  if(path == "/status/ledState"){
    ledState = data.boolData();
    digitalWrite(LED_PIN, ledState);
    Serial.println("LED State: " + String(ledState));
  }

  if(path == "/manual/brightness"){
    brightness = data.intData();
    Serial.println("Brightness: " + String(brightness));
  }
  Serial.println("Stream data received...");
}

// Stream timeout callback
void streamTimeoutCallback(bool timeout)
{
  if (timeout)
    Serial.println("Stream timeout, reconnecting...");
}

///////// READ LUX SENSOR //////////

void readPIRSensor(){
  motionVal = digitalRead(PIR_PIN);
  if (motionVal == HIGH){
    if (pirState == LOW){
      Serial.println("Motion detected!");
      pirState = HIGH;
      currentPresence = true;
      motionStartTime = millis();
    }
  }
  if(pirState == HIGH){
    if(millis() - motionStartTime >= ON_DURATION){
      pirState = LOW;
      currentPresence = false;
    }
  }
  if (currentPresence != lastPresenceSent){
    Firebase.RTDB.setBool(&fbdo, "/status/motion", currentPresence);
    Serial.print("Presence set to ");
    Serial.println(currentPresence);
    lastPresenceSent = currentPresence;
  }
}

////////// READ LUX SENSOR //////////

void readLuxSensor(){
  unsigned long currentMillisLux = millis();
  if (currentMillisLux - previousMillisLux >= intervalLux){
    previousMillisLux = currentMillisLux;
    lux = lightMeter.readLightLevel();
    Serial.print("Light: ");
    Serial.println(lux);
    Firebase.RTDB.set(&fbdo, "/status/lux", lux);
  }
}

// SET LED BRIGHTNESS

void setBrightness(int percent){
  int pwm = map(percent, 0, 100, 0, 255);
  ledcWrite(LED_PIN, pwm);
}

// HANDLE AUTO MODE

void handleAuto()
{
  if (lux < 40)
  {
    if (currentPresence == true)
    {
      setBrightness(100);
    }
    else
    {
      setBrightness(0);
    }
  }
  else
  {
    setBrightness(0);
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(PIR_PIN, INPUT);

  ledcAttach(LED_PIN, 5000, 8); // 5000 e frecventa, 8 e rezolutia

  Wire.begin();
  lightMeter.begin();

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }

  Serial.println();
  Serial.println("Connected!");

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.reconnectNetwork(true);
  fbdo.setBSSLBufferSize(4096, 1024);
  fbdo.setResponseSize(2048);

  Firebase.begin(&config, &auth);
  Firebase.setDoubleDigits(5);

  if (!Firebase.RTDB.beginStream(&stream, "/"))
  {
    Serial.println("Could not begin stream");
    Serial.println(stream.errorReason());
  }
  else
  {
    Serial.println("Stream started successfully");
  }

  Firebase.RTDB.setStreamCallback(&stream, streamCallback, streamTimeoutCallback);
}

void loop()
{
  if (!Firebase.ready())
  {
    Serial.println("Firebase not ready...");
  }
  if(currentMode == 0){
    if(ledState)
      setBrightness(brightness);
    else setBrightness(0);
  }
  else{
    // automatic
    readPIRSensor();
    readLuxSensor();
  }
}
