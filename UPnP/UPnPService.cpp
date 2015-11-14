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

UPnPService::UPnPService(const char *serviceType, const char *serviceId) {
#ifdef UPNP_DEBUG
  UPNP_DEBUG.println("UPnPService ctor");
#endif
  this->serviceType = serviceType;
  this->serviceId = serviceId;
  nactions = 0;
  nvariables = 0;
  actions = new Action [N_ACTIONS];
  variables = new StateVariable [N_VARIABLES];
}

UPnPService::~UPnPService() {
  delete actions;
  delete variables;
}

void UPnPService::addAction(const char *name, ActionFunction handler, const char *xml) {
#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("UPnPService.addAction(%s,_,%s)\n", name, xml);
#endif
  // FIXME intentionally no bounds checking code
  actions[nactions].name = name;
  actions[nactions].handler = handler;
  actions[nactions].xml = xml;
  nactions++;
}

void UPnPService::addStateVariable(const char *name, const char *datatype, boolean sendEvents) {
#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("UPnPService.addStateVariable(%s,%s,%s)\n",
    name, datatype, sendEvents ? "true" : "false");
#endif

  variables[nvariables].name = name;
  variables[nvariables].dataType = datatype;
  variables[nvariables].sendEvents = sendEvents;
  nvariables++;
}

#if 0
/* FIXME there is clearly a problem in this code
 *
 * Sending description.xml ...
 *
 * UPnPService::getServiceXML() 77
 * UPnPService::getServiceXML() 79, r {}
 * UPnPService::getServiceXML() 81, r {<service>}
 * UPnPService::g
 * 
 * Exception (28):
 *
 * epc1=0x4000bf80 epc2=0x00000000 epc3=0x00000000 excvaddr=0x00000000 depc=0x00000000
 *
 * ctx: cont 
 *
 * sp: 3ffed6d0 end: 3ffeda40 offset: 01a0
 *
 * >>>stack>>>
 *
 * 3ffed870:  00000000 ff000000 3ffed940 3ffec9e8  
 * 3ffed880:  40203633 3ffed8e0 3fff55d8 40207e66  
 */
char *UPnPService::getServiceXML() {
  Serial.printf("UPnPService::getServiceXML() %d\n", __LINE__);
  char *r = new char[256];	// FIXME
  Serial.printf("UPnPService::getServiceXML() %d, r {%s}\n", __LINE__, r);
  strcpy(r, "<service>");
  Serial.printf("UPnPService::getServiceXML() %d, r {%s}\n", __LINE__, r);
  strcat(r, "<serviceType>");
  Serial.printf("UPnPService::getServiceXML() %d, r {%s}\n", __LINE__, r);
  strcat(r, serviceType);
  Serial.printf("UPnPService::getServiceXML() %d, r {%s}\n", __LINE__, r);
  strcat(r, "</serviceType><serviceId>");
  strcat(r, serviceId);
  strcat(r, "</serviceId><controlURL>/control</controlURL><eventSubURL>/event</eventSubURL><SCPDURL>/sensor.xml</SCPDURL></service>");
  return r;
}
#else
char *UPnPService::getServiceXML() {
  return "<service><serviceType>urn:danny-backx-info:service:sensor:1</serviceType><serviceId>urn:danny-backx-info:serviceId:sensor1</serviceId><controlURL>/control</controlURL><eventSubURL>/event</eventSubURL><SCPDURL>/sensor.xml</SCPDURL></service>";
}
#endif

char * UPnPService::getActionListXML() {
  int l = 32;
  int i;
  for (i=0; i<nactions; i++)
    l += strlen(actions[i].xml);
  char *r = new char[l];	// FIXME
  strcpy(r, "<ActionList>\r\n");
  for (i=0; i<nactions; i++)
    strcat(r, actions[i].xml);
  strcat(r, "</ActionList>\r\n");
  return r;
}

char * UPnPService::getStateVariableListXML() {
  int l = 40;
  int i;
  for (i=0; i<nvariables; i++) {
    l += variables[i].sendEvents ? 70 : 55;
    l += strlen(variables[i].name) + strlen(variables[i].dataType);
  }
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
  return r;
}
