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
static const char *_control_xml = "/control";
static const char *_event_xml = "/event";

static const char *_get_service_xml_template =
  "<service>"
    "<serviceType>%s</serviceType>"
    "<serviceId>%s</serviceId>"
    "<controlURL>%s</controlURL>"
    "<eventSubURL>%s</eventSubURL>"
    "<SCPDURL>%s</SCPDURL>"
  "</service>";

UPnPService::UPnPService(const char *serviceType, const char *serviceId) {
  nactions = 0;
  nvariables = 0;
  actions = new Action [N_ACTIONS];
  variables = new StateVariable [N_VARIABLES];

  this->serviceType = serviceType;
  this->serviceId = serviceId;
}

UPnPService::~UPnPService() {
#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("UPnPService DTOR\n");
#endif
  delete actions;
  delete variables;
}

void UPnPService::addAction(const char *name, ActionFunction handler, const char *xml) {
  // FIXME intentionally no bounds checking code
  actions[nactions].name = name;
  actions[nactions].handler = handler;
  actions[nactions].xml = xml;
#ifdef UPNP_DEBUGx
  UPNP_DEBUG.printf("UPnPService::addAction[%d](%p - %s,_,%p - %s)\n", nactions, name, name, xml, xml);
  UPNP_DEBUG.printf("UPnPService::addAction -> (%p - %s,_,%p - %s)\n", actions[0].name, actions[0].name, actions[0].xml, actions[0].xml);
#endif
  nactions++;
}

void UPnPService::addStateVariable(const char *name, const char *datatype, boolean sendEvents) {
  variables[nvariables].name = name;
  variables[nvariables].dataType = datatype;
  variables[nvariables].sendEvents = sendEvents;
  nvariables++;
}

// Caller must free return pointer
char *UPnPService::getServiceXML() {
  char *r = new char[250];	// FIXME, should 170 + serviceType + serviceId, currently 240 is ok

  sprintf(r, _get_service_xml_template,
    serviceType, serviceId, _control_xml, _event_xml, _scpd_xml);

#ifdef UPNP_DEBUGx
  UPNP_DEBUG.printf("getServiceXML -> %s\n", r);
#endif
  return r;
}

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

extern WebServer HTTP;
UPnPService *srv;

void SendSCPD() {
  UPnP.SCPD(HTTP.client());
}

void SendDescription() {
  UPnP.schema(HTTP.client());
}

static int myindex(const char *ptr, char c) {
  int i;
  for (i=0; ptr[i] != c && ptr[i] != '\0'; i++)
    ;
  if (ptr[i] == c)
    return i;
  return -1;
}

Action * UPnPService::findAction(const char *name) {
#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("findAction(%s)\n", name);
#endif
  int i;
  for (i=0; i<nactions; i++) {
    if (strcasecmp(name, actions[i].name) == 0)
      return &actions[i];
  }
  return 0;
}

void UPnPService::EventHandler() {
  // Fetch the request, this is encoded as a parameter name and its value,
  // we could reassemble that but there's no point :
  // HTTP.argName(0) --> <?xml version
  // HTTP.arg(0) --> "1.0" encoding="utf-8?><s:Envelope xmlns:s="http://schemas.xmlsoap.org...
  // so we only need HTTP.arg(0)
#ifdef UPNP_DEBUG
  UPNP_DEBUG.print("GetFreeHeap1 : "); UPNP_DEBUG.println(ESP.getFreeHeap());
#endif

  const char *msg = HTTP.plainBuf;
#ifdef UPNP_DEBUGx
  UPNP_DEBUG.printf("Message len %d : >>>> %s <<<<\n", HTTP.plainLen, msg);
#endif
  const char *body1 = strstr(msg, "<s:Body>");
  const char *body2 = strstr(msg, "</s:Body>");
  if (body2 < body1)
    return;	// Silently return

  body1 += 8;	// bypass <s:Body>
  int bodylen = (body2 - body1);
  char *xml = (char *)malloc(bodylen+1);
  strncpy(xml, body1, bodylen);
  xml[bodylen] = '\0';
#ifdef UPNP_DEBUGx
  UPNP_DEBUG.printf("Body : >>>> %s <<<<\n", xml);
#endif

  // <u:getState xmlns:u = "urn:upnp-org:serviceId:ContentDirectory"></u:getState>
  if (xml[0] != '<' || xml[1] != 'u' || xml[2] != ':') {
    free(xml);
    return;
  }
  // We're guessing an action name ends with a space or a >
  int s1 = myindex(xml, ' '),
      s2 = myindex(xml, '>');
  int space = (s1 < s2) ? s1 : s2;
  char *action = new char[space-2];
  strncpy(action, xml+3, space-3);
  action[space-3] = '\0';
  free(xml);

#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("EventHandler action(%s)\n", action);

  UPNP_DEBUG.printf("EventHandler srv(%p)\n", srv);
  UPNP_DEBUG.printf("EventHandler actions [0] (%p,%s)\n", srv->actions[0].name, srv->actions[0].name);


  UPNP_DEBUG.print("GetChipId : "); UPNP_DEBUG.println(ESP.getChipId());
  UPNP_DEBUG.print("GetFlashChipId : "); UPNP_DEBUG.println(ESP.getFlashChipId());
  UPNP_DEBUG.print("GetFreeHeap : "); UPNP_DEBUG.println(ESP.getFreeHeap());
  // UPNP_DEBUG.printf("EventHandler actions [0] (%p)\n", srv->actions[0].name);
#endif

  Action *pAction = srv->findAction(action);
  free(action);
  if (pAction == 0)
    return;

#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("Have it ... %s\n", pAction->name);
#endif
  ActionFunction fn = pAction->handler;
 
#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("Function ptr ... %p\n", fn);
#endif

  (*fn)();
}

void UPnPService::begin() {
  HTTP.on(_scpd_xml, HTTP_GET, SendSCPD);
  HTTP.on(_description_xml, HTTP_GET, SendDescription);
  HTTP.on(_event_xml, UPnPService::EventHandler);
  srv = this;
#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("UPnPService::begin(), this %p, srv %p\n", this, srv);
#endif
}
