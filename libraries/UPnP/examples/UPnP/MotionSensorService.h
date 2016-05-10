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
#ifndef _INCLUDE_MOTION_SENSOR_SERVICE_H_
#define _INCLUDE_MOTION_SENSOR_SERVICE_H_

#include "UPnP.h"
#include "UPnP/UPnPService.h"
#include <UPnP/WebServer.h>

#define MSS_STATE_LENGTH  16

//class MotionSensorService;
//typedef void (MotionSensorService::*MActionFunction)();

class MotionSensorService : public UPnPService {
  public:
    MotionSensorService();
    MotionSensorService(const char *deviceURN);
    MotionSensorService(const char *serviceType, const char *serviceId);
    ~MotionSensorService();
    void begin();
    const char *GetState();
    void GetStateHandler();

    void poll();            // periodically poll the sensor
    
  private:
    Configuration *config;

    int sensorpin;
    
    char state[MSS_STATE_LENGTH];
    WebServer *http;
    int oldstate, newstate;
};

#undef MSS_GLOBAL

#ifdef MSS_GLOBAL
extern MotionSensorService MSS;
#endif

#endif /* _INCLUDE_MOTION_SENSOR_SERVICE_H_ */
