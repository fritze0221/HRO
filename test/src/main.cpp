
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson (use v6.xx)
#include <time.h>
#include <FastLED.h>
#include <iostream>
#include <Arduino.h>
#include <SPI.h>

#define emptyString String()
#define LED_PIN     23
#define NUM_LEDS    144


//Follow instructions from https://github.com/debsahu/ESP-MQTT-AWS-IoT-Core/blob/master/doc/README.md
//Enter values in secrets.h ▼
#include "secrets.h"



unsigned long t_millis;
unsigned long delta_t_millis; 
int check_led = 0;


CRGB leds[NUM_LEDS];


const int MQTT_PORT = 8883; 
const char MQTT_SUB_TOPIC[] = "$aws/things/LED-Streifen/shadow/update/delta";
const char MQTT_PUB_TOPIC[] = "$aws/things/LED-Streifen/shadow/update";

String POWER_STATE= "";
String HUE = "";
String SATURATION= "";
String BRIGHTNESS = "";


StaticJsonDocument<300> doc;

WiFiClientSecure net;

PubSubClient client(net);



void pubSubErr(int8_t MQTTErr)
{
  if (MQTTErr == MQTT_CONNECTION_TIMEOUT)
    Serial.print("Connection tiemout");
  else if (MQTTErr == MQTT_CONNECTION_LOST)
    Serial.print("Connection lost");
  else if (MQTTErr == MQTT_CONNECT_FAILED)
    Serial.print("Connect failed");
  else if (MQTTErr == MQTT_DISCONNECTED)
    Serial.print("Disconnected");
  else if (MQTTErr == MQTT_CONNECTED)
    Serial.print("Connected");
  else if (MQTTErr == MQTT_CONNECT_BAD_PROTOCOL)
    Serial.print("Connect bad protocol");
  else if (MQTTErr == MQTT_CONNECT_BAD_CLIENT_ID)
    Serial.print("Connect bad Client-ID");
  else if (MQTTErr == MQTT_CONNECT_UNAVAILABLE)
    Serial.print("Connect unavailable");
  else if (MQTTErr == MQTT_CONNECT_BAD_CREDENTIALS)
    Serial.print("Connect bad credentials");
  else if (MQTTErr == MQTT_CONNECT_UNAUTHORIZED)
    Serial.print("Connect unauthorized");
}


void firstHandshake(void)
{
  DynamicJsonDocument jsonBuffer(JSON_OBJECT_SIZE(3) + 100);
  JsonObject root = jsonBuffer.to<JsonObject>();
  JsonObject state = root.createNestedObject("state");
  JsonObject state_reported = state.createNestedObject("reported");


  state_reported["powerState"] = POWER_STATE;

  Serial.printf("Sending  [%s]: ", MQTT_PUB_TOPIC);
  serializeJson(root, Serial);
  Serial.println();
  char shadow[measureJson(root) + 1];
  serializeJson(root, shadow, sizeof(shadow));
  if (!client.publish(MQTT_PUB_TOPIC, shadow, false))
    pubSubErr(client.state());

  state_reported.remove("powerState");
  
  state_reported["saturation"] = SATURATION;

  Serial.printf("Sending  [%s]: ", MQTT_PUB_TOPIC);
  serializeJson(root, Serial);
  Serial.println();
  serializeJson(root, shadow, sizeof(shadow));
  if (!client.publish(MQTT_PUB_TOPIC, shadow, false))
    pubSubErr(client.state());

  state_reported.remove("saturation");

  state_reported["hue"] = HUE;

    Serial.printf("Sending  [%s]: ", MQTT_PUB_TOPIC);
  serializeJson(root, Serial);
  Serial.println();
  serializeJson(root, shadow, sizeof(shadow));
  if (!client.publish(MQTT_PUB_TOPIC, shadow, false))
    pubSubErr(client.state());

  state_reported.remove("hue");

  state_reported["brightness"] = BRIGHTNESS;  

    Serial.printf("Sending  [%s]: ", MQTT_PUB_TOPIC);
  serializeJson(root, Serial);
  Serial.println();
  serializeJson(root, shadow, sizeof(shadow));
  if (!client.publish(MQTT_PUB_TOPIC, shadow, false))
    pubSubErr(client.state());

  state_reported.remove("brightness");

}

void messageReceived(char *topic, byte *payload, unsigned int length)
{

  DynamicJsonDocument jsonBuffer(JSON_OBJECT_SIZE(3) + 100);
  JsonObject root = jsonBuffer.to<JsonObject>();
  JsonObject state = root.createNestedObject("state");
  JsonObject state_reported = state.createNestedObject("reported");


  Serial.println("Receiving...");

  int sendcheck = 0;

  deserializeJson(doc, payload);

  const char* powerState = doc["state"]["powerState"];
  if(String(powerState).length() != 0){
    POWER_STATE  = powerState;
    state_reported["powerState"] = POWER_STATE;
    Serial.print("POWER_STATE:" );
    Serial.print(POWER_STATE);
    Serial.println(" ");
    sendcheck = 1;
  }

  const char* hue = doc["state"]["hue"];
  if(String(hue).length() != 0){
    HUE = hue;
    state_reported["hue"] = HUE;
    Serial.print("HUE:" );
    Serial.print(HUE);
    Serial.println(" ");
    sendcheck = 1;
  }

  const char* saturation = doc["state"]["saturation"];
  if(String(saturation).length() != 0){
    SATURATION = saturation;
    state_reported["saturation"] = SATURATION;
    Serial.print("SATURATION:" );
    Serial.print(SATURATION);
    Serial.println(" ");
    sendcheck = 1;
  }

  const char* brightness = doc["state"]["brightness"];
  if(String(brightness).length() != 0){
    BRIGHTNESS = brightness;
    state_reported["brightness"] = BRIGHTNESS;
    Serial.print("BRIGHTNESS:" );
    Serial.print(BRIGHTNESS);
    Serial.println(" ");
    sendcheck = 1;
  }

  if(sendcheck == 1){
      Serial.printf("Sending  [%s]: ", MQTT_PUB_TOPIC);
      serializeJson(root, Serial);
      Serial.println();
      char shadow[measureJson(root) + 1];
      serializeJson(root, shadow, sizeof(shadow));
      if (!client.publish(MQTT_PUB_TOPIC, shadow, false))
        pubSubErr(client.state());
      
  }

    state_reported.remove("powerState");
    state_reported.remove("hue");
    state_reported.remove("saturation");
    state_reported.remove("brightness");

    t_millis= millis();
    check_led = 1;

}

void connectToMqtt(bool nonBlocking = false)
{
  Serial.print("MQTT connecting ");
  while (!client.connected())
  {
    if (client.connect(THINGNAME))
    {
      Serial.println("connected!");
      if (!client.subscribe(MQTT_SUB_TOPIC))
        pubSubErr(client.state());
    }
    else
    {
      Serial.print("failed, reason -> ");
      pubSubErr(client.state());
      if (!nonBlocking)
      {
        Serial.println(" < try again in 5 seconds");
        delay(5000);
      }
      else
      {
        Serial.println(" <");
      }
    }
    if (nonBlocking)
      break;
  }
}

void connectToWiFi(String init_str)
{
  if (init_str != emptyString)
    Serial.print(init_str);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(1000);
  }
  if (init_str != emptyString)
    Serial.println("ok!");
}

void checkWiFiThenMQTT(void)
{
  connectToWiFi("Checking WiFi");
  connectToMqtt();
}

unsigned long previousMillis = 0;
const long interval = 5000;

void checkWiFiThenMQTTNonBlocking(void)
{
  connectToWiFi(emptyString);
  if (millis() - previousMillis >= interval && !client.connected()) {
    previousMillis = millis();
    connectToMqtt(true);
  }
}

void checkWiFiThenReboot(void)
{
  connectToWiFi("Checking WiFi");
  Serial.print("Rebooting");
  ESP.restart();
}



void setup()
{
  Serial.begin(9600);

  FastLED.addLeds<WS2811, LED_PIN, GRB>(leds, NUM_LEDS);

  delay(5000);
  Serial.println();
  Serial.println();
  WiFi.setHostname(THINGNAME);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  connectToWiFi(String("Attempting to connect to SSID: ") + String(ssid));

  net.setCACert(cacert);
  net.setCertificate(client_cert);
  net.setPrivateKey(privkey);

  client.setServer(MQTT_HOST, MQTT_PORT);
  client.setCallback(messageReceived);

  connectToMqtt();

  delay(1000);
  firstHandshake();
}


void ledController(){

  uint8_t hue = atoi(HUE.c_str());
  uint8_t brightness = atof(BRIGHTNESS.c_str()) * 255;
  uint8_t saturation = atof(SATURATION.c_str()) * 255;


  if(POWER_STATE == "ON"){
    for(int i = 0;i < 72;i++){
          leds[i] = CHSV(hue,brightness,saturation);
          leds[143 - i] = CHSV(hue,brightness,saturation);
          FastLED.show();
      
    }
  }
  if(POWER_STATE == "OFF"){
    FastLED.clear();
  }
  
  
}

void loop()
{

  if(!client.connected()){
    connectToMqtt();
  }
  
  delta_t_millis = millis() - t_millis;

  if(check_led == 1 && delta_t_millis >= 1000){
    Serial.println("SET LED");
    ledController();
    check_led = 0;
  }

  client.loop();
}   
