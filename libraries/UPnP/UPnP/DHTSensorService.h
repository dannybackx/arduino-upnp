/*
 * This is a sample of a UPnP service that runs on a IoT device.
 * 
 * UPnP commands/queries can be used from an application or a script.
 * This service represents the DHT-11 temperature and humidity sensor.
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
#ifndef _INCLUDE_DHT_SENSOR_SERVICE_H_
#define _INCLUDE_DHT_SENSOR_SERVICE_H_

#include "UPnP.h"
#include "UPnP/UPnPService.h"
#include <UPnP/WebServer.h>
#include "dht.h"

#define DHT_STATE_LENGTH	16
#define	DHT_SENSOR_PIN_DEFAULT	12
#define DHT_SENSOR_TYPE_DEFAULT	11

class DHTSensorService : public UPnPService {
  public:
    DHTSensorService();
    DHTSensorService(const char *deviceURN);
    DHTSensorService(const char *serviceType, const char *serviceId);
    ~DHTSensorService();
    void begin();
    const char *GetState();
    void GetStateHandler();

    void poll();            // periodically poll the sensor
    
  private:
    Configuration *config;
    dht *sensor;

    int sensorpin, sensortype;
    char state[DHT_STATE_LENGTH];
    WebServer *http;
    float oldtemperature, newtemperature;
    float newhumidity, oldhumidity;
};

#endif /* _INCLUDE_DHT_SENSOR_SERVICE_H_ */
