#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "secrets.h"
#include "pins.h"
#include <Wire.h>
#include <BH1750.h>
#include <NeoPixelBus.h>
#include <time.h>

// GLOBAL VARIABLES FOR LED MODES

int colorMode = 0;       // 0=static, 1=breathing, 2=strobe, 3=smooth, 4=wave
int animSpeed = 100;      // 0-100, controlled from React interface

unsigned long lastAnimStep = 0;
float breathVal = 0.0f;
float breathDir = 1.0f;
int smoothHue = 0;
int waveOffset = 0;

// Declaring sensor variables
// PIR
int motionVal = 0;
int pirState = LOW;
unsigned long motionStartTime = 0;
const unsigned long ON_DURATION = 5000;

// LUX
unsigned long previousMillisLux = 0;
const long intervalLux = 2000;
float currentLux;

// STATUS
bool lastPresenceSent = false;
bool currentPresence = false;

// LED
bool ledState = true;
int NUM_LEDS=15;
int brightness = 25;
int red=255;
int green=0;
int blue=0;
int last_color[4]={255,0,0};

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(NUM_LEDS, RGB_PIN);

// AUTO MODE VARIABLES
bool darkState = false;           // light state
unsigned long lastMotionTime = 0; // millis() at last PIR detection
bool autoLedOn = false;           // LED state in auto mode
const unsigned long AUTO_TIMEOUT = 15UL * 60UL * 1000UL; // 15 min in ms
const float LUX_DARK  = 40.0f;
const float LUX_LIGHT = 60.0f;

// MODE
int currentMode = 0;
unsigned long lastStatusPush = 0;
const unsigned long STATUS_INTERVAL = 2000; // Firebase push every 5 seconds

BH1750 lightMeter;

// Firebase objects
FirebaseData fbdo;
FirebaseData stream;
FirebaseAuth auth;
FirebaseConfig config;
int firebaseTimeouts=0;

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
  }else if(path == "/manual/color"){
    FirebaseJson &json = data.jsonObject();
    
    FirebaseJsonData result;
    if (json.get(result, "r")) red = result.intValue;
    if (json.get(result, "g")) green = result.intValue;
    if (json.get(result, "b")) blue = result.intValue;
    
    Serial.println("R:" + String(red) + " G:" + String(green) + " B:" + String(blue));
  }else if (path == "/manual/colorMode") {
    colorMode = data.intData();
    // Reset stare animație la schimbarea modului
    breathVal = 0.0f;
    smoothHue = 0;
    waveOffset = 0;
    strip.ClearTo(RgbColor(0, 0, 0));
    strip.Show();
  }else if (path == "/manual/animSpeed") {
      animSpeed = data.intData();
      Serial.println("Animation speed set to: " + String(animSpeed));
  }
  Serial.println("Stream data received...");
}

// Stream timeout callback
void streamTimeoutCallback(bool timeout)
{
  if (timeout){
    firebaseTimeouts++;
    Serial.println(firebaseTimeouts);
    Serial.println("Stream timeout, reconnecting...");
    if(firebaseTimeouts > 1){
      Serial.println("Timeouted for more than 4 times. Restarting ESP!");
      ESP.restart();
    }
  }
}

// HSV TO RGB CONVERSION

RgbColor hsvToRgb(int hue, int sat, int val) {
    // hue: 0-359, sat: 0-255, val: 0-255
    float h = hue / 60.0f;
    float s = sat / 255.0f;
    float v = val / 255.0f;
    float c = v * s;
    float x = c * (1.0f - fabs(fmod(h, 2.0f) - 1.0f));
    float m = v - c;
    float r = 0, g = 0, b = 0;
    if      (h < 1) { r = c; g = x; }
    else if (h < 2) { r = x; g = c; }
    else if (h < 3) { g = c; b = x; }
    else if (h < 4) { g = x; b = c; }
    else if (h < 5) { r = x; b = c; }
    else            { r = c; b = x; }
    return RgbColor(
        (uint8_t)((r + m) * 255),
        (uint8_t)((g + m) * 255),
        (uint8_t)((b + m) * 255)
    );
}

// TIME SYNCING

void syncTime() {
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  Serial.print("Syncing time");
  time_t now = time(nullptr);
  while (now < 1000000000) {
    Serial.print(".");
    delay(500);
    now = time(nullptr);
  }
  Serial.println("\nTime synced: " + String(ctime(&now)));
}


// ANIMATION SPEED

unsigned long animInterval() {
    // animSpeed 0 -> 200ms, animSpeed 100 -> 10ms
    return map(animSpeed, 0, 100, 200, 10);
}

// ANIMATION FUNCTIONS

void animBreathing() {
    if (millis() - lastAnimStep < animInterval()) return;
    lastAnimStep = millis();

    // breathVal oscilating 0.0 -> 1.0 -> 0.0
    breathVal += breathDir * 0.02f;
    if (breathVal >= 1.0f) { breathVal = 1.0f; breathDir = -1.0f; }
    if (breathVal <= 0.0f) { breathVal = 0.0f; breathDir =  1.0f; }

    int scale = (int)(breathVal * brightness / 100.0f * 255);
    RgbColor c(
        (red * scale) / 255,
        (green * scale) / 255,
        (blue * scale) / 255
    );
    strip.ClearTo(c);
    strip.Show();
}

void animStrobe() {
    if (millis() - lastAnimStep < animInterval()*5) return;
    lastAnimStep = millis();

    static int strobeStep = 0;
    // 0=red, 1=green, 2=blue
    RgbColor colors[3] = {
        RgbColor((255*brightness)/255, 0, 0),
        RgbColor(0, (255*brightness)/255, 0),
        RgbColor(0, 0, (255*brightness)/255)
    };
    strip.ClearTo(colors[strobeStep]);
    strip.Show();
    strobeStep = (strobeStep + 1) % 3;
}

void animSmooth() {
    if (millis() - lastAnimStep < animInterval()) return;
    lastAnimStep = millis();

    RgbColor c = hsvToRgb(smoothHue, 255, (int)(brightness / 100.0f * 255));
    strip.ClearTo(c);
    strip.Show();
    smoothHue = (smoothHue + 1) % 360;
}

void animWave() {
    if (millis() - lastAnimStep < animInterval()) return;
    lastAnimStep = millis();

    for (int i = 0; i < NUM_LEDS; i++) {
        // Every LED has a different hue offset with the LED next to it
        int hue = (smoothHue + (i * 360 / NUM_LEDS)) % 360;
        strip.SetPixelColor(i, hsvToRgb(hue, 255, (int)(brightness / 100.0f * 255)));
    }
    strip.Show();
    smoothHue = (smoothHue + 2) % 360;
}

// HANDLE AUTO MODE

void handleAuto()
{
  unsigned long now = millis();
  unsigned long currentMillisLux = millis();

  // READING LUX SENSOR EVERY 2 SECONDS

  if (currentMillisLux - previousMillisLux >= intervalLux){
    previousMillisLux = currentMillisLux;
    currentLux = lightMeter.readLightLevel();
    Serial.println("Light: " + String(currentLux));
  }

  // CHECKING LIGHT INTERVAL

  if (currentLux < LUX_DARK)  darkState = true;
  if (currentLux > LUX_LIGHT) darkState = false;

  // PIR CHECKED ONLY WHEN LIGHT IS THE INTERVAL

  bool pirActive = digitalRead(PIR_PIN) == HIGH;

  if (darkState && pirActive) {
      lastMotionTime = now;   // TIMER RESET AT EVERY PIR DETECTION
      currentPresence = true;
  }

  // DECIDING LED STATE
  bool shouldBeOn = darkState &&
                        currentPresence &&
                        (now - lastMotionTime < AUTO_TIMEOUT);

      if (!darkState || (now - lastMotionTime >= AUTO_TIMEOUT)) {
          currentPresence = false;
      }

      // Adapting light intensity depending on ambient light
      // Cu cât e mai întuneric, cu atât banda e mai puternică
      int autoBrightness = 100;
      if (darkState) {
          // lux 0-40 → brightness 100-60%
          autoBrightness = map((int)currentLux, 0, 40, 100, 60);
          autoBrightness = constrain(autoBrightness, 60, 100);
      }

      // APPLYING LED STATE
      if (shouldBeOn != autoLedOn) {
          autoLedOn = shouldBeOn;
      }

      if (autoLedOn) {
          strip.ClearTo(RgbColor((255*autoBrightness)/255,(255*autoBrightness)/255,(255*autoBrightness)/255));
          strip.Show();
      }else {
          strip.ClearTo(RgbColor(0, 0, 0));
          strip.Show();
      }

      // PUSHING DATA TO FIREBASE EVERY 5 SECONDS
      if (now - lastStatusPush >= STATUS_INTERVAL) {
          lastStatusPush = now;

          // Time left until light is turned off
          long remaining = 0;
          if (autoLedOn && lastMotionTime > 0) {
              long elapsed = (long)(now - lastMotionTime);
              remaining = max(0L, (long)(AUTO_TIMEOUT / 1000) - elapsed / 1000);
          }

          Firebase.RTDB.setFloat(&fbdo,  "/status/lux",          currentLux);
          Firebase.RTDB.setBool(&fbdo,   "/status/motion",        currentPresence);
          Firebase.RTDB.setBool(&fbdo,   "/status/ledState",      autoLedOn);
          Firebase.RTDB.setInt(&fbdo,    "/status/remaining",     (int)remaining);
          Firebase.RTDB.setBool(&fbdo,   "/status/isDark",        darkState);

          // Timestamp for last detection Unix ms
          if (lastMotionTime > 0 && currentPresence) {
              time_t now_t = time(nullptr);
              long secSinceMotion = (long)(millis() - lastMotionTime) / 1000;
              Firebase.RTDB.setInt(&fbdo, "/status/lastMotion",
                                  (int)(now_t - secSinceMotion));
          }
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
  clearColor();
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
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
  Serial.println();
  Serial.println("Connected!");

// TIME SYNC
  syncTime();

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
      switch (colorMode)
      {
      case 0:
        updateColor(red, green, blue, brightness);
        break;
      case 1:
        animBreathing();
        break;
      case 2:
        animStrobe();
        break;
      case 3:
        animSmooth();
        break;
      case 4:
        animWave();
        break;
      }
    }
    else{
      clearColor();
    }
  }
  else{
    // automatic
    handleAuto();
  }
}