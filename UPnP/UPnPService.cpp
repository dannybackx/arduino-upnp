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
#if 0
    /* serviceType */ "urn:danny-backx-info:service:sensor:1",
    /* serviceId */ "urn:danny-backx-info:serviceId:sensor1",
    _control_xml, _event_xml, _scpd_xml);
#else
    serviceType, serviceId, _control_xml, _event_xml, _scpd_xml);
#endif
#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("getServiceXML -> %s\n", r);
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
  Serial.printf("findAction(%s)\n", name);
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
  Serial.print("GetFreeHeap1 : "); Serial.println(ESP.getFreeHeap());
  if (HTTP.args() == 0)
    return;	// empty request ? silently return
  Serial.print("GetFreeHeap2 : "); Serial.println(ESP.getFreeHeap());
  String xmls = HTTP.arg(0);
  Serial.print("GetFreeHeap3 : "); Serial.println(ESP.getFreeHeap());
  int body1 = xmls.indexOf("<s:Body>");
  int body2 = xmls.indexOf("</s:Body>");
  if (body1 < 0 || body2 < 0 || body2 <= body1)
    return;	// Again, silently return

  const char *xml = xmls.substring(body1+8, body2).c_str(),
             *x3 = xml + 3;
  //delete xmls;
  Serial.printf("EventHandler(%s)\n", xml);

  // <u:getState xmlns:u = "urn:upnp-org:serviceId:ContentDirectory"></u:getState>
  if (xml[0] != '<' || xml[1] != 'u' || xml[2] != ':')
    return;
  // We're guessing an action name ends with a space or a >
  int s1 = myindex(xml, ' '),
      s2 = myindex(xml, '>');
  int space = (s1 < s2) ? s1 : s2;
  char *action = new char[space-2];
  strncpy(action, xml+3, space-3);
  action[space-3] = '\0';
  Serial.printf("EventHandler action(%s)\n", action);

  Serial.printf("EventHandler srv(%p)\n", srv);
  Serial.printf("EventHandler actions [0] (%p,%s)\n", srv->actions[0].name, srv->actions[0].name);


  Serial.print("GetChipId : "); Serial.println(ESP.getChipId());
  Serial.print("GetFlashChipId : "); Serial.println(ESP.getFlashChipId());
  Serial.print("GetFreeHeap : "); Serial.println(ESP.getFreeHeap());
  //Serial.println(" ");

  // Serial.printf("EventHandler actions [0] (%p)\n", srv->actions[0].name);
  // Action *pAction = srv->findAction("GetState");
  Action *pAction = srv->findAction(action);

  free(action);
  if (pAction == 0)
    return;

  Serial.printf("Have it ... %s\n", pAction->name);
  ActionFunction fn = pAction->handler;
 
  Serial.printf("Function ptr ... %p\n", fn);
  /*
  Serial.printf("Call it ...\n");
  (*fn)(HTTP);
  return;

  /* */
  HTTP.send(200, "text/xml; charset=\"utf-8\"",
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n"
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">\r\n"
    "<s:body>\r\n"
    "<u:GetStateResponse xmlns=\"urn:danny-backx-info:service:sensor:1\">\r\n"
    "<State>0</State>\r\n"
    "</u:GetStateResponse>\r\n"
    "</s:body>\r\n"    
    "</s:Envelope>\r\n"
   );
   /* */
}

void UPnPService::begin() {
  HTTP.on(_scpd_xml, HTTP_GET, SendSCPD);
  HTTP.on(_description_xml, HTTP_GET, SendDescription);
  HTTP.on(_event_xml, UPnPService::EventHandler);
  srv = this;
  Serial.printf("UPnPService::begin(), this %p, srv %p\n", this, srv);
}
