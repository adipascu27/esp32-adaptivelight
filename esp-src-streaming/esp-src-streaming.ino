#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "secrets.h"
#include "pins.h"
#include <Wire.h>
#include <BH1750.h>
#include <NeoPixelBus.h>

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
bool ledState = false;
int brightness = 0;
int red=255;
int green=0;
int blue=0;
int last_color[4]={255,0,0};

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(NUM_LEDS, RGB_PIN);

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
  }else if(path == "/status/ledState"){
    ledState = data.boolData();
    digitalWrite(LED_PIN, ledState);
    Serial.println("LED State: " + String(ledState));
  }else if(path == "/manual/brightness"){
    brightness = data.intData();
    Serial.println("Brightness: " + String(brightness));
  }
  if(path == "/manual/color"){
    FirebaseJson &json = data.jsonObject();
    
    FirebaseJsonData result;
    if (json.get(result, "r")) red = result.intValue;
    if (json.get(result, "g")) green = result.intValue;
    if (json.get(result, "b")) blue = result.intValue;
    
    Serial.println("R:" + String(red) + " G:" + String(green) + " B:" + String(blue));
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

// HANDLE AUTO MODE

void handleAuto()
{
  if (lux < 40)
  {
    if (currentPresence == true)
    {
      //setbrightness full
    }
    else
    {
      //setbrightness 0
    }
  }
  else
  {
    //setbrightness 0
  }
}

// UPDATE COLOR

void updateColor(int r, int g, int b, int percent){
  int scale = map(percent, 0, 100, 0, 255);
  RgbColor color(
    (r * scale) / 255,
    (g * scale) / 255,
    (b * scale) / 255
  );
  for (int i = 0; i < NUM_LEDS; i++){
    strip.SetPixelColor(i, color);
  }
  strip.Show();
}

void clearColor(){
  strip.ClearTo(RgbColor(0,0,0));
  strip.Show();
}


void setup()
{

  Serial.begin(115200);

  // Initializing sensors and LED strip
  pinMode(PIR_PIN, INPUT);
  strip.Begin();
  strip.Show();
  Wire.begin();
  lightMeter.begin();

// Waiting for Wi-Fi connection

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.println("Connected!");

// Initializing Firebase connection

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
    if(ledState){
      updateColor(red, green, blue, brightness);

    }
    else{
      clearColor();
    }
  }
  else{
    // automatic
    readPIRSensor();
    readLuxSensor();
  }
}