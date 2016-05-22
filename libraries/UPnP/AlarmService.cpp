/*
 * This is a sample of a UPnP service that runs on a IoT device.
 * 
 * UPnP commands/queries can be used from an application or a script.
 * This service is an Alarm output (buzzer, lights, ..).
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

#include "UPnP.h"
#include "UPnP/UPnPService.h"
#include "UPnP/AlarmService.h"
#include "UPnP/WebServer.h"

#include <WiFiClientSecure.h>
#include <SmtpClient.h>
#include <Mail.h>

extern WebServer HTTP;
static void GetVersion();

#define DEBUG Serial

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
// UPnP stuff
static const char *myServiceName = "AlarmService";
static const char *myServiceType = "urn:danny-backx-info:service:alarm:1";
static const char *myServiceId = "urn:danny-backx-info:serviceId:alarm1";
// Variables
static const char *codeString = "Code";
static const char *stateString = "State";
static const char *fromString = "From";
static const char *toString = "To";
static const char *mailHostString = "MailHost";
// Actions
static const char *getStateString = "getState";
static const char *setStateString = "setState";
static const char *getVersionString = "getVersion";
// Types
static const char *stringString = "string";

AlarmService::AlarmService() :
  UPnPService(myServiceName, myServiceType, myServiceId)
{
  addAction(getStateString, static_cast<MemberActionFunction>(&AlarmService::GetStateHandler), getStateXML);
  addAction(setStateString, static_cast<MemberActionFunction>(&AlarmService::SetStateHandler), setStateXML);
  addAction(getVersionString, GetVersion, getVersionXML);
  addStateVariable(stateString, stringString, true);
  addStateVariable(codeString, stringString, false);
  addStateVariable(fromString, stringString, false);
  addStateVariable(toString, stringString, false);
  addStateVariable(mailHostString, stringString, false);
  begin();
}

AlarmService::AlarmService(const char *deviceURN) :
  UPnPService(myServiceName, myServiceType, myServiceId)
{
  addAction(getStateString, static_cast<MemberActionFunction>(&AlarmService::GetStateHandler), getStateXML);
  addAction(setStateString, static_cast<MemberActionFunction>(&AlarmService::SetStateHandler), setStateXML);
  addAction(getVersionString, GetVersion, getVersionXML);
  addStateVariable(stateString, stringString, true);
  addStateVariable(codeString, stringString, false);
  addStateVariable(fromString, stringString, false);
  addStateVariable(toString, stringString, false);
  addStateVariable(mailHostString, stringString, false);
  begin();
}

AlarmService::AlarmService(const char *serviceType, const char *serviceId) :
  UPnPService(myServiceName, serviceType, serviceId)
{
  addAction(getStateString, static_cast<MemberActionFunction>(&AlarmService::GetStateHandler), getStateXML);
  addAction(setStateString, static_cast<MemberActionFunction>(&AlarmService::SetStateHandler), setStateXML);
  addAction(getVersionString, GetVersion, getVersionXML);
  addStateVariable(stateString, stringString, true);
  addStateVariable(codeString, stringString, false);
  addStateVariable(fromString, stringString, false);
  addStateVariable(toString, stringString, false);
  addStateVariable(mailHostString, stringString, false);
  begin();
}

AlarmService::~AlarmService() {
#ifdef DEBUG
  DEBUG.println("AlarmService DTOR");
#endif  
}

void AlarmService::begin() {
  if (state != ALARM_STATE_INVALID)
    return;	// Already been here
  state = ALARM_STATE_OFF;

  config = new Configuration("Alarm",
    new ConfigurationItem("code", "1234"),
    // new ConfigurationItem(mail, 1),			// FIXME
    new ConfigurationItem("from", ""),
    new ConfigurationItem("to", ""),
    NULL);
  UPnPService::begin(config);
  alarmpin = config->GetValue("pin");

#ifdef DEBUG
  DEBUG.printf("AlarmService::begin (pin %d)\n", alarmpin);
#endif

  pinMode(alarmpin, OUTPUT);

  state = ALARM_STATE_OFF;

  /*
  if (config->configured("active") && config->configured("passive")) {
    setPeriod(config->GetValue("active"), config->GetValue("passive"));
    SetState(ALARM_STATE_BLINK);
#ifdef DEBUG
    DEBUG.printf("AlarmService blink %d %d\n",
      config->GetValue("active"),
      config->GetValue("passive"));
#endif
  }
  /* */
}

enum AlarmState AlarmService::GetState() {
  return state;
}

void AlarmService::SetState(enum AlarmState state) {
  this->state = state;
}

/*
 * Note this depends on how frequently the caller calls this method.
 */
void AlarmService::setPeriod(int active, int passive) {
  this->active = active;
  this->passive = passive;
  this->count = 0;
}

void AlarmService::periodic() {
  switch (state) {
  case ALARM_STATE_ALARM:
  case ALARM_STATE_ON:
    digitalWrite(alarmpin, HIGH);
    break;

  case ALARM_STATE_OFF:
    digitalWrite(alarmpin, LOW);
    break;
  }
}

// Example of a static function to handle UPnP requests : only access to global variables here.
static void GetVersion() {
  char msg[128];
  sprintf(msg, versionTemplate, versionFileInfo, versionDateInfo, versionTimeInfo);
  HTTP.send(200, UPnPClass::mimeTypeXML, msg);
}

// Example of a member function to handle UPnP requests : this can access stuff in the class
void AlarmService::GetStateHandler() {
  int l2 = strlen(gsh_template) + strlen(myServiceType) + MSS_STATE_LENGTH,
      l1 = strlen(UPnPClass::envelopeHeader) + l2 + strlen(UPnPClass::envelopeTrailer) + 5;
  char *tmp2 = (char *)malloc(l2),
       *tmp1 = (char *)malloc(l1);
#ifdef DEBUG
  DEBUG.println("AlarmService::GetStateHandler");
#endif
  strcpy(tmp1, UPnPClass::envelopeHeader);
  sprintf(tmp2, gsh_template, myServiceType, AlarmService::state);
  strcat(tmp1, tmp2);
  free(tmp2);
  strcat(tmp1, UPnPClass::envelopeTrailer);
  HTTP.send(200, UPnPClass::mimeTypeXML, tmp1);
  free(tmp1);
}

// Example of a member function to handle UPnP requests : this can access stuff in the class
void AlarmService::SetStateHandler() {
  int l2 = strlen(gsh_template) + strlen(myServiceType) + MSS_STATE_LENGTH,
      l1 = strlen(UPnPClass::envelopeHeader) + l2 + strlen(UPnPClass::envelopeTrailer) + 5;
  char *tmp2 = (char *)malloc(l2),
       *tmp1 = (char *)malloc(l1);
#ifdef DEBUG
  DEBUG.println("AlarmService::SetStateHandler");
#endif
  strcpy(tmp1, UPnPClass::envelopeHeader);
  sprintf(tmp2, gsh_template, myServiceType, AlarmService::state);
  strcat(tmp1, tmp2);
  free(tmp2);
  strcat(tmp1, UPnPClass::envelopeTrailer);
  HTTP.send(200, UPnPClass::mimeTypeXML, tmp1);
  free(tmp1);
}

void AlarmService::SendMailSample(int port) {
  SmtpClient *smtp;
  byte mailip[] = {193, 74, 71, 25};	// smtp.scarlet.be
  // byte mailip[] = {192, 168, 1, 176};	// Local test

#if 1
  // Regular unsecure connection
  WiFiClient wc;
  smtp = new SmtpClient(&wc, mailip, 25);
#else
  WiFiClientSecure wc;
  smtp = new SmtpClient(&wc, mailip, 465);
#endif

  // Try sending mail
  Mail mail;
  mail.from("<danny.backx@scarlet.be>");
  mail.to("<danny.backx@scarlet.be>");
  mail.subject("Test from arduino (AlarmService::SendMailSample)");
  mail.body("Yes I can\n");
  int r = smtp->send(&mail);
  Serial.printf("Send mail : %d\n", r);
  if (r == 0)
    Serial.printf("Error text %s, line %d\n", smtp->GetErrorText(), smtp->GetErrorLine());
}
