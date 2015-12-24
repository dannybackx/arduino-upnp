#include <ESP8266WiFi.h>
#include <UPnP/WebServer.h>
#include <UPnP/UPnPDevice.h>
#include <UPnP.h>
#include <UPnP/SSDP.h>
#include <FS.h>

//#include <sntp.h>

#include "MotionSensorService.h"
#include "UPnP/LEDService.h"
#include "UPnP/DHTSensorService.h"

// #include "GDBStub.h"

// Stuff to sync this source file in github
// Provide a "mywifi.h" file that defines the two macros below
//
// So copy something like this into that file and put in the right values :
// #define MY_SSID "your-ssid"
// #define MY_WIFI_PASSWORD "your-password"

#include "mywifi.h"
const char* ssid     = MY_SSID;
const char* password = MY_WIFI_PASSWORD;

// const char *serviceType = "urn:danny-backx-info:service:sensor:1";
// const char *serviceId = "urn:danny-backx-info:serviceId:sensor1";
char *deviceURN = "urn:schemas-upnp-org:device:ESP8266:1";

WebServer HTTP(80);
UPnPDevice device;

void setup() {
  Serial.begin(115200);
//  Serial.println();
  Serial.printf("Boot version %d\n", ESP.getBootVersion());
  Serial.printf("Flash chip Real size %d, size %d\n", ESP.getFlashChipRealSize(), ESP.getFlashChipSize());
  Serial.printf("SDK version %s\n", ESP.getSdkVersion());
  // Serial.printf("Vcc %d\n", ESP.getVcc());

  Serial.print("Starting WiFi... ");

  WiFi.mode(WIFI_STA);

  int wifi_tries = 3;
  int wcr;
  while (wifi_tries-- >= 0) {
    WiFi.begin(ssid, password);
    wcr = WiFi.waitForConnectResult();
    if (wcr == WL_CONNECTED)
      break;
  }
  
  if (wcr == WL_CONNECTED) {  
    IPAddress ip = WiFi.localIP();
    String ips = ip.toString();
    Serial.print("MAC "); Serial.print(WiFi.macAddress());
    Serial.printf(", SSID {%s}, IP address %s\n", WiFi.SSID().c_str(), ips.c_str());
    WiFi.printDiag(Serial);

#ifdef ENABLE_SPIFFS
    SPIFFS.begin();
    Dir dir = SPIFFS.openDir("/");
    Serial.println("SPIFFS directory {/} :");
    while (dir.next()) {
      Serial.print("  "); Serial.println(dir.fileName());
    }

    FSInfo fs_info;
    SPIFFS.info(fs_info);
    Serial.printf("SPIFFS total %d used %d maxpathlength %d\n",
      fs_info.totalBytes, fs_info.usedBytes, fs_info.maxPathLength);

//    SPIFFS.format();
//    SPIFFS.remove("/config.xml");
    
    // Create a file
    File f = SPIFFS.open("/config.txt", "r");
    if (f) {
      Serial.printf("SPIFFS : /config.txt exists, content %d bytes\n", f.size());
    } else {
      f = SPIFFS.open("/config.txt", "w");
      f.printf("LED:active:50\n");
      f.printf("LED:passive:450\n");
      f.close();
    }
#endif

/*
    sntp_init();
    sint8 tz = sntp_get_timezone();
    // sntp_asctime();
/* */

    
    Serial.printf("Starting HTTP...\n");
    HTTP.on("/index.html", HTTP_GET, [](){
      // Serial.println("Sending hello");
      HTTP.send(200, "text/plain", "Hello World!");
      // Serial.println("Hello ok");
    });
    HTTP.begin();

    Serial.printf("Starting SSDP...\n");
    device.setDeviceURN(deviceURN);
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

    UPnP.begin(&HTTP, &device);

    MotionSensorService ms_srv = MotionSensorService();
    UPnP.addService(&ms_srv);

#ifdef ENABLE_LED_SERVICE
    LEDService led_srv = LEDService();
    led_srv.begin();
    UPnP.addService(&led_srv);
#endif
#ifdef ENABLE_DHT_SERVICE
    DHTSensorService dht = DHTSensorService();
    UPnP.addService(&dht);
    dht.begin();
#endif

    Serial.printf("Ready!\n");
    while (1) {
      ms_srv.poll();
      HTTP.handleClient();
#ifdef ENABLE_LED_SERVICE
      led_srv.periodic();
#endif
      // Serial.printf("After HandleClient : Heap %X\n", ESP.getFreeHeap());
      delay(10);
    }

  } else {
    Serial.printf("WiFi Failed\n");
    while(1) delay(100);
  }
}

void loop() {
  HTTP.handleClient();
  Serial.printf("After HandleClient : Heap %X\n", ESP.getFreeHeap());
  // Serial.printf("Called handleClient()...\n");
  delay(10);
}
