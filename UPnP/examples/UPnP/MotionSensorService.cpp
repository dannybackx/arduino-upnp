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
// Note :
//   avoid GPIO6 (D0) : it flashes the LED in the center of the ESP-12E board, but crashes the board when used as output
//   avoid GPIO9 (SD2) : appears to be linked to "5V POWER"
//const int sensor = 5;   // ESP8266-12E line D1 (GPIO5)
//const int sensor = 4;   // ESP8266-12E line D2 (GPIO4)
const int led = 0;      // ESP8266-12E D3 (GPIO0), beware of using this : enter flash mode after reboot if LOW
// ESP8266-12E line D6 (GPIO12)
// ESP8266-12E line D5 (GPIO14)
// ESP8266-12E line D7 (GPIO13)
// ESP8266-12E line D8 (GPIO15)
// ESP8266-12E line SD3 (GPIO10)

// Printf style template, parameters : serviceType, state
static const char *gsh_template = "<u:GetStateResponse xmlns=\"%s\">\r\n<State>%s</State>\r\n</u:GetStateResponse>\r\n";

static const char *getStateXML PROGMEM = "<action>"
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
  config = new Configuration("MotionSensor",
    new ConfigurationItem("pin", 4),
    new ConfigurationItem("name", ""),
    NULL);
  UPnPService::begin(config);

  sensorpin = config->GetValue("pin");

#ifdef DEBUG
  DEBUG.printf("MotionSensorService::begin (sensor pin %d)\n", sensorpin);
#endif
  pinMode(sensorpin, INPUT);
  oldstate = newstate = digitalRead(sensorpin);
#ifdef HAVE_LED
  pinMode(led, OUTPUT);
#endif
}

void MotionSensorService::poll() {
  oldstate = newstate;
  newstate = digitalRead(sensorpin);

  if (oldstate != newstate) {
#ifdef HAVE_LED
    digitalWrite(led, newstate);
#endif
    sprintf(state, "%d", newstate);
//    Serial.printf("State changed to %d (MotionSensorService %p)\n", newstate, this);

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
