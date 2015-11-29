/*
 * ESP8266 Simple UPnP Framework
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
#ifndef	__UPnP_H_
#define	__UPnP_H_

#include "UPnP/UPnPDevice.h"
#include "UPnP/SSDP.h"
#include "WiFiUdp.h"
#include "UPnP/UPnPService.h"
#include "UPnP/WebServer.h"

#define	N_SERVICES	4

class UPnPClass {
  public:
    UPnPClass();
    ~UPnPClass();
    //void begin(UPnPDevice &device);
    void begin(WebServer *http, UPnPDevice *device);

    void setSchemaURL(const char *url);
    void setHTTPPort(uint16_t port);
    void setName(const char *name);
    void setURL(const char *url);
    void setSerialNumber(const char *serialNumber);
    void setModelName(const char *name);
    void setModelNumber(const char *num);
    void setModelURL(const char *url);
    void setManufacturer(const char *name);
    void setManufacturerURL(const char *url);

    void schema(WiFiClient client);
    void SCPD(WiFiClient client);

    void addService(UPnPService *service);

    static const char *mimeType;
    static const char *envelopeHeader;
    static const char *envelopeTrailer;

  private:
    UPnPDevice *device;
    WebServer *http;

  protected:
    UPnPService *services;
};

extern UPnPClass UPnP;	// FIXME

#endif
