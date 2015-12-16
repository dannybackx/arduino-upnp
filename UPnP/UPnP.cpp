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
#include "UPnP/Headers.h"

extern WebServer HTTP;

#undef	DEBUG_UPNP
// #define	DEBUG_UPNP	Serial

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

const char *_http_header =
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
    "<URLBase>http://%s:%u/</URLBase>" /* WiFi.localIP(), _port */
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

const char *UPnPClass::mimeTypeXML = "text/xml; charset=\"utf-8\"";
const char *UPnPClass::mimeTypeText = "text/plain; charset=\"utf-8\"";
const char *UPnPClass::envelopeHeader = 
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n"
    "<s:body>\r\n";
const char *UPnPClass::envelopeTrailer = 
    "</s:body>\r\n"    
    "</s:Envelope>\r\n";

// Called by HTTP server when our description XML is queried
void UPnPClass::schema(WiFiClient client) {
  client.printf(_http_header);

  IPAddress ip = WiFi.localIP();
  client.printf(_upnp_device_template_1,
    ip.toString().c_str(), device->getPort(),
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
    for (int i=0; i<nservices; i++) {
      // Need to free
      char *tmp = services[i]->getServiceXML();
      client.print(tmp);
      free(tmp);
    }
  }
  client.print(_upnp_device_template_2);
}

void UPnPClass::addService(UPnPService *srv) {
  if (nservices == maxservices) {
    maxservices += N_SERVICES;
    services = (UPnPService **)realloc(services, maxservices * sizeof(UPnPService *));
  }
  services[nservices++] = srv;
}

/*
 * This function is a pass-through for the member function just below.
 */
void staticSendSCPD() {
  UPnP.SendSCPD();
}

/*
 * Use the URL that the web server received, isolate the UPnPService name from it,
 * find that service, and call its SendSCPD method.
 */
void UPnPClass::SendSCPD() {
#ifdef DEBUG_UPNP
  DEBUG_UPNP.printf("SendSCPD(%s)\n", HTTP.httpUri());
#endif

  // Find out which UPnPService this was called for
  // The URL here is e.g. "/LEDService/scpd.xml"
  const char *url = HTTP.httpUri();
  const char *name = url+1;
  const char *p;

  for (p=name; *p && *p != '/'; p++) ;
  if (*p == '\0')
    return;	// silently

  int len = (p-name);
  for (int i=0; i<nservices; i++)
    if (strncmp(name, services[i]->serviceName, len) == 0) {
#ifdef DEBUG_UPNP
      DEBUG_UPNP.printf("SendSCPD : service %d, %s\n", i, services[i]->serviceName);
#endif

      // Call it !
      services[i]->SendSCPD(HTTP.client());
      return;
    }
}
