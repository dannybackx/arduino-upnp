/*
 * This is a sample of a UPnP service that runs on a IoT device.
 * 
 * UPnP commands/queries can be used from an application or a script.
 * This service represents the BMP180 barometric pressure and temperature sensor.
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
#include "SFE_BMP180.h"
#include "UPnP/BMP180SensorService.h"
#include "UPnP/WebServer.h"

extern WebServer HTTP;
static void GetVersion();

#undef DEBUG
// #define DEBUG Serial

// Printf style template, parameters : serviceType, state
static const char *gsh_template = "<u:GetStateResponse xmlns=\"%s\">\r\n<Temperature>%s</Temperature>\r\n<Pressure>%s</Pressure>\r\n</u:GetStateResponse>\r\n";

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
}

BMP180SensorService::BMP180SensorService(const char *deviceURN) :
  UPnPService(myServiceName, myServiceType, myServiceId)
{
  addAction(getStateString, static_cast<MemberActionFunction>(&BMP180SensorService::GetPressureHandler), getStateXML);
  addAction(getVersionString, GetVersion, getVersionXML);
  addStateVariable(temperatureString, stringString, true);
  addStateVariable(pressureString, stringString, true);
  addStateVariable(percentageString, stringString, false);
}

BMP180SensorService::BMP180SensorService(const char *serviceType, const char *serviceId) :
  UPnPService(myServiceName, serviceType, serviceId)
{
  addAction(getStateString, static_cast<MemberActionFunction>(&BMP180SensorService::GetPressureHandler), getStateXML);
  addAction(getVersionString, GetVersion, getVersionXML);
  addStateVariable(temperatureString, stringString, true);
  addStateVariable(pressureString, stringString, true);
  addStateVariable(percentageString, stringString, false);
}

BMP180SensorService::~BMP180SensorService() {
#ifdef DEBUG
  DEBUG.println("BMP180SensorService DTOR");
#endif  
}

void BMP180SensorService::begin() {
#ifdef DEBUG
  DEBUG.println("BMP180SensorService::begin");
#endif
  temperature[0] = 0;
  pressure[0] = 0;

  config = new Configuration("BMP180",
    new ConfigurationItem("name", ""),
    new ConfigurationItem(percentageString, defaultPercentage),
    NULL);
  UPnPService::begin(config);
  percentage = config->GetValue(percentageString);

  bmp = new SFE_BMP180();
  bmp->begin();
}

// Return true if a noticable difference
bool BMP180SensorService::Difference(float oldval, float newval) {
  float cmp1, cmp2;
  if (oldval == 0 && newval == 0)
    return false;
  if (oldval == 0)
    cmp1 = newval;
  else
    cmp1 = oldval;
  cmp2 = ((oldval - newval) / cmp1) * 100;
#ifdef DEBUGx
  DEBUG.print("Difference ");
  DEBUG.print(cmp2);
  DEBUG.print(" = ");
  DEBUG.print(oldval);
  DEBUG.print(" - ");
  DEBUG.print(newval);
  DEBUG.print(" / ");
  DEBUG.print(cmp1);
  DEBUG.print(" * 100");
#endif

  if (cmp2 < 0)
    cmp2 = -cmp2;
  if (percentage < cmp2) {
#ifdef DEBUGx
    DEBUG.println(" --> true");
#endif
    return true;
  }
#ifdef DEBUGx
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
  oldTemperature = newTemperature;
  oldPressure = newPressure;

  // This is a multi-part query to the I2C device, see the SFE_BMP180 source files.
  char d = bmp->startTemperature();
  delay(d);
  d = bmp->getTemperature(newTemperature);
  d = bmp->startPressure(0);
  delay(d);
  d = bmp->getPressure(newPressure, newTemperature);

  if (isnan(newTemperature) || isnan(newPressure)) {
    newTemperature = oldTemperature;
    newPressure = oldPressure;
    return;
  }

  if (Difference(oldTemperature, newTemperature)) {
    UpdateTemperature();
    SendNotify(temperatureString);
  }
  if (Difference(oldPressure, newPressure)) {
    UpdatePressure();
    SendNotify(pressureString);

#ifdef DEBUG
  DEBUG.print("BMP180 : Temp ");
  DEBUG.print(newTemperature);
  DEBUG.print(" Pressure ");
  DEBUG.println(newPressure);
#endif
  }
}

const char *BMP180SensorService::GetTemperature() {
  return temperature;
}

const char *BMP180SensorService::GetPressure() {
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
  int l2 = strlen(gsh_template) + strlen(myServiceType) + BMP180_STATE_LENGTH * 2,
      l1 = strlen(UPnPClass::envelopeHeader) + l2 + strlen(UPnPClass::envelopeTrailer) + 5;
  char *tmp2 = (char *)malloc(l2),
       *tmp1 = (char *)malloc(l1);
#ifdef DEBUG
  DEBUG.println("BMP180SensorService::GetPressureHandler");
  delay(1000);
#endif
  strcpy(tmp1, UPnPClass::envelopeHeader);
  sprintf(tmp2, gsh_template, myServiceType,
    BMP180SensorService::temperature, BMP180SensorService::pressure);
  strcat(tmp1, tmp2);
  free(tmp2);
  strcat(tmp1, UPnPClass::envelopeTrailer);
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
  FloatToString(newTemperature, temperature);
}

void BMP180SensorService::UpdatePressure() {
  FloatToString(newPressure, pressure);
}
