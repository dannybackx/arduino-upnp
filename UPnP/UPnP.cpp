/*
 * ESP8266 Simple UPnP framework
 *   There's no separate class for multiple devices,
 *   also currently we allocate only one service,
 *   because we assume one (IoT) device is all we do.
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
#include "UPnP/WebServer.h"

extern "C" {
  #include "user_interface.h"
}

#define	DEBUG_UPNP	Serial

UPnPClass UPnP;	// FIXME

UPnPClass::UPnPClass() {
  services = 0;
}

UPnPClass::~UPnPClass() {
#ifdef DEBUG_UPNP
  DEBUG_UPNP.printf("UPnPClass DTOR\n");
#endif
}

void UPnPClass::begin(WebServer *http, UPnPDevice *device) {
  this->device = device;
  this->http = http;
}

static const char *_http_header =
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
    "<URLBase>http://%u.%u.%u.%u:%u/</URLBase>" /* WiFi.localIP(), _port */
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

static const char *_upnp_scpd_template =
  "<?xml version=\"1.0\"?>"
  "<scpd xmlns=\"urn:danny-backx-info:service-1-0\">"
  "<specVersion>"
  "<major>1</major>"
  "<minor>0</minor>"
  "</specVersion>"
  "%s"			// getActionListXML
  "%s"			// getStateVariableListXML
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
  if (services) {
    // Need to free
    char *tmp = services->getServiceXML();
    client.print(tmp);
    free(tmp);
  }
  client.print(_upnp_device_template_2);
}

void UPnPClass::SCPD(WiFiClient client) {
  uint32_t ip = WiFi.localIP();
  client.print(_http_header);

  char *al = services->getActionListXML();
  char *svl = services->getStateVariableListXML();

  int len = strlen(_upnp_scpd_template) + strlen(al) + strlen(svl);
  char *scpd = (char *)malloc(len);
  sprintf(scpd, _upnp_scpd_template, al, svl);
  client.print(scpd);
  free(scpd);
  free(al);
  free(svl);
}

void UPnPClass::addService(UPnPService *srv) {
  this->services = srv;
}

const char *UPnPClass::mimeTypeXML = "text/xml; charset=\"utf-8\"";
const char *UPnPClass::mimeTypeText = "text/plain; charset=\"utf-8\"";
const char *UPnPClass::envelopeHeader = 
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n"
    "<s:body>\r\n";
const char *UPnPClass::envelopeTrailer = 
    "</s:body>\r\n"    
    "</s:Envelope>\r\n";
