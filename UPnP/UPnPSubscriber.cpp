/*
 * Manage subscribing with this device for receiving notifications,
 * and also send these notifications.
 * The notifications are basically event reports that some variable changed state.
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
#include "UPnP/Headers.h"

//#undef	UPNP_DEBUG
#define	UPNP_DEBUG Serial

static const char *_notify_header_template =
  "NOTIFY %s HTTP/1.0\r\n"
  "HOST: %s:%p\r\n"
  "CONTENT-TYPE: text/xml; charset=\"utf-8\"\r\n"
  "NT: upnp:event\r\n"
  "NTS: upnp:propchange\r\n"
  "SID: %s\r\n"
  "SEQ: %d\r\n"
  "\r\n"
  ;
static const char *_notify_body_template =
  "<?xml version=\"1.0\"?>\r\n"
  "<e:propertyset xmlns:e=\"urn:schemas-upnp-org:event-1-0\">\r\n"
  "<e:property>\r\n"
  "<variableName>%s</variableName>\r\n"
  "</e:property>\r\n"
  "</e:propertyset>\r\n"
  "\r\n"
  ;

char *upnp_headers[UPNP_END_METHODS];

void UPnPSubscriber::SendNotify(StateVariable &sv) {
  SendNotify(sv.name);
}

/*
 * NOTIFY delivery path HTTP/1.0
 * HOST: delivery host:delivery port
 * CONTENT-TYPE: text/xml; charset="utf-8"
 * NT: upnp:event
 * NTS: upnp:propchange
 * SID: uuid:subscription-UUID
 * SEQ: event key
 * CONTENT-LENGTH: bytes in body
 * <?xml version="1.0"?>
 * <e:propertyset xmlns:e="urn:schemas-upnp-org:event-1-0">
 * <e:property>
 * <variableName>new value</variableName>
 * </e:property>
 * Other variable names and values (if any) go here.
 * </e:propertyset>
 */
void UPnPSubscriber::SendNotify(const char *varName) {
#ifdef UPNP_DEBUG
  UPNP_DEBUG.println("SendNotify");
#endif
  if (varName == NULL)
    return;	// FIXME Silently ignore
  char *body = (char *)malloc(strlen(varName) + strlen(_notify_body_template));
  sprintf(body, _notify_body_template, varName);

  char *header = (char *)malloc(strlen(_notify_header_template) + 128);
  sprintf(header, _notify_header_template,
    "delivery/path",		// FIXME
    "host", 45678,		// FIXME
    sid,			// Use the memory address of this instance as UUID, see ctor
    seq++);

  char *msg = (char *)malloc(strlen(body) + strlen(header) + 40);
  sprintf(msg, "%s\r\nContent-Length: %d\r\n%s", header, strlen(body), body);

  if (wc == NULL) {
    wc = new WebClient();
    wc->connect(url);
  }
  if (wc)
    wc->send(UPnPClass::mimeTypeXML, msg);
  free(msg);
  free(body);
  free(header);
}

void UPnPSubscriber::setService(UPnPService *s) {
  service = s;
}

UPnPSubscriber::UPnPSubscriber() {
#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("UPnPSubscriber::UPnPSubscriber(%p)\n", this);
#endif

  wc = NULL;
  seq = 1;
  sid = (char *)malloc(16);
  sprintf(sid, "uuid:%08x", this);
  nvariables = 0;
  variables = NULL;
}

UPnPSubscriber::~UPnPSubscriber() {
#ifdef UPNP_DEBUG
  UPNP_DEBUG.println("UPnPSubscriber::~UPnPSubscriber");
#endif
  if (wc)
    delete wc;
  free(sid);
}

void UPnPSubscriber::setUrl(char *url) {
  this->url = url;	// FIXME make a copy ?
}

/*
 * Cut a list of state variable (comma separated) into separate variable names.
 * Then try to subscribe to info on them
 */
void UPnPSubscriber::setStateVarList(char *stateVarList) {
  char *ptr, *begin, **list;
  bool inword = false;
  int n = 0;

  // First pass : count words
  for (ptr = stateVarList; *ptr; ptr++) {
    // UPNP_DEBUG.printf("sSVL(%s) %d %s\n", ptr, n, inword ? "inword" : "out");
    if (inword && !isalnum(*ptr)) {
      inword = false;
    } else if (inword && isalnum(*ptr)) {
      ;
    } else if (inword == false && !isalnum(*ptr)) {
      ;
    } else if (inword == false && isalnum(*ptr)) {
      n++;
      inword = true;
    }
  }

  // Allocate
#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("UPnPSubscriber::setStateVarList(%s) : %d words\n", stateVarList, n);
  delay(1000);
#endif
  list = (char **)malloc(sizeof(char *) * n);

#define PICKUP() \
      int len = ptr - begin; \
      list[i] = (char *)malloc(len); \
      strncpy(list[i], begin, len); \
      list[i][len] = 0; \
      setStateVar(list[i]); \
      i++;

  // Second pass : record
  int i = 0;
  inword = false;
  for (ptr = stateVarList; *ptr; ptr++) {
    if (inword && !isalnum(*ptr)) {
      inword = false;
      PICKUP();
    } else if (inword && isalnum(*ptr)) {
      ;
    } else if (inword == false && !isalnum(*ptr)) {
      ;
    } else if (inword == false && isalnum(*ptr)) {
      // n++;
      inword = true;
      begin = ptr;
    }
  }
  PICKUP();
}

/*
 * Subscribe to notifications for this state variable.
 * This function silently ignores failures, but the result will be reported back
 * by the feedback of one of our callers. (See UPnPService::Subscribe.)
 */
void UPnPSubscriber::setStateVar(char *name) {
  StateVariable *vp = service->lookupVariable(name);
  if (! vp)
    return;	// Silently ignore

  // Add this to the watch list
  int ix = nvariables++;
  variables = (StateVariable **)realloc(variables, nvariables * sizeof(StateVariable *));
  variables[ix] = vp;

#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("Subscribe : StateVar(%s) %d\n", name, nvariables);
  delay(1000);
#endif
}

void UPnPSubscriber::setTimeout(char *timeout) {
}

char *UPnPSubscriber::getSID() {
  return sid;
}

// Caller should free result.
char *UPnPSubscriber::getAcceptedStateVar() {
  // Quickly discard empty list.
  // Return NULL, not "" to be able to detect what to free in the caller.
  if (nvariables == 0)
    return NULL;

  // Calculate allocation size
  int len = 0;
  for (int i=0; i<nvariables; i++)
    len += strlen(variables[i]->name) + 1;
  char *r = (char *)malloc(len);

  // Create the list. There's always one, see the test above.
  strcpy(r, variables[0]->name);
  for (int i=1; i<nvariables; i++) {
    strcat(r, ",");
    strcat(r, variables[i]->name);
  }
#ifdef UPNP_DEBUG
  UPNP_DEBUG.printf("getAcceptedStateVar() -> %s\n", r);
#endif
  return r;
}
