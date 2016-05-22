/*
 * This is a sample of a UPnP service that runs on a IoT device.
 * 
 * UPnP commands/queries can be used from an application or a script.
 * This service represents the BMP-180 Barometic pressure and temperature sensor.
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
#ifndef _INCLUDE_BMP180_SENSOR_SERVICE_H_
#define _INCLUDE_BMP180_SENSOR_SERVICE_H_

#include "UPnP.h"
#include "UPnP/UPnPService.h"
#include <UPnP/WebServer.h>

#define BMP180_STATE_LENGTH	16

class BMP180SensorService : public UPnPService {
  public:
    BMP180SensorService();
    BMP180SensorService(const char *deviceURN);
    BMP180SensorService(const char *serviceType, const char *serviceId);
    ~BMP180SensorService();
    void begin();
    const char *GetTemperature(), *GetPressure();
    const float GetFloatTemperature(), GetFloatPressure();
    void GetPressureHandler();

    void poll();            // periodically poll the sensor
    
  private:
    Configuration *config;
    bool inited;
    int count;

    char temperature[BMP180_STATE_LENGTH], pressure[BMP180_STATE_LENGTH];
    WebServer *http;
    double newPressure, newTemperature;
    double oldPressure, oldTemperature;
    int percentage;	// How much of a difference before notify
    SFE_BMP180 *bmp;

    void FloatToString(float f, char *s);
    void UpdateTemperature();
    void UpdatePressure();
    bool Difference(float oldval, float newval);
};

#endif /* _INCLUDE_BMP180_SENSOR_SERVICE_H_ */
