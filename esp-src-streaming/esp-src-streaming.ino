#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "secrets.h"

// Firebase objects
FirebaseData fbdo;
FirebaseData stream;
FirebaseAuth auth;
FirebaseConfig config;

const int ledPin = 5;

// STREAM CALLBACK
void streamCallback(FirebaseStream data)
{
  Serial.println("Stream data received...");
  if (data.dataType() == "boolean")
  {
    int ledState = data.intData();
    digitalWrite(ledPin, ledState);
    Serial.print("LED State changed to: ");
    Serial.println(ledState);
  }
}

// Stream timeout callback
void streamTimeoutCallback(bool timeout)
{
  if (timeout)
    Serial.println("Stream timeout, reconnecting...");
}

void setup()
{
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  Serial.begin(115200);

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

  if (!Firebase.RTDB.beginStream(&stream, "/led"))
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
}
