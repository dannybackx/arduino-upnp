/*
 * ESP8266 Simple UPnP Service framework
 *   Simple means little or no support for lots of services and devices.
 *   An IoT device probably implements just one thing...
 *
 * Copyright (c) 2015 Danny Backx
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
 */
#include "Arduino.h"
#include "UPnP.h"

#define	UPNP_DEBUG Serial

static const char *_scpd_xml = "/scpd.xml";
static const char *_description_xml = "/description.xml";

UPnPService::UPnPService(const char *serviceType, const char *serviceId) {
  nactions = 0;
  nvariables = 0;
  actions = new Action [N_ACTIONS];
  variables = new StateVariable [N_VARIABLES];

  this->serviceType = serviceType;
  this->serviceId = serviceId;
}

UPnPService::~UPnPService() {
  delete actions;
  delete variables;
}

void UPnPService::addAction(const char *name, ActionFunction handler, const char *xml) {
  // FIXME intentionally no bounds checking code
  actions[nactions].name = name;
  actions[nactions].handler = handler;
  actions[nactions].xml = xml;
  nactions++;
}

void UPnPService::addStateVariable(const char *name, const char *datatype, boolean sendEvents) {
  variables[nvariables].name = name;
  variables[nvariables].dataType = datatype;
  variables[nvariables].sendEvents = sendEvents;
  nvariables++;
}

static const char *_get_service_xml_template =
  "<service>"
    "<serviceType>%s</serviceType>"
    "<serviceId>%s</serviceId>"
    "<controlURL>/control</controlURL>"
    "<eventSubURL>/event</eventSubURL>"
    "<SCPDURL>%s</SCPDURL>"
  "</service>";

// Caller must free return pointer
char *UPnPService::getServiceXML() {
  char *r = new char[250];	// FIXME, should 170 + serviceType + serviceId, currently 240 is ok
  sprintf(r, _get_service_xml_template,
#if 1
    /* serviceType */ "urn:danny-backx-info:service:sensor:1",
    /* serviceId */ "urn:danny-backx-info:serviceId:sensor1",
    _scpd_xml);
#else
      serviceType, serviceId, _scpd_xml);
#endif
  return r;
}

/*
char *UPnPService::getServiceXML() {
  return "<service><serviceType>urn:danny-backx-info:service:sensor:1</serviceType><serviceId>urn:danny-backx-info:serviceId:sensor1</serviceId><controlURL>/control</controlURL><eventSubURL>/event</eventSubURL><SCPDURL>/scpd.xml</SCPDURL></service>";
}
/* */

char *UPnPService::getActionListXML() {
  int l = 32;
  int i;
  for (i=0; i<nactions; i++)
    l += strlen(actions[i].xml);
#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("getActionListXML : alloc %d\n", l);
#endif
  char *r = new char[l];	// FIXME
  strcpy(r, "<ActionList>\r\n");
  for (i=0; i<nactions; i++)
    strcat(r, actions[i].xml);
  strcat(r, "</ActionList>\r\n");
#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("getActionListXML : len %d\n", strlen(r));
#endif
  return r;
}

char *UPnPService::getStateVariableListXML() {
  int l = 40;
  int i;
  for (i=0; i<nvariables; i++) {
    l += variables[i].sendEvents ? 70 : 55;
    l += strlen(variables[i].name) + strlen(variables[i].dataType);
  }
#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("getStateVariableListXML : alloc %d\n", l);
#endif
  char *r = new char[l];	// FIXME
  strcpy(r, "<serviceStateTable>\r\n");
  for (i=0; i<nvariables; i++) {
    if (variables[i].sendEvents)
      strcat(r, "<stateVariable sendEvents=\"yes\">");
    else
      strcat(r, "<stateVariable>");
    strcat(r, "<name>");
    strcat(r, variables[i].name);
    strcat(r, "</name><dataType>");
    strcat(r, variables[i].dataType);
    strcat(r, "</dataType>");
  }
  strcat(r, "</serviceStateTable>\r\n");
#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("getStateVariableListXML : len %d\n", strlen(r));
#endif
  return r;
}

extern ESP8266WebServer HTTP;

void SendSCPD() {
  UPnP.SCPD(HTTP.client());
}

void SendDescription() {
  UPnP.schema(HTTP.client());
}

void UPnPService::begin() {
  HTTP.on(_scpd_xml, HTTP_GET, SendSCPD);
  HTTP.on(_description_xml, HTTP_GET, SendDescription);
}
