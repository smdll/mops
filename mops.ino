#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "WiFiManager.h"
#include <FS.h>
#include <LittleFS.h>

const char *WIFI_SSID = "MopsConfig";
const char *WIFI_PASSWORD = "codetyphon";
String mDNSName;
String AccessCode;
const int ServicePort = 80;
ESP8266WebServer WebServer(ServicePort);

const int led = 12;
const int relay = 15;
const int btn = 13;

int buttonState = LOW;
int powerState = LOW;

bool shouldSaveConfig = false;

void saveConfigCallback () {
  shouldSaveConfig = true;
}

void turnon()
{
  Serial.println("turn on");
  digitalWrite(led, LOW);    //led是低电平触发
  digitalWrite(relay, HIGH); //继电器是高电平触发
  powerState = HIGH;
}

void turnoff()
{
  Serial.println("turn off");
  digitalWrite(led, HIGH);
  digitalWrite(relay, LOW);
  powerState = LOW;
}

void setup(void)
{
  pinMode(led, OUTPUT);
  pinMode(relay, OUTPUT);
  pinMode(btn, INPUT);
  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount failed");
    return;
  }

  // Load mDNS name here
  int charCount = 0;
  File mDNSNameF = LittleFS.open("mdnsname", "rw");
  while (charCount < 16 && mDNSNameF.available() > 0) {
    mdnsname[charCount] = mDNSNameF.read();
    charCount++;
  }
  
  // Read access code here
  charCount = 0;
  File AccessCodeF = LittleFS.open("accesscode", "rw");
  while (charCount < 16 && AccessCodeF.available() > 0) {
    accesscode[charCount] = AccessCodeF.read();
    charCount++;
  }

  turnoff();

  Serial.begin(9600);

  WiFiManager wifiManager;
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  WiFiManagerParameter custom_mdnsname("mdns", "mDNS Name", mdnsname, 16);
  WiFiManagerParameter custom_accesscode("accesscode", "Access Code", accesscode, 16);
  wifiManager.addParameter(&custom_mdnsname);
  wifiManager.addParameter(&custom_accesscode);

  if (!wifiManager.autoConnect(WIFI_SSID, WIFI_PASSWORD)) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (shouldSaveConfig) {
    strncpy(mdnsname, custom_mdnsname.getValue(), 16);
    mdnsname[16] = '\0';
    strncpy(accesscode, custom_accesscode.getValue(), 16);
    accesscode[16] = '\0';
    AccessCodeF.print(accesscode);
    mDNSNameF.print(mdnsname);
  }

  WebServer.on("/", []() {
    WebServer.send(200, "text/plain", String(powerState));
  });

  WebServer.on("/switch", []() {
    if (WebServer.method() != HTTP_POST) {
      if (strncmp(accesscode, WebServer.arg("accesscode").c_str(), 16) == 0) {
        bool stat = WebServer.arg("status");
        if (stat) {
          turnon();
          WebServer.send(200, "text/plain", "power on");
        } else {
          turnoff();
          WebServer.send(200, "text/plain", "power off");
        }
      } else
        WebServer.send(401, "text/plain", "Unauthorized");
    } else
      WebServer.send(405, "text/plain", "Method Not Allowed");
  });

  WebServer.onNotFound([]() {
    WebServer.send(404, "text/plain", "File Not Found");
  });
  
  WebServer.begin();
  MDNS.addService("http", "tcp", ServicePort);
  Serial.println("HTTP Server Started");

  if (MDNS.begin(mdnsname))
  {
    Serial.print("mDNS responder started, mDNS name:\n\t");
    Serial.println(mdnsname);
  }
  Serial.print("Access Code:\n\t");
  Serial.println(accesscode);
  mDNSNameF.close();
  AccessCodeF.close();
}

void loop(void) {
  MDNS.update();
  WebServer.handleClient();
  buttonState = digitalRead(btn);
  if (buttonState == LOW) {
    Serial.println("button click");
    if (powerState == LOW) {
      turnon();
    } else {
      turnoff();
    }
    delay(1000);
  }
}
