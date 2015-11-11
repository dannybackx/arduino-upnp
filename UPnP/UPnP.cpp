/*
 * ESP8266 Simple UPnP framework
 *   There is no separate Device class, we assume one (IoT) device is all we do.
 *
 * Copyright (c) 2015 Hristo Gochkov
 * Copyright (c) 2015 Danny Backx
 * 
 * Original (Arduino) version by Filippo Sallemi, July 23, 2014.
 * Can be found at: https://github.com/nomadnt/uSSDP
 * 
 * License (MIT license):
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:
 * 
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 * 
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *   THE SOFTWARE.
 * 
 */
#include "Arduino.h"
#include "UPnP.h"

#define LWIP_OPEN_SRC
#include <functional>
#include "UPnP/SSDP.h"
#include "WiFiUdp.h"
#include "debug.h"
#include "ESP8266WebServer.h"

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


ESP8266WebServer *http;

UPnPClass::UPnPClass(ESP8266WebServer *http) {
  services = 0;
  this->http = http;
}

UPnPClass::~UPnPClass() {
}

void UPnPClass::begin(UPnPDevice &device) {
  Serial.printf("UPnP begin()\n");
  this->device = device;
}

// UPnPClass UPnP;

static const char* _http_header = 
  "HTTP/1.1 200 OK\r\n"
  "Content-Type: text/xml\r\n"
  "Connection: close\r\n"
  "Access-Control-Allow-Origin: *\r\n"
  "\r\n";

static const char* _upnp_schema_template = 
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
      "<controlURL>/control</controlURL>"
      "<eventSubURL>/event</eventSubURL>"
      "<SCPDURL>/sensor.xml</SCPDURL>"
      "</service>"
      "</serviceList>"
    "</device>"
  "</root>\r\n"
  "\r\n";

static const char* _upnp_scpd_template = 
  "<?xml version=\"1.0\"?>"
  "<scpd xmlns=\"urn:danny-backx-info:service-1-0\">"
  "<specVersion>"
  "<major>1</major>"
  "<minor>0</minor>"
  "</specVersion>"
  "<actionList>"
  "<action>"
  "<name>getState</name>"
  "<argumentList>"
  "<argument>"
  "<retval/>"
  "<name>State</name>"
  "<relatedStateVariable>State</relatedStateVariable>"
  "<direction>out</direction>"
  "</argument>"
  "</argumentList>"
  "</action>"
  "</actionList>"
  "<serviceStateTable>"
  "<stateVariable sendEvents=\"yes\">"
  "<name>State</name>"
  "<dataType>String</dataType>"
  "<defaultValue></defaultValue>"
  "</stateVariable>"
  "</serviceStateTable>"
  "</scpd>\r\n"
  "\r\n";


// Called by HTTP server when our description XML is queried
void UPnPClass::schema(WiFiClient client) {
  uint32_t ip = WiFi.localIP();
  client.printf(_http_header);
  client.printf(_upnp_schema_template,
    IP2STR(&ip), device.getPort(),
    device.getFriendlyName(),
    device.getPresentationURL(),
    device.getSerialNumber(),
    device.getModelName(),
    device.getModelNumber(),
    device.getModelURL(),
    device.getManufacturer(),
    device.getManufacturerURL(),
    device.getUuid()
  );
}

void UPnPClass::SCPD(WiFiClient client) {
  uint32_t ip = WiFi.localIP();
  client.printf(_http_header);
  client.printf(_upnp_scpd_template);
}

void UPnPClass::addService(UPnPService *srv) {
  this->services = srv;
}
