#ifndef	__UPnP_H_
#define	__UPnP_H_

#define LWIP_OPEN_SRC
#include <functional>
#include "UPnP/SSDP.h"
#include "WiFiUdp.h"
#include "debug.h"

extern "C" {
  #include "osapi.h"
  #include "ets_sys.h"
  #include "user_interface.h"
}

#include "lwip/opt.h"
#include "lwip/udp.h"
#include "lwip/inet.h"
#include "lwip/igmp.h"
#include "lwip/mem.h"
#include "include/UdpContext.h"

class UPnPClass {
  public:
    UPnPClass();
    ~UPnPClass();
    void begin();
    void schema(WiFiClient client);
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

  private:

  protected:
    uint16_t _port;
    char _schemaURL[SSDP_SCHEMA_URL_SIZE];
    char _uuid[SSDP_UUID_SIZE];
    char _friendlyName[SSDP_FRIENDLY_NAME_SIZE];
    char _serialNumber[SSDP_SERIAL_NUMBER_SIZE];
    char _presentationURL[SSDP_PRESENTATION_URL_SIZE];
    char _manufacturer[SSDP_MANUFACTURER_SIZE];
    char _manufacturerURL[SSDP_MANUFACTURER_URL_SIZE];
    char _modelName[SSDP_MODEL_NAME_SIZE];
    char _modelURL[SSDP_MODEL_URL_SIZE];
    char _modelNumber[SSDP_MODEL_VERSION_SIZE];
};

extern UPnPClass UPnP;

/* */

#endif
