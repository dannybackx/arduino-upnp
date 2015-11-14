/*
 * ESP8266 Simple UPnP framework
 *   There's no separate class for multiple devices, we assume one (IoT) device is all we do.
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
#include "debug.h"
#include "ESP8266WebServer.h"

extern "C" {
  #include "user_interface.h"
}

UPnPClass UPnP;	// FIXME

UPnPClass::UPnPClass() {
  services = 0;
}

UPnPClass::~UPnPClass() {
}

void UPnPClass::begin(ESP8266WebServer *http, UPnPDevice *device) {
  Serial.printf("UPnP begin(%p, %p)\n", http, device);
  this->device = device;
  this->http = http;
}

static const char* _http_header = 
  "HTTP/1.1 200 OK\r\n"
  "Content-Type: text/xml\r\n"
  "Connection: close\r\n"
  "Access-Control-Allow-Origin: *\r\n"
  "\r\n";

static const char *_upnp_device_template_1 =
  "<?xml version=\"1.0\"?>"
  "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
    "<specVersion>"
      "<major>1</major>"
      "<minor>0</minor>"
    "</specVersion>"
    "<URLBase>http://%u.%u.%u.%u:%u/</URLBase>" // WiFi.localIP(), _port
    "<device>"
      "<deviceType>%s</deviceType>"
      "<friendlyName>%s</friendlyName>"
      "<presentationURL>%s</presentationURL>"
      "<serialNumber>%s</serialNumber>"
      "<modelName>%s</modelName>"
      "<modelNumber>%s</modelNumber>"
      "<modelURL>%s</modelURL>"
      "<manufacturer>%s</manufacturer>"
      "<manufacturerURL>%s</manufacturerURL>"
      "<UDN>uuid:%s</UDN>"
      "<serviceList>";

static const char *_upnp_device_template_2 =
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

  client.printf(_upnp_device_template_1,
    IP2STR(&ip), device->getPort(),
    device->getDeviceURN(),
    device->getFriendlyName(),
    device->getPresentationURL(),
    device->getSerialNumber(),
    device->getModelName(),
    device->getModelNumber(),
    device->getModelURL(),
    device->getManufacturer(),
    device->getManufacturerURL(),
    device->getUuid()
  );
  char *tmp = services->getServiceXML();
  if (services)
    client.print(services->getServiceXML());
  client.print(_upnp_device_template_2);
}

void UPnPClass::SCPD(WiFiClient client) {
  uint32_t ip = WiFi.localIP();
  client.printf(_http_header);
  client.printf(_upnp_scpd_template);
}

void UPnPClass::addService(UPnPService *srv) {
#ifdef DEBUG
  DEBUG.println("UPnPClass::addService()");
#endif
  this->services = srv;
}
