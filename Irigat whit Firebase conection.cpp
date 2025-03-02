#include <WiFi.h>
#include "time.h"
#include <Arduino.h>
#include <string>
#include <iostream>
#include <FirebaseESP32.h>
#include "addons/RTDBHelper.h"
#include "addons/TokenHelper.h"

#define FIREBASE_HOST "link to Firebase"
#define FIREBASE_API_KEY "Firebase API Key"
#define USER_EMAIL "email"
#define USER_PASSWORD "password"
#define STARTER 19

const char *ssid = "wifi name";
const char *password = "wifi password";

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

const char *ntpServer = "ntp.lonelybinary.com";
const long gmtOffset_sec = 3600L * 3;
const int daylightOffset_sec = 0;

void connectWiFi();
int previousIndex = 0;
int currentIndex = 0;
int index2 = 0;
long timer = 0;
bool start = true;
bool dataTime = false;
bool checkTime(int min);
long interval;
bool stopTime = false;
void startValve();
void duration(long interval);
void synchronizeTime();
void initializeFirebase();
void streamTimeoutCallback(bool timeout);
void streamCallback(StreamData data);
void fetchInitialValues();

void setup(){
    Serial.begin(9600);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(STARTER, OUTPUT);
    digitalWrite(STARTER, HIGH);

    connectWiFi();
    initializeFirebase();
    fetchInitialValues();
    Firebase.setStreamCallback(fbdo, streamCallback, streamTimeoutCallback);
}

void loop()
{
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    int hour = timeinfo.tm_hour;
    int min = timeinfo.tm_min;
    bool clockCheck = checkTime(min);
    if(clockCheck){
        interval = millis() + timer;
        stopTime = true;
    }
    startValve();
}

bool checkTime(int min) {
    bool flag1 = false;
    if ((currentIndex == min && previousIndex == currentIndex - 1) && (start)) {
        flag1 = true;
        previousIndex = min;
    } else if ((index2 == min && previousIndex == index2 - 1) && (start)) {
        flag1 = true;
        previousIndex = min;
    } else {
        flag1 = false;
        previousIndex = min;
    }
    return flag1;
}

void startValve() {
    if(stopTime){
        duration(interval); 
    }
}

void duration(long interval) {
    if (millis() < interval) {
        digitalWrite(STARTER, LOW); 
    }
    else{
        digitalWrite(STARTER, HIGH);
        stopTime = false;
    } 
}

void connectWiFi(){
    boolean ledState = false;

    Serial.print("Connecting to WiFi network ");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
        ledState = !ledState;
        digitalWrite(LED_BUILTIN, ledState);
    }
    Serial.println("");
    synchronizeTime();
}

void synchronizeTime(){
    boolean ledState = false;
    Serial.print("Syncing time with NTP server ");
    struct tm timeinfo;
    while (!getLocalTime(&timeinfo))
    {
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        Serial.print(".");
        ledState = !ledState;
        digitalWrite(LED_BUILTIN, ledState);
    }

    Serial.println("");
    digitalWrite(LED_BUILTIN, true);
}

void initializeFirebase() {
    Serial.println("🔥 Initializing Firebase...");

    config.api_key = FIREBASE_API_KEY;
    config.database_url = FIREBASE_HOST;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;

    config.token_status_callback = tokenStatusCallback;
    Firebase.begin(&config, &auth);
    Firebase.reconnectWiFi(true);
    Firebase.beginStream(fbdo, "/");
}

void streamCallback(StreamData data) {
    Serial.println("📡 Firebase data updated!");
    Serial.print("📌 Changed data path: ");
    Serial.println(data.dataPath());

    if (data.dataType() == "int") {
        String path = data.dataPath();

        if (path == "/index") {
            currentIndex = data.intData();
        } else if (path == "/index2") {
            index2 = data.intData();
        } else if (path == "/timer") {
            timer = data.intData();
        } else if (path == "/start") {
            start = data.intData();
        }

        Serial.print("🔄 New value: ");
        Serial.println(data.intData());
    }
}

void streamTimeoutCallback(bool timeout) {
    if (timeout) {
        Serial.println("🔥 Firebase stream timeout, retrying...");
        Firebase.beginStream(fbdo, "/");
    }
}

void fetchInitialValues() {
    Serial.println("📥 Loading initial values from the database...");

    if (Firebase.getInt(fbdo, "/index")) {
        currentIndex = fbdo.intData();
        Serial.print("✔️ Loaded value for currentIndex: ");
        Serial.println(currentIndex);
    } else {
        Serial.print("❌ Error fetching /index: ");
        Serial.println(fbdo.errorReason());
    }

    if (Firebase.getInt(fbdo, "/index2")) {
        index2 = fbdo.intData();
        Serial.print("✔️ Loaded value for index2: ");
        Serial.println(index2);
    }

    if (Firebase.getInt(fbdo, "/timer")) {
        timer = fbdo.intData();
        Serial.print("✔️ Loaded value for timer: ");
        Serial.println(timer);
    }

     if (Firebase.getInt(fbdo, "/start")) {
        start = fbdo.intData();
        Serial.print("✔️ Loaded value for start: ");
        Serial.println(start);
    }
}
