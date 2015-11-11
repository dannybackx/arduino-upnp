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
#include "ESP8266WebServer.h"

#define	N_ACTIONS	4
#define	N_VARIABLES	4

typedef void (*ActionFunction)(ESP8266WebServer);

typedef struct {
  String name;
  ActionFunction handler;
  String xml;
} Action;

typedef struct {
  String name, dataType;
  boolean sendEvents;
} StateVariable;

class UPnPService {
  public:
    UPnPService(String serviceType, String serviceId);
    ~UPnPService();
    //void addAction(String name, ActionFunction handler);
    void addAction(String name, ActionFunction handler, String xml);
    void addStateVariable(String name, String datatype, boolean sendEvents);
    String getActionListXML();
    String getStateVariableListXML();
    String getServiceXML();

  private:

  protected:
    String serviceId;
    String serviceType;
    int nactions;
    Action *actions;
    StateVariable *variables;
};

#endif
