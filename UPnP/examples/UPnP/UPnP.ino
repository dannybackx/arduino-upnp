#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <UPnP/UPnPDevice.h>
#include <UPnP.h>
#include <UPnP/SSDP.h>

const char* ssid     = "your-ssid";
const char* password = "your-password";

ESP8266WebServer HTTP(80);
UPnPDevice device;

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
    HTTP.on("/ssdp/schema.xml", HTTP_GET, []() {
      //HTTP.send(200, "text/plain", "<root></root>");
      UPnP.schema(HTTP.client());
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

    Serial.printf("Ready!\n");
  } else {
    Serial.printf("WiFi Failed\n");
    while(1) delay(100);
  }

}

void loop() {
  HTTP.handleClient();
  // Serial.printf("Called handleClient()...\n");
  delay(1000);
}
