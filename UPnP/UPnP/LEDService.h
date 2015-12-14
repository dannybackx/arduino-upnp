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
#ifndef _INCLUDE_LED_SERVICE_H_
#define _INCLUDE_LED_SERVICE_H_

#include "UPnP.h"
#include "UPnP/UPnPService.h"
#include <UPnP/WebServer.h>

#define MSS_STATE_LENGTH  16

enum LEDState {
    // Detect that begin() wasn't called yet
    LED_STATE_INVALID,

    LED_STATE_OFF,
    LED_STATE_BLINK,
    LED_STATE_ALARM,
    LED_STATE_ON,

    // Add stuff before, not after this
    LED_STATE_END
};

class LEDService : public UPnPService {
  public:
    LEDService();
    LEDService(const char *deviceURN);
    LEDService(const char *serviceType, const char *serviceId);
    ~LEDService();
    void begin();
    enum LEDState GetState();
    void SetState(enum LEDState);
    void GetStateHandler();
    void SetStateHandler();

    void setPeriod(int active, int passive);
    void periodic();
    
  private:
    enum LEDState state;
    int count, passive, active;
    void periodicBlink();

};

#define LED_GLOBAL

#ifdef LED_GLOBAL
extern LEDService LED;
#endif

#endif /* _INCLUDE_LED_SERVICE_H_ */
