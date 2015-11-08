#ifndef	__UPnPDevice_H_
#define	__UPnPDevice_H_

#define LWIP_OPEN_SRC
#include <functional>
//#include "UPnP/SSDP.h"
//#include "WiFiUdp.h"
#include "debug.h"

extern "C" {
  #include "osapi.h"
  #include "ets_sys.h"
//  #include "user_interface.h"
}

// #include "lwip/opt.h"
// #include "lwip/udp.h"
// #include "lwip/inet.h"
// #include "lwip/igmp.h"
// #include "lwip/mem.h"
// #include "include/UdpContext.h"

#define UPnP_UUID_SIZE              37
#define UPnP_SCHEMA_URL_SIZE        64
#define UPnP_FRIENDLY_NAME_SIZE     64
#define UPnP_SERIAL_NUMBER_SIZE     32
#define UPnP_PRESENTATION_URL_SIZE  128
#define UPnP_MODEL_NAME_SIZE        64
#define UPnP_MODEL_URL_SIZE         128
#define UPnP_MODEL_VERSION_SIZE     32
#define UPnP_MANUFACTURER_SIZE      64
#define UPnP_MANUFACTURER_URL_SIZE  128

class UPnPDevice {
  public:
    UPnPDevice();
    ~UPnPDevice();
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

    char _uuid[UPnP_UUID_SIZE];
    char _modelName[UPnP_MODEL_NAME_SIZE];
    char _modelNumber[UPnP_MODEL_VERSION_SIZE];

  private:

  protected:
    uint16_t _port;
    char _schemaURL[UPnP_SCHEMA_URL_SIZE];
    char _friendlyName[UPnP_FRIENDLY_NAME_SIZE];
    char _serialNumber[UPnP_SERIAL_NUMBER_SIZE];
    char _presentationURL[UPnP_PRESENTATION_URL_SIZE];
    char _manufacturer[UPnP_MANUFACTURER_SIZE];
    char _manufacturerURL[UPnP_MANUFACTURER_URL_SIZE];
    char _modelURL[UPnP_MODEL_URL_SIZE];
};

#endif
