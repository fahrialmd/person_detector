#include <WiFi.h>      //Wifi library
#include "esp_wpa2.h"  //wpa2 library for connections to Enterprise networks
#include <Firebase_ESP_Client.h>
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

#define EAP_IDENTITY "achmadfahri.a"  //if connecting from another corporation, use identity@organisation.domain in Eduroam
#define EAP_USERNAME "achmadfahri.a"  //oftentimes just a repeat of the identity
#define EAP_PASSWORD "123CptHook@"    //your Eduroam password

const char* ssid = "WiFi-UB.x";        // Eduroam SSID
const char* host = "arduino.php5.sk";  //external server domain for HTTP connection after authentification
int counter = 0;
long interval1 = 60000;
long previousMillis1 = 0;
long interval2 = 300;
long previousMillis2 = 0;
#define DATABASE_URL "https://esp32-test-9f190-default-rtdb.asia-southeast1.firebasedatabase.app/"
#define API_KEY "AIzaSyApMZLu1oA_Rib3EkymtEYloVpWKGC6cdU"

//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signupOK = false;
int count = 0;

const int trigPin = 18;
const int echoPin = 5;

//define sound speed in cm/uS
#define SOUND_SPEED 0.034

long duration;
float distanceCm;
long personpass = 0;

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println();
  Serial.print("Connecting to network: ");
  Serial.println(ssid);
  WiFi.disconnect(true);  //disconnect form wifi to set new wifi connection
  WiFi.mode(WIFI_STA);    //init wifi mode

  WiFi.begin(ssid, WPA2_AUTH_PEAP, EAP_IDENTITY, EAP_USERNAME, EAP_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    counter++;
    if (counter >= 60) {  //after 30 seconds timeout - reset board
      ESP.restart();
    }
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address set: ");
  Serial.println(WiFi.localIP());  //print LAN IP

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  pinMode(trigPin, OUTPUT);  // Sets the trigPin as an Output
  pinMode(echoPin, INPUT);   // Sets the echoPin as an Input
}

void loop() {
  if (millis() - previousMillis1 > interval1 || previousMillis1 == 0) {
    previousMillis1 = millis();
    if (WiFi.status() == WL_CONNECTED) {  //if we are connected to Eduroam network
      counter = 0;                        //reset counter
      Serial.println("Wifi is still connected with IP: ");
      Serial.println(WiFi.localIP());            //inform user about his IP address
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
  // Clears the trigPin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin on HIGH state for 10 micro seconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);

  // Calculate the distance
  distanceCm = duration * SOUND_SPEED / 2;
  Firebase.RTDB.setInt(&fbdo, "distanceCM", distanceCm);
  Serial.print("Distance (cm): ");
  Serial.println(distanceCm);

  if (distanceCm <= 150) {
    personpass++;
    Firebase.RTDB.setInt(&fbdo, "person", personpass);
    Serial.print("person: ");
    Serial.println(personpass);
  }
}
