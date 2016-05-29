/*
 * This is a sample of a UPnP service that runs on a IoT device.
 * 
 * UPnP commands/queries can be used from an application or a script.
 * This service discovers sensors.
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
#ifndef _INCLUDE_DISCOVER_SERVICE_H_
#define _INCLUDE_DISCOVER_SERVICE_H_

#include "UPnP.h"
#include "UPnP/UPnPService.h"
#include <UPnP/WebServer.h>

#define DISCOVER_PIN_DEFAULT		0
#define	DISCOVER_PASSIVE_DEFAULT	190
#define	DISCOVER_ACTIVE_DEFAULT		10

#define	DISCOVER_DEVICES_INCREMENT	8

struct DiscoveredDevice {
  IPAddress	ip;
  uint16_t	port;
  char		*usn;
  char		*location;
  char		*upnptype;
  char		*friendlyname;
};

class DiscoveryManager : public UPnPService {
  public:
    DiscoveryManager();
    ~DiscoveryManager();
    void begin();

    void AddConfiguredServers();
    bool QuerySensors();
    bool QuerySensors(const char *);

    /*
    DiscoveryManager(const char *deviceURN);
    DiscoveryManager(const char *serviceType, const char *serviceId);
    enum DISCOVERState GetState();
    void SetState(enum DISCOVERState);
    void GetStateHandler();
    void SetStateHandler();

    void setPeriod(int active, int passive);
    /* */
    void periodic();
    
  private:
    Configuration *config;
    UdpContext* udpctx;
    void receivePacket();
    char *packet, *buffer;
    WiFiUDP udp;

    void ProcessPacket(char *packet);

    void AddDevice();
    void RemoveDevice();
  public:
    struct DiscoveredDevice *devices;
    int ndevices, maxdevices;
};

#define DISCOVER_GLOBAL

#ifdef DISCOVER_GLOBAL
extern DiscoveryManager DISCOVER;
#endif

#endif /* _INCLUDE_DISCOVER_SERVICE_H_ */
