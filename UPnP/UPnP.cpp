/*
 *
 */
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

UPnPClass::UPnPClass() {
}

UPnPClass::~UPnPClass() {
}

void UPnPClass::begin(UPnPDevice &device) {
	Serial.printf("UPnP begin()\n");
	d = device;
}

UPnPClass UPnP;

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
    IP2STR(&ip), d.getPort(),
    d.getFriendlyName(),
    d.getPresentationURL(),
    d.getSerialNumber(),
    d.getModelName(),
    d.getModelNumber(),
    d.getModelURL(),
    d.getManufacturer(),
    d.getManufacturerURL(),
    d.getUuid()
  );
}
