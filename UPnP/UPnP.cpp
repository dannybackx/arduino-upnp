#include "Arduino.h"
#include "UPnP.h"

#define LWIP_OPEN_SRC
#include <functional>
#include "UPnP/SSDP.h"
#include "WiFiUdp.h"
#include "debug.h"

extern "C" {
  #include "osapi.h"
  #include "ets_sys.h"
  #include "user_interface.h"
}

#include "lwip/opt.h"
#include "lwip/udp.h"
#include "lwip/inet.h"
#include "lwip/igmp.h"
#include "lwip/mem.h"
#include "include/UdpContext.h"

int a = 0;

UPnPClass::UPnPClass() {
	a = 0xdead;
}

UPnPClass::~UPnPClass() {
}

void UPnPClass::begin() {
	Serial.printf("UPnP begin(%x)\n", a);
}

UPnPClass UPnP;
/* */

static const char* _ssdp_schema_template = 
  "HTTP/1.1 200 OK\r\n"
  "Content-Type: text/xml\r\n"
  "Connection: close\r\n"
  "Access-Control-Allow-Origin: *\r\n"
  "\r\n"
  "<?xml version=\"1.0\"?>"
  "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
    "<specVersion>"
      "<major>1</major>"
      "<minor>0</minor>"
    "</specVersion>"
    "<URLBase>http://%u.%u.%u.%u:%u/</URLBase>" // WiFi.localIP(), _port
    "<device>"
      "<deviceType>urn:schemas-upnp-org:device:Basic:1</deviceType>"
      "<friendlyName>%s</friendlyName>"
      "<presentationURL>%s</presentationURL>"
      "<serialNumber>%s</serialNumber>"
      "<modelName>%s</modelName>"
      "<modelNumber>%s</modelNumber>"
      "<modelURL>%s</modelURL>"
      "<manufacturer>%s</manufacturer>"
      "<manufacturerURL>%s</manufacturerURL>"
      "<UDN>uuid:%s</UDN>"
      "<serviceList>"
      "<service>"
      "<serviceType>urn:danny-backx-info:service:sensor:1</serviceType>"
      "<serviceId>urn:danny-backx-info:serviceId:sensor1</serviceId>"
      "<controlURL>/control.xml</controlURL>"
      "<eventSubURL>/event.xml</eventSubURL>"
      "<SCPDURL>/scpd.xml</SCPDURL>"
      "</service>"
      "</serviceList>"
    "</device>"
  "</root>\r\n"
  "\r\n";


// Called by HTTP server when our description XML is queried
void UPnPClass::schema(WiFiClient client) {
  uint32_t ip = WiFi.localIP();
  client.printf(_ssdp_schema_template,
    IP2STR(&ip), _port,
    _friendlyName,
    _presentationURL,
    _serialNumber,
    _modelName,
    _modelNumber,
    _modelURL,
    _manufacturer,
    _manufacturerURL,
    _uuid
  );
}

void UPnPClass::setSchemaURL(const char *url) {
  strlcpy(_schemaURL, url, sizeof(_schemaURL));
}

void UPnPClass::setHTTPPort(uint16_t port){
  _port = port;
}

void UPnPClass::setName(const char *name){
  strlcpy(_friendlyName, name, sizeof(_friendlyName));
}

void UPnPClass::setURL(const char *url){
  strlcpy(_presentationURL, url, sizeof(_presentationURL));
}

void UPnPClass::setSerialNumber(const char *serialNumber){
  strlcpy(_serialNumber, serialNumber, sizeof(_serialNumber));
}

void UPnPClass::setModelName(const char *name){
  strlcpy(_modelName, name, sizeof(_modelName));
}

void UPnPClass::setModelNumber(const char *num){
  strlcpy(_modelNumber, num, sizeof(_modelNumber));
}

void UPnPClass::setModelURL(const char *url){
  strlcpy(_modelURL, url, sizeof(_modelURL));
}

void UPnPClass::setManufacturer(const char *name){
  strlcpy(_manufacturer, name, sizeof(_manufacturer));
}

void UPnPClass::setManufacturerURL(const char *url){
  strlcpy(_manufacturerURL, url, sizeof(_manufacturerURL));
}
