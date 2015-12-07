/*
 * This is a sample of a UPnP service that runs on a IoT device.
 * 
 * UPnP commands/queries can be used from an application or a script.
 * This device is a PIR motion detector.
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

#include "UPnP.h"
#include "UPnP/UPnPService.h"
#include "MotionSensorService.h"
#include "UPnP/WebServer.h"

extern WebServer HTTP;
static void GetVersion();

#define DEBUG Serial
const int sensor = 5;   // ESP8266-12E line D1 (GPIO5)
const int led = 0;      // ESP8266-12E D3 (GPIO0)

// Printf style template, parameters : serviceType, state
static const char *gsh_template = "<u:GetStateResponse xmlns=\"%s\">\r\n<State>%s</State>\r\n</u:GetStateResponse>\r\n";

static const char *getStateXML = "<action>"
  "<name>getState</name>"
  "<argumentList>"
  "<argument>"
  "<retval/>"
  "<name>State</name>"
  "<relatedStateVariable>State</relatedStateVariable>"
  "<direction>out</direction>"
  "</argument>"
  "</argumentList>"
  "</action>";

static const char *getVersionXML = "<action>"
  "<name>getVersion</name>"
  "<argumentList>"
  "<argument>"
  "<retval/>"
  "<name>Version</name>"
  "<direction>out</direction>"
  "</argument>"
  "</argumentList>"
  "</action>";

// These are static/global variables as a demo that you can query such variables.
static const char *versionTemplate = "%s %s %s\n";
static const char *versionFileInfo = __FILE__;
static const char *versionDateInfo = __DATE__;
static const char *versionTimeInfo = __TIME__;

static const char *myServiceName = "motionSensor";
static const char *myServiceType = "urn:danny-backx-info:service:sensor:1";
static const char *myServiceId = "urn:danny-backx-info:serviceId:sensor1";
static const char *stateString = "State";
static const char *getStateString = "getState";
static const char *getVersionString = "getVersion";
static const char *stringString = "string";

MotionSensorService::MotionSensorService() :
  UPnPService(myServiceName, myServiceType, myServiceId)
{
  addAction(getStateString, static_cast<MemberActionFunction>(&MotionSensorService::GetStateHandler), getStateXML);
  addAction(getVersionString, GetVersion, getVersionXML);
  addStateVariable(stateString, stringString, true);
  begin();
}

MotionSensorService::MotionSensorService(const char *deviceURN) :
  UPnPService(myServiceName, myServiceType, myServiceId)
{
  addAction(getStateString, static_cast<MemberActionFunction>(&MotionSensorService::GetStateHandler), getStateXML);
  addAction(getVersionString, GetVersion, getVersionXML);
  addStateVariable(stateString, stringString, true);
  begin();
}

MotionSensorService::MotionSensorService(const char *serviceType, const char *serviceId) :
  UPnPService(myServiceName, serviceType, serviceId)
{
  addAction(getStateString, static_cast<MemberActionFunction>(&MotionSensorService::GetStateHandler), getStateXML);
  addAction(getVersionString, GetVersion, getVersionXML);
  addStateVariable(stateString, stringString, true);
  begin();
}

MotionSensorService::~MotionSensorService() {
#ifdef DEBUG
  DEBUG.println("MotionSensorService DTOR");
#endif  
}

void MotionSensorService::begin() {
  UPnPService::begin();
#ifdef DEBUG
  DEBUG.println("MotionSensorService::begin");
#endif
  pinMode(sensor, INPUT);
  oldstate = newstate = digitalRead(sensor);
#ifdef HAVE_LED
  pinMode(led, OUTPUT);
#endif

  // Subscribe();
}

void MotionSensorService::poll() {
  oldstate = newstate;
  newstate = digitalRead(sensor);

  if (oldstate != newstate) {
#ifdef HAVE_LED
    digitalWrite(led, newstate);
#endif
    sprintf(state, "%d", newstate);
    //Serial.printf("State changed to %d\n", newstate);

    // FIXME trigger something from here
    SendNotify("State");
  }
}

#ifdef MSS_GLOBAL
MotionSensorService MSS;
#endif

const char *MotionSensorService::GetState() {
  return state;
}

// Example of a static function to handle UPnP requests : only access to global variables here.
static void GetVersion() {
  char msg[128];
  sprintf(msg, versionTemplate, versionFileInfo, versionDateInfo, versionTimeInfo);
  HTTP.send(200, UPnPClass::mimeTypeXML, msg);
}

// Example of a member function to handle UPnP requests : this can access stuff in the class
void MotionSensorService::GetStateHandler() {
  int l2 = strlen(gsh_template) + strlen(myServiceType) + MSS_STATE_LENGTH,
      l1 = strlen(UPnPClass::envelopeHeader) + l2 + strlen(UPnPClass::envelopeTrailer) + 5;
  char *tmp2 = (char *)malloc(l2),
       *tmp1 = (char *)malloc(l1);
#ifdef DEBUG
  DEBUG.println("MotionSensorService::GetStateHandler");
#endif
  strcpy(tmp1, UPnPClass::envelopeHeader);
  sprintf(tmp2, gsh_template, myServiceType, MotionSensorService::state);
  strcat(tmp1, tmp2);
  free(tmp2);
  strcat(tmp1, UPnPClass::envelopeTrailer);
  HTTP.send(200, UPnPClass::mimeTypeXML, tmp1);
  free(tmp1);
}
