#include <ESP8266WiFi.h>
#include <UPnP/WebServer.h>
#include <UPnP/UPnPDevice.h>
#include <UPnP.h>
#include <UPnP/SSDP.h>
#include <FS.h>

//#include <sntp.h>

#include "MotionSensorService.h"
#include "UPnP/LEDService.h"


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
char *deviceURN = "urn:schemas-upnp-org:device:Basic:1";

WebServer HTTP(80);
UPnPDevice device;
// UPnPClass UPnP;

void setup() {
  Serial.begin(115200);
//  Serial.println();
  Serial.printf("Boot version %d\n", ESP.getBootVersion());
  Serial.printf("Flash chip Real size %d, size %d\n", ESP.getFlashChipRealSize(), ESP.getFlashChipSize());
  Serial.printf("SDK version %s\n", ESP.getSdkVersion());
  // Serial.printf("Vcc %d\n", ESP.getVcc());

  Serial.print("Starting WiFi... ");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if(WiFi.waitForConnectResult() == WL_CONNECTED){
    
    IPAddress ip = WiFi.localIP();
    Serial.print("MAC "); Serial.print(WiFi.macAddress());
    Serial.print(", SSID {");
    Serial.print(WiFi.SSID());
    Serial.printf("}, IP address %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
    // Serial.printf(", SSID {%s}, IP address %d.%d.%d.%d\n", WiFi.SSID(), ip[0], ip[1], ip[2], ip[3]);
    WiFi.printDiag(Serial);

/* */   
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

    SPIFFS.remove("config.xml");
    // Create a file
    File f = SPIFFS.open("/config.xml", "r");
    if (f) {
      Serial.printf("SPIFFS : /config.xml exists, content %d bytes\n", f.size());
    } else {
      f = SPIFFS.open("/config.xml", "w");
      f.printf("<config>hello there</config>\n");
      f.close();
    }
/* */

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
#ifdef MSS_GLOBAL
    MSS = MotionSensorService();
    UPnP.addService(&MSS);
    
    Serial.printf("Ready!\n");
#else
    MotionSensorService ms_srv = MotionSensorService();
    UPnP.addService(&ms_srv);

    LEDService led_srv = LEDService();
    led_srv.begin();
    led_srv.setPeriod(5, 495);
    led_srv.SetState(LED_STATE_BLINK);
    UPnP.addService(&led_srv);
/* */
    Serial.printf("Ready!\n");
    while (1) {
      ms_srv.poll();
      HTTP.handleClient();
      led_srv.periodic();
      
      // Serial.printf("After HandleClient : Heap %X\n", ESP.getFreeHeap());
      delay(10);
    }
#endif
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
