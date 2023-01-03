#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"

#define DATABASE_URL "https://esp32-test-9f190-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define API_KEY "AIzaSyApMZLu1oA_Rib3EkymtEYloVpWKGC6cdU"
#define SOUND_SPEED 0.034
#define LED 2

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;
int count = 0;

const char* ssid = "HD72HT";
const char* password = "drimaon99";

int counter = 0;
long interval1 = 60000;
long previousMillis1 = 0;
long interval2 = 300;
long previousMillis2 = 0;

const int trigPin = 18;
const int echoPin = 5;

long duration;
float distanceCm;
long personpass = 0;

void setup() {
  Serial.begin(115200);

  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

   if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;  

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  pinMode(trigPin, OUTPUT);  
  pinMode(echoPin, INPUT);   
  pinMode(LED, OUTPUT);
  digitalWrite(LED,LOW);  
}
void loop() {
  if (millis() - previousMillis1 > interval1 || previousMillis1 == 0) {
    previousMillis1 = millis();
    if (WiFi.status() == WL_CONNECTED) {  //if we are connected to Eduroam network
      counter = 0;                        //reset counter
      Serial.println("Wifi is still connected with IP: ");
      Serial.println(WiFi.localIP());            //inform user about his IP address
      digitalWrite(LED,HIGH);
    } else if (WiFi.status() != WL_CONNECTED) {  //if we lost connection, retry
      WiFi.begin(ssid);
    }
    while (WiFi.status() != WL_CONNECTED) {  //during lost connection, print dots
      delay(500);
      Serial.print(".");
      counter++;
      if (counter >= 60) {  //30 seconds timeout - reset board
        ESP.restart();
      }
    }
  }
  if (Firebase.ready() && signupOK && (millis() - previousMillis2 > interval2 || previousMillis2 == 0)) {
    previousMillis2 = millis();
    readDistance();
  }
}

void readDistance() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  duration = pulseIn(echoPin, HIGH);
  distanceCm = duration * SOUND_SPEED / 2;
  Firebase.RTDB.setInt(&fbdo, "HCSR-04/distanceCM", distanceCm);
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);
  if (distanceCm <= 150) {
    personpass++;
    Firebase.RTDB.setInt(&fbdo, "HCSR-04/person", personpass);
    Serial.print("person: ");
    Serial.println(personpass);
  }
}