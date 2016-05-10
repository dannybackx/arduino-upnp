/*
 * This is a sample of a UPnP service that runs on a IoT device.
 * 
 * UPnP commands/queries can be used from an application or a script.
 * This service is an Alarm output.
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
#ifndef _INCLUDE_ALARM_SERVICE_H_
#define _INCLUDE_ALARM_SERVICE_H_

#include "UPnP.h"
#include "UPnP/UPnPService.h"
#include <UPnP/WebServer.h>

#define ALARM_DEFAULT_PIN	0

// FIXME
#define MSS_STATE_LENGTH	16

enum AlarmState {
    // Detect that begin() wasn't called yet
    ALARM_STATE_INVALID,

    ALARM_STATE_OFF,
    ALARM_STATE_ALARM,
    ALARM_STATE_ON,

    // Add stuff before, not after this
    ALARM_STATE_END
};

class AlarmService : public UPnPService {
  public:
    AlarmService();
    AlarmService(const char *deviceURN);
    AlarmService(const char *serviceType, const char *serviceId);
    ~AlarmService();

    void begin();
    enum AlarmState GetState();
    void SetState(enum AlarmState);
    void GetStateHandler();
    void SetStateHandler();

    void setPeriod(int active, int passive);
    void periodic();

    void SendMailSample(int port);
    
  private:
    Configuration *config;

    //
    int alarmpin;
    enum AlarmState state;
    int count, passive, active;		// FIXME from LED

    // Mail parameters
    int mail;				// Whether to send e-mail on alarm
    char *from, *to;

    // Alarm controller
    char *code;				// Unlock code
};

#define ALARM_GLOBAL

#ifdef ALARM_GLOBAL
extern AlarmService ALARM;
#endif

#endif /* _INCLUDE_ALARM_SERVICE_H_ */
