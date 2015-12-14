/*
 * This is a sample of a UPnP service that runs on a IoT device.
 * 
 * UPnP commands/queries can be used from an application or a script.
 * This service is a programmable LED.
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
#include "UPnP/LEDService.h"
#include "UPnP/WebServer.h"

extern WebServer HTTP;
static void GetVersion();

#define DEBUG Serial
#define HAVE_LED
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

static const char *setStateXML = "<action>"
  "<name>setState</name>"
  "<argumentList>"
  "<argument>"
  "<name>State</name>"
  "<relatedStateVariable>State</relatedStateVariable>"
  "<direction>in</direction>"
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

static const char *myServiceName = "LEDService";
static const char *myServiceType = "urn:danny-backx-info:service:led:1";
static const char *myServiceId = "urn:danny-backx-info:serviceId:led1";
static const char *stateString = "State";
static const char *getStateString = "getState";
static const char *setStateString = "setState";
static const char *getVersionString = "getVersion";
static const char *stringString = "string";

LEDService::LEDService() :
  UPnPService(myServiceName, myServiceType, myServiceId)
{
  addAction(getStateString, static_cast<MemberActionFunction>(&LEDService::GetStateHandler), getStateXML);
  addAction(setStateString, static_cast<MemberActionFunction>(&LEDService::SetStateHandler), setStateXML);
  addAction(getVersionString, GetVersion, getVersionXML);
  addStateVariable(stateString, stringString, true);
  begin();
}

LEDService::LEDService(const char *deviceURN) :
  UPnPService(myServiceName, myServiceType, myServiceId)
{
  addAction(getStateString, static_cast<MemberActionFunction>(&LEDService::GetStateHandler), getStateXML);
  addAction(setStateString, static_cast<MemberActionFunction>(&LEDService::SetStateHandler), setStateXML);
  addAction(getVersionString, GetVersion, getVersionXML);
  addStateVariable(stateString, stringString, true);
  begin();
}

LEDService::LEDService(const char *serviceType, const char *serviceId) :
  UPnPService(myServiceName, serviceType, serviceId)
{
  addAction(getStateString, static_cast<MemberActionFunction>(&LEDService::GetStateHandler), getStateXML);
  addAction(setStateString, static_cast<MemberActionFunction>(&LEDService::SetStateHandler), setStateXML);
  addAction(getVersionString, GetVersion, getVersionXML);
  addStateVariable(stateString, stringString, true);
  begin();
}

LEDService::~LEDService() {
#ifdef DEBUG
  DEBUG.println("LEDService DTOR");
#endif  
}

void LEDService::begin() {
  if (state != LED_STATE_INVALID)
    return;	// Already been here
  state = LED_STATE_OFF;

  UPnPService::begin();
#ifdef DEBUG
  DEBUG.println("LEDService::begin");
#endif

#ifdef HAVE_LED
  pinMode(led, OUTPUT);
#endif

  state = LED_STATE_OFF;
}

enum LEDState LEDService::GetState() {
  return state;
}

void LEDService::SetState(enum LEDState state) {
  this->state = state;
}

/*
 * Note this depends on how frequently the caller calls this method.
 */
void LEDService::setPeriod(int active, int passive) {
  this->active = active;
  this->passive = passive;
  this->count = 0;
}

void LEDService::periodic() {
  switch (state) {
  case LED_STATE_ALARM:
  case LED_STATE_ON:
    digitalWrite(led, HIGH);
    break;

  case LED_STATE_BLINK:
    periodicBlink();
    break;

  case LED_STATE_OFF:
    digitalWrite(led, LOW);
    break;
  }
}

void LEDService::periodicBlink() {
  count++;

  // The LED is on between 0 and active
  if (count == active)
    digitalWrite(led, LOW);

  // The LED is off between active and active+passive
  if (count == active + passive) {
      count = 0;
    digitalWrite(led, HIGH);
  }
}

// Example of a static function to handle UPnP requests : only access to global variables here.
static void GetVersion() {
  char msg[128];
  sprintf(msg, versionTemplate, versionFileInfo, versionDateInfo, versionTimeInfo);
  HTTP.send(200, UPnPClass::mimeTypeXML, msg);
}

// Example of a member function to handle UPnP requests : this can access stuff in the class
void LEDService::GetStateHandler() {
  int l2 = strlen(gsh_template) + strlen(myServiceType) + MSS_STATE_LENGTH,
      l1 = strlen(UPnPClass::envelopeHeader) + l2 + strlen(UPnPClass::envelopeTrailer) + 5;
  char *tmp2 = (char *)malloc(l2),
       *tmp1 = (char *)malloc(l1);
#ifdef DEBUG
  DEBUG.println("LEDService::GetStateHandler");
#endif
  strcpy(tmp1, UPnPClass::envelopeHeader);
  sprintf(tmp2, gsh_template, myServiceType, LEDService::state);
  strcat(tmp1, tmp2);
  free(tmp2);
  strcat(tmp1, UPnPClass::envelopeTrailer);
  HTTP.send(200, UPnPClass::mimeTypeXML, tmp1);
  free(tmp1);
}

// Example of a member function to handle UPnP requests : this can access stuff in the class
void LEDService::SetStateHandler() {
  int l2 = strlen(gsh_template) + strlen(myServiceType) + MSS_STATE_LENGTH,
      l1 = strlen(UPnPClass::envelopeHeader) + l2 + strlen(UPnPClass::envelopeTrailer) + 5;
  char *tmp2 = (char *)malloc(l2),
       *tmp1 = (char *)malloc(l1);
#ifdef DEBUG
  DEBUG.println("LEDService::SetStateHandler");
#endif
  strcpy(tmp1, UPnPClass::envelopeHeader);
  sprintf(tmp2, gsh_template, myServiceType, LEDService::state);
  strcat(tmp1, tmp2);
  free(tmp2);
  strcat(tmp1, UPnPClass::envelopeTrailer);
  HTTP.send(200, UPnPClass::mimeTypeXML, tmp1);
  free(tmp1);
}
