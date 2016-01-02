#include <ESP8266WiFi.h>
#include <UPnP.h>
#include <UPnP/WebServer.h>
#include <UPnP/UPnPDevice.h>
#include <UPnP/SSDP.h>
#include <Wire.h>       // Required to compile UPnP
#include <FS.h>
#include <UPnP/AlarmService.h>

// Prepare for OTA software installation
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
static int OTAprev;

// Stuff for sending mail
#include <SmtpClient.h>
#include "Mail.h"

#include "mywifi.h"
const char* ssid     = MY_SSID;
const char* password = MY_WIFI_PASSWORD;

char *deviceURN = "urn:schemas-upnp-org:device:ESP8266 Alarm Controller:1";

#include <sntp.h>
#include <time.h>

//#include <GDBStub.h>

WebServer HTTP(80);
UPnPDevice device;

void setup() {
  Serial.begin(9600);
//  Serial.begin(115200);
  Serial.println("Alarm Controller");
  Serial.printf("Boot version %d, flash chip size %d, SDK version %s\n",
    ESP.getBootVersion(), ESP.getFlashChipSize(), ESP.getSdkVersion());

  Serial.printf("Free sketch space %d\n", ESP.getFreeSketchSpace());
  Serial.print("Starting WiFi... ");
  WiFi.mode(WIFI_STA);

  int wifi_tries = 3;
  int wcr;
  while (wifi_tries-- >= 0) {
    WiFi.begin(ssid, password);
    wcr = WiFi.waitForConnectResult();
    if (wcr == WL_CONNECTED)
      break;
    Serial.printf(" %d ", wifi_tries + 1);
  }

  if (wcr != WL_CONNECTED) {
    Serial.printf("WiFi Failed\n");
    delay(2000);
    ESP.restart();
  }

  IPAddress ip = WiFi.localIP();
  String ips = ip.toString();
  IPAddress gw = WiFi.gatewayIP();
  String gws = gw.toString();
  Serial.print("MAC "); Serial.print(WiFi.macAddress());
  Serial.printf(", SSID {%s}, IP %s, GW %s\n", WiFi.SSID().c_str(), ips.c_str(), gws.c_str());

  // Set up real time clock
  sntp_init();
  sntp_setservername(0, "ntp.scarlet.be");
  sntp_setservername(1, "ntp.belnet.be");
  (void)sntp_set_timezone(+1);

#ifdef ENABLE_SPIFFS
  SPIFFS.begin();
#if 0
  Serial.print("SPIFFS : formatting ...");
  SPIFFS.format();
  Serial.println(" done");
#endif
  Dir dir = SPIFFS.openDir("/");
  Serial.println("SPIFFS directory {/} :");
  while (dir.next()) {
    Serial.print("  "); Serial.println(dir.fileName());
  }
  dir = SPIFFS.openDir("");
  Serial.println("SPIFFS directory {} :");
  while (dir.next()) {
    Serial.print("  "); Serial.println(dir.fileName());
  }

  FSInfo fs_info;
  SPIFFS.info(fs_info);
  Serial.printf("SPIFFS total %d used %d maxpathlength %d\n",
                fs_info.totalBytes, fs_info.usedBytes, fs_info.maxPathLength);

  File f = SPIFFS.open("/config.txt", "r");
  if (f) {
    Serial.printf("SPIFFS : /config.txt exists, content %d bytes\n", f.size());
    f.close();
  }
#endif

  Serial.printf("Starting HTTP...\n");
  HTTP.on("/index.html", HTTP_GET, []() {
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

  AlarmService alarm = AlarmService();
  UPnP.addService(&alarm);

#ifdef ENABLE_OTA
  Serial.printf("Starting OTA listener ...\n");
  ArduinoOTA.onStart([]() {
    Serial.print("OTA Start : ");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA Complete");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    int curr;
    curr = (progress / (total / 50));
    if (OTAprev < curr)
      Serial.print('#');
    OTAprev = curr;
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.setPort(8266);
  ArduinoOTA.setHostname("OTA-Controller");
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.begin();

#endif

  time_t t;
  Serial.printf("Time ");
  t = sntp_get_current_timestamp();
  while (t < 0x1000) {
    Serial.printf(".");
    delay(1000);
    t = sntp_get_current_timestamp();
  }
  Serial.printf(" is %s\n", asctime(localtime(&t)));

  Serial.printf("Ready!\n");
  while (1) {
    HTTP.handleClient();
#ifdef ENABLE_OTA
    ArduinoOTA.handle();
#endif
  }
}

void loop() {
}
