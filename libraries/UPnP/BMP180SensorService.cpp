/*
 * This is a sample of a UPnP service that runs on a IoT device.
 * 
 * UPnP commands/queries can be used from an application or a script.
 * This service represents the BMP180 barometric pressure and temperature sensor.
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
#include "SFE_BMP180.h"
#include "UPnP/BMP180SensorService.h"
#include "UPnP/WebServer.h"

extern WebServer HTTP;
static void GetVersion();

#define VERBOSE	Serial
#undef DEBUG
// #define DEBUG Serial

// Printf style template, parameters : serviceType, state
static const char *gsh_template = "<u:GetStateResponse xmlns=\"%s\">\r\n<Temperature>%s</Temperature>\r\n<Pressure>%s</Pressure>\r\n</u:GetStateResponse>\r\n";
static const char *empty_template = "<u:GetStateResponse xmlns=\"%s\">\r\n</u:GetStateResponse>\r\n";

static const char *getStateXML = "<action>"
  "<name>getState</name>"
  "<argumentList>"
  "<argument>"
  "<retval/>"
  "<name>Temperature</name>"
  "<relatedStateVariable>Temperature</relatedStateVariable>"
  "<direction>out</direction>"
  "</argument>"
  "<argument>"
  "<retval/>"
  "<name>Pressure</name>"
  "<relatedStateVariable>Pressure</relatedStateVariable>"
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

static const char *myServiceName = "PressureSensor";
static const char *myServiceType = "urn:danny-backx-info:service:bmp180sensor:1";
static const char *myServiceId = "urn:danny-backx-info:serviceId:bmp180sensor1";
static const char *temperatureString = "Temperature";
static const char *pressureString = "Pressure";
static const char *percentageString = "Percentage";
static const char *getStateString = "getState";
static const char *getVersionString = "getVersion";
static const char *stringString = "string";
static const int defaultPercentage = 3;		// Percentage, see poll().

BMP180SensorService::BMP180SensorService() :
  UPnPService(myServiceName, myServiceType, myServiceId)
{
  addAction(getStateString, static_cast<MemberActionFunction>(&BMP180SensorService::GetPressureHandler), getStateXML);
  addAction(getVersionString, GetVersion, getVersionXML);
  addStateVariable(temperatureString, stringString, true);
  addStateVariable(pressureString, stringString, true);
  addStateVariable(percentageString, stringString, false);
  inited = false;
  oldTemperature = 0;
  oldPressure = 0;
  count = 0;
}

BMP180SensorService::BMP180SensorService(const char *deviceURN) :
  UPnPService(myServiceName, myServiceType, myServiceId)
{
  addAction(getStateString, static_cast<MemberActionFunction>(&BMP180SensorService::GetPressureHandler), getStateXML);
  addAction(getVersionString, GetVersion, getVersionXML);
  addStateVariable(temperatureString, stringString, true);
  addStateVariable(pressureString, stringString, true);
  addStateVariable(percentageString, stringString, false);
  inited = false;
  oldTemperature = 0;
  oldPressure = 0;
  count = 0;
}

BMP180SensorService::BMP180SensorService(const char *serviceType, const char *serviceId) :
  UPnPService(myServiceName, serviceType, serviceId)
{
  addAction(getStateString, static_cast<MemberActionFunction>(&BMP180SensorService::GetPressureHandler), getStateXML);
  addAction(getVersionString, GetVersion, getVersionXML);
  addStateVariable(temperatureString, stringString, true);
  addStateVariable(pressureString, stringString, true);
  addStateVariable(percentageString, stringString, false);
  inited = false;
  oldTemperature = 0;
  oldPressure = 0;
  count = 0;
}

BMP180SensorService::~BMP180SensorService() {
#ifdef DEBUG
  DEBUG.println("BMP180SensorService DTOR");
#endif  
}

void BMP180SensorService::begin() {
  temperature[0] = 0;
  pressure[0] = 0;

  config = new Configuration("BMP180",
    new ConfigurationItem("name", ""),
    new ConfigurationItem(percentageString, defaultPercentage),
    NULL);
  UPnPService::begin(config);
  percentage = config->GetValue(percentageString);
  name = config->GetStringValue("name");

#ifdef DEBUG
  DEBUG.printf("BMP180SensorService::begin (%d %%)\n", percentage);
#endif

  bmp = new SFE_BMP180();
  char ok = bmp->begin();	// 0 return value is a problem
  if (ok == 0) {
    delete bmp;
    bmp = 0;
#ifdef DEBUG
    DEBUG.println("No BMP180 detected");
#endif
  }
}

// Return true if a noticable difference
bool BMP180SensorService::Difference(float oldval, float newval) {
  float cmp1, cmp2;
  if (isnan(oldval) || isnan(newval) || isinf(oldval) || isinf(newval))
    return true;
  if (oldval == 0 && newval == 0)
    return false;
  if (oldval == 0)
    cmp1 = newval;
  else
    cmp1 = oldval;
  cmp2 = ((oldval - newval) / cmp1) * 100;
#ifdef DEBUGx
  if (count < 3) {
    DEBUG.print("Difference ");
    DEBUG.print(cmp2);
    DEBUG.print(" = ");
    DEBUG.print(oldval);
    DEBUG.print(" - ");
    DEBUG.print(newval);
    DEBUG.print(" / ");
    DEBUG.print(cmp1);
    DEBUG.print(" * 100");
  }
#endif

  if (cmp2 < 0)
    cmp2 = -cmp2;
  if (percentage < cmp2) {
#ifdef DEBUGx
  if (count < 3)
    DEBUG.println(" --> true");
#endif
    return true;
  }
#ifdef DEBUGx
  if (count < 3)
  DEBUG.println(" --> false");
#endif
  return false;
}

/*
 * Query the sensor. This is an I2C device, but all that complexity is in a separate class.
 *
 * This method makes sure we report changes only if they exceed a settable percentage.
 * (Working with float readings requires something like this.)
 */
void BMP180SensorService::poll() {
  bool diff = false, nan = false;
  
  if (bmp == 0)
    return;
  count++;

  oldTemperature = newTemperature;
  oldPressure = newPressure;

  // This is a multi-part query to the I2C device, see the SFE_BMP180 source files.
  char d1 = bmp->startTemperature();
  delay(d1);
  char d2 = bmp->getTemperature(newTemperature);
  if (d2 == 0) { // Error communicating with device
#ifdef DEBUG
    DEBUG.printf("BMP180 : communication error (temperature)\n");
#endif
    return;
  }

  char d3 = bmp->startPressure(0);
  delay(d3);
  char d4 = bmp->getPressure(newPressure, newTemperature);
  if (d4 == 0) { // Error communicating with device
#ifdef DEBUG
    DEBUG.printf("BMP180 : communication error (pressure)\n");
#endif
    return;
  }

  if (isnan(newTemperature) || isinf(newTemperature)) {
    newTemperature = oldTemperature;
    nan = true;
  }
  if (isnan(newPressure) || isinf(newPressure)) {
    newPressure = oldPressure;
    nan = true;
  }
  if (nan) {
#ifdef DEBUG
    DEBUG.println("BMP180 nan");
#endif
    return;
  }

  // Only do this if we really have non-nan values
  inited = true;
  UpdateTemperature();
  UpdatePressure();

  if (Difference(oldTemperature, newTemperature)) {
    UpdateTemperature();
    SendNotify(temperatureString);
    diff = true;
  }
  if (Difference(oldPressure, newPressure)) {
    UpdatePressure();
    SendNotify(pressureString);
    diff = true;
  }

#ifdef VERBOSE
  if (diff) {
    VERBOSE.print("BMP180 : temperature ");
    VERBOSE.print(newTemperature);
    VERBOSE.print(" pressure ");
    VERBOSE.println(newPressure);
  }
#elif defined(DEBUG)
  if (diff) {
    DEBUG.print("BMP180 : Temp ");
    DEBUG.print(newTemperature);
    DEBUG.print(" Pressure ");
    DEBUG.println(newPressure);
  }
#endif
}

const float BMP180SensorService::GetFloatTemperature() {
  if (! inited)
    poll();	// try once

  return newTemperature;
}

const char *BMP180SensorService::GetTemperature() {
  if (! inited)
    poll();	// try once

  return temperature;
}

const float BMP180SensorService::GetFloatPressure() {
  if (! inited)
    poll();	// try once

  return newPressure;
}

const char *BMP180SensorService::GetPressure() {
  if (! inited)
    poll();	// try once

  return pressure;
}

// Example of a static function to handle UPnP requests : only access to global variables here.
static void GetVersion() {
  char msg[128];
  sprintf(msg, versionTemplate, versionFileInfo, versionDateInfo, versionTimeInfo);
  HTTP.send(200, UPnPClass::mimeTypeXML, msg);
}

// Example of a member function to handle UPnP requests : this can access stuff in the class
void BMP180SensorService::GetPressureHandler() {
  if (bmp == 0) {
    int l = strlen(empty_template) + strlen(myServiceType);
    char *tmp = (char *)malloc(l);
    sprintf(tmp, empty_template, myServiceType);
    HTTP.send(200, UPnPClass::mimeTypeXML, tmp);
    free(tmp);
    return;
  }

  // Calculate buffer lengths
  int l2 = strlen(gsh_template) + strlen(myServiceType) + BMP180_STATE_LENGTH * 2,
      l1 = strlen(UPnPClass::envelopeHeader) + l2 + strlen(UPnPClass::envelopeTrailer) + 5;

  // Allocate buffers accordingly
  char *tmp2 = (char *)malloc(l2),
       *tmp1 = (char *)malloc(l1);

#ifdef DEBUG
  DEBUG.println("BMP180SensorService::GetPressureHandler");
#endif

  // String manipulation, prepare our answer
  strcpy(tmp1, UPnPClass::envelopeHeader);
  //UpdateTemperature();
  //UpdatePressure();
  sprintf(tmp2, gsh_template, myServiceType, temperature, pressure);
  strcat(tmp1, tmp2);
  free(tmp2);
  strcat(tmp1, UPnPClass::envelopeTrailer);

  // Send answer
  HTTP.send(200, UPnPClass::mimeTypeXML, tmp1);
  free(tmp1);
}

void BMP180SensorService::FloatToString(float f, char *s) {
  // sprintf(s, "%1.2f", f);
  int a = (int)f,
      b = (int)(100 * (f-(float)a));
  sprintf(s, "%d.%02d", a, b);
}

void BMP180SensorService::UpdateTemperature() {
  if (bmp)
    FloatToString(newTemperature, temperature);
}

void BMP180SensorService::UpdatePressure() {
  if (bmp)
    FloatToString(newPressure, pressure);
}

bool BMP180SensorService::Works() {
  return inited;
}
