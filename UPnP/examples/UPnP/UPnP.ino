#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <UPnP/UPnPDevice.h>
#include <UPnP.h>
#include <UPnP/SSDP.h>
#include "MotionSensorService.h"

const char* ssid     = "your-ssid";
const char* password = "your-password";

ESP8266WebServer HTTP(80);
UPnPDevice device;
UPnPClass UPnP(&HTTP);

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.printf("Boot version %d\n", ESP.getBootVersion());
  Serial.printf("Flash chip Real size %d, size %d\n", ESP.getFlashChipRealSize(), ESP.getFlashChipSize());
  Serial.printf("SDK version %s\n", ESP.getSdkVersion());
  Serial.printf("Vcc %d\n", ESP.getVcc());

  Serial.print("Starting WiFi... ");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if(WiFi.waitForConnectResult() == WL_CONNECTED){
    
    IPAddress ip = WiFi.localIP();
    Serial.print("MAC "); Serial.print(WiFi.macAddress());
    Serial.printf(", SSID {%s}, IP address %d.%d.%d.%d\n", WiFi.SSID(), ip[0], ip[1], ip[2], ip[3]);
    WiFi.printDiag(Serial);

    Serial.printf("Starting HTTP...\n");
    HTTP.on("/index.html", HTTP_GET, [](){
      HTTP.send(200, "text/plain", "Hello World!");
    });
    HTTP.on("/description.xml", HTTP_GET, [](){
      UPnP.schema(HTTP.client());
    });
    HTTP.on("/event", eventHandler);
    HTTP.on("/sensor.xml", HTTP_GET, []() {
      UPnP.SCPD(HTTP.client());
    });
    HTTP.begin();

    Serial.printf("Starting SSDP...\n");
    device.setSchemaURL("description.xml");
    device.setHTTPPort(80);
    device.setName("UPnP Motion Sensor Kit");
    device.setSerialNumber("001788102201");
    device.setURL("index.html");
    device.setModelName("UPnP Motion Sensor Kit");
    device.setModelNumber("929000226503");
    device.setModelURL("http://danny.backx.info");
    device.setManufacturer("Danny Backx");
    device.setManufacturerURL("http://danny.backx.info");
    SSDP.begin(device);

    UPnP.begin(device);

    MotionSensorService srv = MotionSensorService("urn:danny-backx-info:service:sensor:1", "urn:danny-backx-info:serviceId:sensor1");
    UPnP.addService(&srv);

    Serial.printf("Ready!\n");
  } else {
    Serial.printf("WiFi Failed\n");
    while(1) delay(100);
  }

}

void loop() {
  HTTP.handleClient();
  // Serial.printf("Called handleClient()...\n");
  delay(10);
}

// This is a hack to demonstrate
// TODO : decode the request, see if it is one we want to answer
// TODO : the actual response needs to come from the (MotionSensor)Service class
void eventHandler() {
  Serial.print("eventHandler(");
  Serial.print(HTTP.uri());
  Serial.println(")");

  // Assumption that we get called with a GetState action
  HTTP.send(200, "text/xml; charset=\"utf-8\"",
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
      "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n"
      "<s:body>\r\n"
      "<u:GetStateResponse xmlns=\"urn:danny-backx-info:service:sensor:1\">\r\n"
      "<State>0</State>\r\n"
      "</u:GetStateResponse>\r\n"
      "</s:body>\r\n"    
      "</s:Envelope>\r\n"
    );
}
