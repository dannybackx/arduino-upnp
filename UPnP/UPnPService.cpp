/*
 * ESP8266 Simple UPnP Service framework
 *   Simple means little or no support for lots of services and devices.
 *   An IoT device probably implements just one thing...
 *
 * Copyright (c) 2015, 2016 Danny Backx
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
#include "UPnP/Headers.h"
#include "UPnP/Configuration.h"

#undef	UPNP_DEBUG
// #define	UPNP_DEBUG Serial

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

UPnPService::UPnPService() {
#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("UPnPService()\n");
#endif
  nactions = 0;
  maxsubscribers = nsubscribers = 0;
  subscriber = NULL;
  actions = new Action [N_ACTIONS];

  // Initial allocation
  maxvariables = nvariables = 0;
  variables = NULL;

  line = NULL;

  this->serviceName = NULL;
  this->serviceType = NULL;
  this->serviceId = NULL;
}

UPnPService::UPnPService(const char *name, const char *serviceType, const char *serviceId) {
#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("UPnPService(%s), %p\n", name, this);
#endif
  nactions = 0;
  maxsubscribers = nsubscribers = 0;
  subscriber = NULL;
  actions = new Action [N_ACTIONS];

  // Initial allocation
  maxvariables = nvariables = 0;
  variables = NULL;

  line = NULL;

  this->serviceName = name;
  this->serviceType = serviceType;
  this->serviceId = serviceId;
}

UPnPService::~UPnPService() {
#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("UPnPService DTOR\n");
#endif
  delete actions;

  for (int i=0; i<maxvariables; i++)
    if (variables[i])
      free(variables[i]);
  free(variables);

  if (line)
    free(line);

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
#ifdef UPNP_DEBUGx
  UPNP_DEBUG.printf("UPnPService UPnPService::addStateVariable %s\n", name);
#endif
  if (nvariables == maxvariables) {
    maxvariables += N_VARIABLES;
    variables = (StateVariable **)realloc(variables, maxvariables * sizeof(StateVariable *));;
    for (int i=nvariables; i<maxvariables; i++)
      variables[i] = NULL;
  }
  StateVariable *sv = (StateVariable *)malloc(sizeof(StateVariable));
  variables[nvariables++] = sv;

  sv->name = name;
  sv->dataType = datatype;
  sv->sendEvents = sendEvents;
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
  UPNP_DEBUG.printf("UPnPService UPnPService::getServiceXML -> %s\n", r);
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

#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("UPnPService UPnPService::getActionListXML -> %s\n", r);
#endif

  return r;
}

char *UPnPService::getStateVariableListXML() {
  int l = 40;
  int i;

  for (i=0; i<nvariables; i++)
    if (variables[i]) {
      l += variables[i]->sendEvents ? 86 : 71;
      l += strlen(variables[i]->name) + strlen(variables[i]->dataType);
    }

  char *r = new char[l];	// FIXME
  strcpy(r, "<serviceStateTable>\r\n");
  for (i=0; i<nvariables; i++) {

    if (variables[i]) {
      if (variables[i]->sendEvents)
        strcat(r, "<stateVariable sendEvents=\"yes\">");
      else
        strcat(r, "<stateVariable>");
      strcat(r, "<name>");
      strcat(r, variables[i]->name);
      strcat(r, "</name><dataType>");
      strcat(r, variables[i]->dataType);
      strcat(r, "</dataType>");
      strcat(r, "</stateVariable>");
    }
  }
  strcat(r, "</serviceStateTable>\r\n");

  return r;
}

extern WebServer HTTP;
UPnPService *srv;

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

void UPnPService::ControlHandler() {
#ifdef UPNP_DEBUGmem
  UPNP_DEBUGmem.print("GetFreeHeap1 : "); UPNP_DEBUG.println(ESP.getFreeHeap());
#endif

  char *msg;
  int len;

  HTTP.ReadData(len, msg);
#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("UPnPService::ControlHandler Message len %d : >>>> %s <<<<\n", len, msg);
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

extern void SendSCPD();

void UPnPService::begin(Configuration *cfg) {
  HTTP.on(_description_xml, HTTP_GET, SendDescription);

#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("UPnPService::begin(%s,%s,%s)\n", serviceName, _scpd_xml, _event_xml);
#endif
 
  int len = strlen(_scpd_xml) + 3 + strlen(serviceName);
  char *url = (char *)malloc(len);
  sprintf(url, "/%s/%s", serviceName, _scpd_xml);
  HTTP.on(url, HTTP_GET, staticSendSCPD);
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
  // FIXME which HTTP commands should we implement ?
  // HTTP.on(url, HTTP_GET, staticEventHandler);
  HTTP.on(url, HTTP_SUBSCRIBE, staticEventHandler);
  HTTP.on(url, HTTP_UNSUBSCRIBE, staticEventHandler);
  //HTTP.on(url, UPnPService::EventHandler);
#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("UPnPService::begin(%s)\n", url); 
#endif
  free(url);

  srv = this;

  // Configuration
  config = cfg;
  if (config != NULL) {
    ReadConfiguration(config->GetName(), config);
  }
}

void UPnPService::SendNotify(const char *varName) {
#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("UPnPService::SendNotify(%s), %d\n", varName, nsubscribers); 
#endif
  for (int i=0; i < nsubscribers; i++) {
    UPnPSubscriber *s = subscriber[i];
    s->SendNotify(varName);
  }
}

void UPnPService::SendNotify(UPnPSubscriber *s, const char *varName) {
#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("UPnPService::SendNotify(_, %s)\n", varName); 
#endif
  s->SendNotify(varName);
}

static char *_upnp_subscribe_reply_template =
  "Server: Arduino/ESP8266 UPnP 0.1 © 2015 Danny Backx\r\n"
  "SID: %s\r\n"
  "ACCEPTED-STATEVAR: %s\r\n"
  "\r\n";

/*
acer: {303} subscribe
Query 192.168.1.144 ...
* Hostname was NOT found in DNS cache
*   Trying 192.168.1.144...
* Connected to 192.168.1.144 (192.168.1.144) port 49157 (#0)
> SUBSCRIBE /upnp/event/basicevent1 HTTP/1.0
> Host: 192.168.1.144:49157
> Content-type: text/xml; charset="utf-8"
> CALLBACK: "<http://192.168.1.176:1234/testupnp>"
> NT: upnp:event
> TIMEOUT: Second-600
> 
* HTTP 1.0, assume close after body
< HTTP/1.0 200 OK
< DATE: Fri, 04 Dec 2015 22:21:05 GMT
< SERVER: Linux/2.6.21, UPnP/1.0, Portable SDK for UPnP devices/1.6.6
< CONTENT-LENGTH: 0
< X-User-Agent: redsonic
< SID: uuid:4f3e11b4-1dd2-11b2-942b-831c34640f7b
< TIMEOUT: Second-600
< 
* Closing connection 0
acer: {304} 
 */
/*
 * This is configured by HTTP.on() in UPnPService::begin(), but then
 * treated in UPnPClass to determine which UPnPService got called
 * (by looking up the URL). Then the method below is called on the
 * right UPnPService.
 */
void UPnPService::EventHandler() {
#ifdef UPNP_DEBUG
  UPNP_DEBUG.println("UPnPService::EventHandler()");
#endif

  // Ok now we're good to go, the variable "srv" points right
  if (HTTP.method() == HTTP_SUBSCRIBE) {
    // Register the new subscriber
    UPnPSubscriber *ns = Subscribe();
  } else if (HTTP.method() == HTTP_UNSUBSCRIBE) {
    // FIXME
    Unsubscribe();
  } else {
    // silently ignore
  }
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
UPnPSubscriber *UPnPService::Subscribe() {
  UPnPSubscriber *ns = new UPnPSubscriber(srv);
  Subscribe(ns);

  // Setup its parameters
  ns->setUrl(upnp_headers[UPNP_METHOD_CALLBACK]);
  ns->setStateVarList(upnp_headers[UPNP_METHOD_STATEVAR]);
  ns->setTimeout(upnp_headers[UPNP_METHOD_TIMEOUT]);

#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("Subscribe URL %s\n", upnp_headers[UPNP_METHOD_CALLBACK]);
#endif

  // Provide feedback
  char *asv = ns->getAcceptedStateVar();
  char *sid = ns->getSID();
  char *fb = (char *)malloc(strlen(_upnp_subscribe_reply_template)
    + strlen(sid) + (asv ? strlen(asv) : 0) + 4);
  sprintf(fb, _upnp_subscribe_reply_template, sid, asv ? asv : "");
  // free(sid);	// Don't free SID. FIXME this is confusing.
  if (asv) free(asv);
  HTTP.send(200, UPnPClass::mimeTypeText, fb);

  return ns;
}

/*
 * Add this new subscriber
 */
void UPnPService::Subscribe(UPnPSubscriber *ns) {
  // Allocate array increments per 4 entries
  if (nsubscribers == maxsubscribers) {
    maxsubscribers += SUBSCRIBER_ALLOC_INCREMENT;
    subscriber = (UPnPSubscriber **)realloc(subscriber, maxsubscribers * sizeof(UPnPSubscriber *));

    // NULLify new allocation
    for (int i=nsubscribers; i<maxsubscribers; i++)
      subscriber[i] = NULL;
  }

  // Put new class instance in the first free spot
  nsubscribers++;
  for (int i=0; i<maxsubscribers; i++)
    if (subscriber[i] == NULL) {
      subscriber[i] = ns;

#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("Subscribe -> nsubs %d (UPnPService %p)\n", nsubscribers, this);
#endif
      return;
    }

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

void UPnPService::Unsubscribe(char *uuid) {
#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("Unsubscribe(%s)\n", uuid);
#endif
  UPnPSubscriber *p = (UPnPSubscriber *)strtol(uuid, NULL, 16);
  Unsubscribe(p);
}

void UPnPService::Unsubscribe(UPnPSubscriber *sp) {
#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("Unsubscribe ptr %p\n", sp);
#endif
  int i;
  for (int i=0; i<nsubscribers; i++)
    if (subscriber[i] == sp) {
      // We don't realloc here, just free up this spot. Fits with code in Subscribe().
      // Note this strategy does not preserve order of subscribers.
      // That can be fixed by moving the trailing subscribers down here.
      subscriber[i] = NULL;
      nsubscribers--;

      return;
    }
}

StateVariable *UPnPService::lookupVariable(char *name) {
#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("lookupVariable(%s)\n", name);
#endif

  for (int i=0; i<nvariables; i++)
    if (variables[i])
      if (strcasecmp(name, variables[i]->name) == 0)
        return variables[i];
  return NULL;
}

void UPnPService::SendSCPD(WiFiClient client) {
  uint32_t ip = WiFi.localIP();
  client.print(_http_header);

  char *al = getActionListXML();
  char *svl = getStateVariableListXML();

  int len = strlen(_upnp_scpd_template) + strlen(al) + strlen(svl);
  char *scpd = (char *)malloc(len);
  sprintf(scpd, _upnp_scpd_template, al, svl);
  client.print(scpd);
  free(scpd);
  free(al);
  free(svl);
}
