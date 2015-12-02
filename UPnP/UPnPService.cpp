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
#include "UPnP/WebClient.h"

//#undef	UPNP_DEBUG
#define	UPNP_DEBUG Serial
#undef	UPNP_DEBUGx
//#define	UPNP_DEBUGx Serial
#undef UPNP_DEBUGmem
//#define UPNP_DEBUGmem Serial

static const char *_description_xml = "/description.xml";

// Per service : no preceding "/" as this will be concatenated.
static const char *_scpd_xml = "scpd.xml";
static const char *_control_xml = "control";
static const char *_event_xml = "event";

static const char *_get_service_xml_template =
  "<service>"
    "<serviceType>%s</serviceType>"
    "<serviceId>%s</serviceId>"
    "<controlURL>/%s/%s</controlURL>"
    "<eventSubURL>/%s/%s</eventSubURL>"
    "<SCPDURL>/%s/%s</SCPDURL>"
  "</service>";

UPnPService::UPnPService(const char *name, const char *serviceType, const char *serviceId) {
#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("UPnPService(%s)\n", name);
#endif
  nactions = 0;
  nvariables = 0;
  nsubscribers = 0;
  subscriber = NULL;
  actions = new Action [N_ACTIONS];
  variables = new StateVariable [N_VARIABLES];

  this->serviceName = name;
  this->serviceType = serviceType;
  this->serviceId = serviceId;
}

UPnPService::~UPnPService() {
#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("UPnPService DTOR\n");
#endif
  delete actions;
  delete variables;
  delete subscriber;
}

// Pointer to a member function
void UPnPService::addAction(const char *name, MemberActionFunction handler, const char *xml) {
#ifdef UPNP_DEBUGx
  UPNP_DEBUG.printf("UPnPService::addAction[%d](%s,%p %s)\n", nactions, name, xml, xml);
#endif
  // FIXME intentionally no bounds checking code
  actions[nactions].name = name;
  actions[nactions].mhandler = handler;
  actions[nactions].sensor = this;
  actions[nactions].xml = xml;
  actions[nactions].handler = NULL;
  nactions++;
}

// Pointer to a static function
void UPnPService::addAction(const char *name, ActionFunction handler, const char *xml) {
#ifdef UPNP_DEBUGx
  UPNP_DEBUG.printf("UPnPService::addAction[%d](%s,%p %s)\n", nactions, name, xml, xml);
  //UPNP_DEBUG.printf("UPnPService::addAction[%d](%s,%s)\n", nactions, name, xml);
#endif
  // FIXME intentionally no bounds checking code
  actions[nactions].name = name;
  actions[nactions].handler = handler;
  actions[nactions].xml = xml;
  actions[nactions].mhandler = NULL;
  actions[nactions].sensor = NULL;
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
  int len = strlen(_get_service_xml_template) + strlen(serviceType) + strlen(serviceId)
	  + 3 * strlen(serviceName) + 10
          + strlen(_control_xml) + strlen(_event_xml) + strlen(_scpd_xml);
  char *r = (char *)malloc(len);

  sprintf(r, _get_service_xml_template,
    serviceType, serviceId,
    serviceName, _control_xml,
    serviceName, _event_xml,
    serviceName, _scpd_xml);

#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("getServiceXML -> %s\n", r);
#endif
  return r;
}

static const char *_actionListBegin = "<ActionList>\r\n";
static const char *_actionListEnd = "</ActionList>\r\n";

// Caller needs to free the result
char *UPnPService::getActionListXML() {
  int l = strlen(_actionListBegin) + strlen(_actionListEnd) + 4;
  int i;
  for (i=0; i<nactions; i++)
    l += strlen(actions[i].xml);

  char *r = (char *)malloc(l);
  strcpy(r, _actionListBegin);
  for (i=0; i<nactions; i++) {
    strcat(r, actions[i].xml);
  }
  strcat(r, _actionListEnd);

  return r;
}

char *UPnPService::getStateVariableListXML() {
  int l = 40;
  int i;
  for (i=0; i<nvariables; i++) {
    l += variables[i].sendEvents ? 86 : 71;
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
    strcat(r, "</stateVariable>");
  }
  strcat(r, "</serviceStateTable>\r\n");

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
#ifdef UPNP_DEBUGx
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
#ifdef UPNP_DEBUG
  UPNP_DEBUG.println("UPnPService::EventHandler");
#endif
}

void UPnPService::ControlHandler() {
  // Fetch the request, this is encoded as a parameter name and its value,
  // we could reassemble that but there's no point :
  // HTTP.argName(0) --> <?xml version
  // HTTP.arg(0) --> "1.0" encoding="utf-8?><s:Envelope xmlns:s="http://schemas.xmlsoap.org...
  // so we only need HTTP.arg(0)
#ifdef UPNP_DEBUGmem
  UPNP_DEBUGmem.print("GetFreeHeap1 : "); UPNP_DEBUG.println(ESP.getFreeHeap());
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

#ifdef UPNP_DEBUGx
  UPNP_DEBUG.printf("EventHandler action(%s)\n", action);

  UPNP_DEBUG.printf("EventHandler srv(%p)\n", srv);
  UPNP_DEBUG.printf("EventHandler actions [0] (%p,%s)\n", srv->actions[0].name, srv->actions[0].name);
#endif

#ifdef UPNP_DEBUGmem
  UPNP_DEBUGmem.print("GetFreeHeap : "); UPNP_DEBUG.println(ESP.getFreeHeap());
#endif

  Action *pAction = srv->findAction(action);
  free(action);
  if (pAction == 0)
    return;

  // Two cases : a static handler function, or a pointer to a member function
  ActionFunction fn = pAction->handler;
  MemberActionFunction mfn = pAction->mhandler;
  UPnPService *sensor = pAction->sensor;

  if (mfn != NULL && sensor != NULL) {
    (sensor->*mfn)();
  } else if (fn != NULL) {
    (*fn)();
  }
  // else silently ignore again
}

void UPnPService::begin() {
  HTTP.on(_description_xml, HTTP_GET, SendDescription);

#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("UPnPService::begin(%s,%s,%s)\n", serviceName, _scpd_xml, _event_xml);
#endif
 
  int len = strlen(_scpd_xml) + 3 + strlen(serviceName);
  char *url = (char *)malloc(len);
  sprintf(url, "/%s/%s", serviceName, _scpd_xml);
  HTTP.on(url, HTTP_GET, SendSCPD);
#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("UPnPService::begin(%s)\n", url); 
#endif
  free(url);

  len = strlen(_control_xml) + 3 + strlen(serviceName);
  url = (char *)malloc(len);
  sprintf(url, "/%s/%s", serviceName, _control_xml);
  HTTP.on(url, UPnPService::ControlHandler);
#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("UPnPService::begin(%s)\n", url); 
#endif
  free(url);

  len = strlen(_event_xml) + 3 + strlen(serviceName);
  url = (char *)malloc(len);
  sprintf(url, "/%s/%s", serviceName, _event_xml);
  HTTP.on(url, UPnPService::EventHandler);
#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("UPnPService::begin(%s)\n", url); 
#endif
  free(url);

  srv = this;
}

void UPnPService::SendNotify() {
  if (0 < nsubscribers) {
    UPnPSubscriber *s = subscriber[0];
    s->SendNotify();
  }
}

void UPnPService::SendNotify(UPnPSubscriber *s) {
  s->SendNotify();
}

/*
 * A SUBSCRIBE request was received - process it.
 *
 * SUBSCRIBE publisher path HTTP/1.1
 * HOST: publisher host:publisher port
 * USER-AGENT: OS/version UPnP/2.0 product/version
 * CALLBACK: <delivery URL>
 * NT: upnp:event
 * TIMEOUT: Second-requested subscription duration
 * STATEVAR: CSV of Statevariables
 * (No body for request with method SUBSCRIBE
 *
 * The reply should look like this :
 *
 * HTTP/1.1 200 OK
 * DATE: when response was generated
 * SERVER: OS/version UPnP/2.0 product/version
 * SID: uuid:subscription-UUID
 * CONTENT-LENGTH: 0
 * TIMEOUT: Second-actual subscription duration
 * ACCEPTED-STATEVAR: CSV of state variables
 */
void UPnPService::Subscribe() {
#ifdef UPNP_DEBUG
  UPNP_DEBUG.println("Subscribe");
#endif
  int ix = nsubscribers++;
  subscriber = (UPnPSubscriber **)realloc(subscriber, nsubscribers * sizeof(UPnPSubscriber **));
  UPnPSubscriber *ps = new UPnPSubscriber();
  subscriber[ix] = ps;
}

/*
 * UNSUBSCRIBE publisher path HTTP/1.1
 * HOST: publisher host:publisher port
 * SID: uuid:subscription UUID
 */
void UPnPService::Unsubscribe() {
#ifdef UPNP_DEBUG
  UPNP_DEBUG.println("Unsubscribe");
#endif
}
