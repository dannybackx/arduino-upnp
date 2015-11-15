/*
 * ESP8266 Simple Service Discovery
 *
 * Copyright (c) 2015 Hristo Gochkov
 * Copyright (c) 2015 Danny Backx
 * 
 * Original (Arduino) version by Filippo Sallemi, July 23, 2014.
 * Can be found at: https://github.com/nomadnt/uSSDP
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
 * 
 */

#ifndef _UPNP_SSDP_H_
#define _UPNP_SSDP_H_

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

class UdpContext;

typedef enum {
  NONE,
  SEARCH,
  NOTIFY
} ssdp_method_t;

struct SSDPTimer;

class SSDPClass {
  public:
    SSDPClass();
    ~SSDPClass();

    bool begin(UPnPDevice &device);

  protected:
    void _send(ssdp_method_t method);
    void _update();
    void _startTimer();
    static void _onTimerStatic(SSDPClass* self);

    UdpContext* _server;
    SSDPTimer* _timer;

    IPAddress _respondToAddr;
    uint16_t  _respondToPort;

    bool _pending;
    unsigned short _delay;
    unsigned long _process_time;
    unsigned long _notify_time;
    
    uint16_t _port;

  private:
    UPnPDevice device;
};

extern SSDPClass SSDP;

#endif
