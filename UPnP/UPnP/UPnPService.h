/*
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
 * 
 */
#ifndef	__UPnPService_H_
#define	__UPnPService_H_

#include "debug.h"
#include "UPnP/WebServer.h"

#define	N_ACTIONS	4
#define	N_VARIABLES	4

class UPnPService;
typedef void (UPnPService::*MemberActionFunction)();
typedef void (*ActionFunction)();

typedef struct {
  const char *name;
  UPnPService *sensor;
  ActionFunction handler;
  MemberActionFunction mhandler;
  const char *xml;
} Action;

typedef struct {
  const char *name, *dataType;
  boolean sendEvents;
} StateVariable;

typedef struct {
  const char *url;
  int timeout;
  StateVariable *variables;	// List of variables watched
  char *sid;			// Subscription UUID
} Subscriber;

class UPnPService {
  public:
    UPnPService(const char *name, const char *serviceType, const char *serviceId);
    ~UPnPService();

    // Use the XML to publish a callable action
    // If it is called (via a UPnP query to our web server), call the handler function.
    // There are two types of handler functions : a member function, or a static function.
    void addAction(const char *name, ActionFunction handler, const char *xml);
    void addAction(const char *name, MemberActionFunction handler, const char *xml);

    // Define a state variable
    void addStateVariable(const char *name, const char *datatype, boolean sendEvents);
    void VariableChanged(const char *name, const char *value);
    char *getActionListXML();
    char *getStateVariableListXML();
    char *getServiceXML();
    void begin();
    Action *findAction(const char *);
    static void EventHandler();
    static void ControlHandler();

    int nactions, nvariables;
    Action *actions;
    StateVariable *variables;

    const char *serviceName;
    const char *serviceId;
    const char *serviceType;

    void Subscribe();
    void Unsubscribe();
    void SendNotify();

  private:

  protected:

};

#endif
