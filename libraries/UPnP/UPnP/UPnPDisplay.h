/*
 * This is a sample of a UPnP service that runs on a IoT device.
 * 
 * UPnP commands/queries can be used from an application or a script.
 * This service is an Alarm output.
 *  
 * Copyright (c) 2016 Danny Backx
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
#ifndef _INCLUDE_UPNP_DISPLAY_H
#define _INCLUDE_UPNP_DISPLAY_H

#include <Arduino.h>

#define UPNP_DISPLAY_DEFAULT_PIN	0

enum UPnPDisplayState {
    // Detect that begin() wasn't called yet
    UPNP_DISPLAY_STATE_INVALID,

    UPNP_DISPLAY_STATE_OFF,
    UPNP_DISPLAY_STATE_ALARM,
    UPNP_DISPLAY_STATE_ON,

    // Add stuff before, not after this
    UPNP_DISPLAY_STATE_END
};

class UPnPDisplay {
  public:
    UPnPDisplay();
    UPnPDisplay(const char *deviceURN);
    UPnPDisplay(const char *serviceType, const char *serviceId);
    ~UPnPDisplay();

    void begin();
    void periodic();
    void SetLine(int ln, const char *s);
    
  private:
    Configuration *config;
    char *message;
    enum UPnPDisplayState state;
    char line[9][17];
};

#define UPNP_DISPLAY_GLOBAL

#ifdef UPNP_DISPLAY_GLOBAL
extern UPnPDisplay UPNP_DISPLAY;
#endif

#endif /* _INCLUDE_UPNP_DISPLAY_H_ */
