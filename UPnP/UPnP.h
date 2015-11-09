#ifndef	__UPnP_H_
#define	__UPnP_H_

// #define LWIP_OPEN_SRC
// #include <functional>
#include "UPnP/UPnPDevice.h"
#include "UPnP/SSDP.h"
#include "WiFiUdp.h"
// #include "debug.h"
// 
// extern "C" {
//   #include "osapi.h"
//   #include "ets_sys.h"
//   #include "user_interface.h"
// }
// 
// #include "lwip/opt.h"
// #include "lwip/udp.h"
// #include "lwip/inet.h"
// #include "lwip/igmp.h"
// #include "lwip/mem.h"
// #include "include/UdpContext.h"

class UPnPClass {
  public:
    UPnPClass();
    ~UPnPClass();
    void begin(UPnPDevice &device);
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

  private:
    UPnPDevice d;

  protected:
};

extern UPnPClass UPnP;

/* */

#endif
