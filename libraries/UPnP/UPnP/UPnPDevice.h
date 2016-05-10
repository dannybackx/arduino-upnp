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
#ifndef	__UPnPDevice_H_
#define	__UPnPDevice_H_

#define LWIP_OPEN_SRC
#include <functional>
#include "debug.h"

#define UPnP_UUID_SIZE              37
// #define UPnP_SCHEMA_URL_SIZE        64
#define UPnP_FRIENDLY_NAME_SIZE     64
#define UPnP_SERIAL_NUMBER_SIZE     32
// #define UPnP_PRESENTATION_URL_SIZE  128
#define UPnP_MODEL_NAME_SIZE        64
#define UPnP_MODEL_URL_SIZE         128
#define UPnP_MODEL_VERSION_SIZE     32
#define UPnP_MANUFACTURER_SIZE      64
// #define UPnP_MANUFACTURER_URL_SIZE  128

class UPnPDevice {
  public:
    UPnPDevice();
    ~UPnPDevice();
    void setSchemaURL(char *url);
    void setHTTPPort(uint16_t port);
    void setName(char *name);
    void setURL(char *url);
    void setSerialNumber(char *serialNumber);
    void setModelName(char *name);
    void setModelNumber(char *num);
    void setModelURL(char *url);
    void setManufacturer(char *name);
    void setManufacturerURL(char *url);
    void setDeviceURN(char *urn);

    /*
    void setName(const String& name) { setName(name.c_str()); }
    void setURL(const String& url) { setURL(url.c_str()); }
    void setSchemaURL(const String& url) { setSchemaURL(url.c_str()); }
    void setSerialNumber(const String& serialNumber) { setSerialNumber(serialNumber.c_str()); }
    void setModelName(const String& name) { setModelName(name.c_str()); }
    void setModelNumber(const String& num) { setModelNumber(num.c_str()); }
    void setModelURL(const String& url) { setModelURL(url.c_str()); }
    void setManufacturer(const String& name) { setManufacturer(name.c_str()); }
    void setManufacturerURL(const String& url) { setManufacturerURL(url.c_str()); }
    /* */

    void setPort(uint16_t port);
    uint16_t getPort();

    void setFriendlyName(const char *fn);
    char *getFriendlyName();

    char *getSchemaURL();
    char *getPresentationURL();
    char *getSerialNumber();
    char *getModelName();
    char *getModelNumber();
    char *getModelURL();
    char *getManufacturer();
    char *getManufacturerURL();
    char *getUuid();
    char *getDeviceURN();

    //char _uuid[UPnP_UUID_SIZE];
    //char _modelName[UPnP_MODEL_NAME_SIZE];
    //char _modelNumber[UPnP_MODEL_VERSION_SIZE];
    char *_uuid;
    char *_modelName;
    char *_modelNumber;

  private:

  protected:
    uint16_t _port;
    char *_schemaURL;
    char *_friendlyName;
    char *_serialNumber;
    char *_presentationURL;
    char *_manufacturer;
    char *_manufacturerURL;
    char *_modelURL;
    char *_deviceURN;

    // char _schemaURL[UPnP_SCHEMA_URL_SIZE];
    // char _friendlyName[UPnP_FRIENDLY_NAME_SIZE];
    // char _serialNumber[UPnP_SERIAL_NUMBER_SIZE];
    // char _presentationURL[UPnP_PRESENTATION_URL_SIZE];
    // char _manufacturer[UPnP_MANUFACTURER_SIZE];
    // char _manufacturerURL[UPnP_MANUFACTURER_URL_SIZE];
    // char _modelURL[UPnP_MODEL_URL_SIZE];
    // char _deviceURN[UPnP_MODEL_URL_SIZE];
};

#endif
