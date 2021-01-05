#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "WiFiManager.h"
#include <FS.h>
#include <LittleFS.h>
#include <Ticker.h>

String mDNSName;
String AccessCode;
const int ServicePort = 80;
ESP8266WebServer WebServer(ServicePort);
Ticker ticker;

const int LED = 12;
const int Relay = 15;
const int Button = 13;

int buttonState = LOW;
int powerState = LOW;

bool shouldSaveConfig = false;


//-------------------------- Electrical Control ---------------------------
void turnOn()
{
  Serial.println("Relay On");
  digitalWrite(LED, LOW);
  digitalWrite(Relay, HIGH);
  powerState = HIGH;
}

void turnOff()
{
  Serial.println("Relay Off");
  digitalWrite(LED, HIGH);
  digitalWrite(Relay, LOW);
  powerState = LOW;
}

void tick() {
  digitalWrite(LED, !digitalRead(LED));
}
//####################### End Of Electrical Control #######################


//--------------------------- Config Utilities ----------------------------
String readFile(const char* path) {
  if (!LittleFS.exists(path)) {
    File fileToCreate = LittleFS.open(path, "w");
    fileToCreate.print("");
    fileToCreate.close();
  }
  File fileToRead = LittleFS.open(path, "r");
  if (!fileToRead) {
    Serial.print("Failed To Load File: ");
    Serial.println(path);
    return String("");
  }
  size_t size = fileToRead.size();
  std::unique_ptr<char[]> buf(new char[size]);
  fileToRead.readBytes(buf.get(), size);
  fileToRead.close();
  return String(buf.get());
}

bool writeFile(const char* path, String content) {
  File fileToWrite = LittleFS.open(path, "w");
  if (!fileToWrite) {
    Serial.print("Failed To Write File: ");
    Serial.println(path);
    return false;
  }
  fileToWrite.print(content);
  fileToWrite.close();
  return true;
}

void saveConfigCallback () {
  shouldSaveConfig = true;
}

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}
//######################## End Of Config Utilities ########################


//--------------------------------- Setup ---------------------------------
void setup(void)
{
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  pinMode(Relay, OUTPUT);
  pinMode(Button, INPUT);
  turnOff();
  if (!LittleFS.begin()) {
    Serial.println("LittleFS Mount Failed");
    return;
  }

  // Load Config File Here
  mDNSName = readFile("/mDNSName");
  AccessCode = readFile("/AccessCode");

  WiFiManager wifiManager;
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  WiFiManagerParameter custom_mdnsname("mdns", "mDNS Name", mDNSName.c_str(), 16);
  WiFiManagerParameter custom_accesscode("accesscode", "Access Code", AccessCode.c_str(), 16);
  wifiManager.addParameter(&custom_mdnsname);
  wifiManager.addParameter(&custom_accesscode);

  if (!wifiManager.autoConnect()) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.restart();
    delay(5000);
  } else {
    digitalWrite(LED, HIGH);
  
    if (shouldSaveConfig) {
      mDNSName = custom_mdnsname.getValue();
      AccessCode = custom_accesscode.getValue();
      // Write Config Files
      writeFile("/mDNSName", mDNSName);
      writeFile("/AccessCode", AccessCode);
    }
  
    WebServer.on("/", []() {
      WebServer.send(200, "text/plain", String(powerState));
    });
  
    WebServer.on("/switch", []() {
      if (WebServer.method() != HTTP_POST) {
        if (AccessCode.compareTo(WebServer.arg("accesscode")) == 0) {
          bool stat = WebServer.arg("status");
          if (stat) {
            turnOn();
            WebServer.send(200, "text/plain", "Power ON");
          } else {
            turnOff();
            WebServer.send(200, "text/plain", "Power OFF");
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
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  
    if (MDNS.begin(mDNSName)) {
      Serial.print("mDNS responder started, mDNS name: ");
      Serial.println(mDNSName);
    }
    Serial.print("Access Code: ");
    Serial.println(AccessCode);
  }
}
//############################# End Of Setup ##############################


//------------------------------- Main Loop -------------------------------
void loop(void) {
  MDNS.update();
  WebServer.handleClient();
  buttonState = digitalRead(Button);
  if (buttonState == LOW) {
    if (powerState == LOW) {
      turnOn();
    } else {
      turnOff();
    }
    delay(1000);
  }
}
//############################ End Of Main Loop ###########################
