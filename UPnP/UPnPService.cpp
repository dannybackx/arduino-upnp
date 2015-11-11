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

String serviceId, serviceType;
Action *actions;
int nactions;
StateVariable *variables;
int nvariables;

UPnPService::UPnPService(String serviceType, String serviceId) {
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

#if 0
void UPnPService::addAction(String name, ActionFunction handler) {
#ifdef UPNP_DEBUG
  UPNP_DEBUG.print("UPnPService.addAction(");
  UPNP_DEBUG.print(name);
  UPNP_DEBUG.println(")");
#endif
  // FIXME intentionally no bounds checking code
  actions[nactions].name = name;
  actions[nactions].handler = handler;
  nactions++;
}
#endif

void UPnPService::addAction(String name, ActionFunction handler, String xml) {
#ifdef UPNP_DEBUG
  UPNP_DEBUG.print("UPnPService.addAction(");
  UPNP_DEBUG.print(name);
  UPNP_DEBUG.print(",_,");
  UPNP_DEBUG.print(xml);
  UPNP_DEBUG.println(")");
#endif
  // FIXME intentionally no bounds checking code
  actions[nactions].name = name;
  actions[nactions].handler = handler;
  actions[nactions].xml = xml;
  nactions++;
}

void UPnPService::addStateVariable(String name, String datatype, boolean sendEvents) {
#ifdef UPNP_DEBUG
  UPNP_DEBUG.print("UPnPService.addStateVariable(");
  UPNP_DEBUG.print(name);
  UPNP_DEBUG.print(",");
  UPNP_DEBUG.print(datatype);
  UPNP_DEBUG.print(",");
  UPNP_DEBUG.print(sendEvents);
  UPNP_DEBUG.println(")");
#endif

  variables[nvariables].name = name;
  variables[nvariables].dataType = datatype;
  variables[nvariables].sendEvents = sendEvents;
  nvariables++;
}

String UPnPService::getServiceXML() {
  String r = "<service>";
  r += "<serviceType>" + serviceType;
  r += "</serviceType><serviceId>" + serviceId;
  r += "</serviceId>"
    "<controlURL>/control</controlURL>"
    "<eventSubURL>/event</eventSubURL>"
    "<SCPDURL>/sensor.xml</SCPDURL>"
    "</service>";
  return r;
}

String UPnPService::getActionListXML() {
  String r;
  int i;
  r = "<ActionList>\r\n";
  for (i=0; i<nactions; i++)
    r += actions[i].xml;
  r += "</ActionList>\r\n";
  return r;
}

String UPnPService::getStateVariableListXML() {
  String r;
  int i;
  r = "<serviceStateTable>\r\n";
  for (i=0; i<nvariables; i++) {
    if (variables[i].sendEvents)
      r += "<stateVariable sendEvents=\"yes\">";
    else
      r += "<stateVariable>";
    r += "<name>";
    r += variables[i].name;
    r += "</name><dataType>";
    r += variables[i].dataType;
    r += "</dataType>";
  }
  r += "</serviceStateTable>\r\n";
  return r;
}
