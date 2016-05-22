/*
 * ESP8266 Simple Service Discovery
 *
 * Copyright (c) 2015 Hristo Gochkov
 * Copyright (c) 2015, 2016 Danny Backx
 * 
 * Original (Arduino) version by Filippo Sallemi, July 23, 2014.
 * Can be found at: https://github.com/nomadnt/uSSDP
 *
 * Copyright (c) 2016 Danny Backx : almost complete rewrite.
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
#include "UPnP/UPnPDevice.h"
#include "UPnP/SSDP.h"
#include "WiFiUdp.h"
#include "debug.h"

#include "lwip/udp.h"
#include "lwip/igmp.h"
#include "include/UdpContext.h"

// #undef DEBUG_SSDP
#define DEBUG_SSDP  Serial

#define SSDP_INTERVAL     1200
#define SSDP_PORT         1900
#define SSDP_METHOD_SIZE  10
#define SSDP_URI_SIZE     2
#define SSDP_BUFFER_SIZE  64
#define SSDP_MULTICAST_TTL 1

static const IPAddress SSDP_MULTICAST_ADDR(239, 255, 255, 250);

static const char *_ssdp_response_template =
  "HTTP/1.1 200 OK\r\n"	
  "EXT:\r\n"
  "ST: upnp:rootdevice\r\n";

static const char *_ssdp_notify_template =
  "NOTIFY * HTTP/1.1\r\n"
  "HOST: 239.255.255.250:1900\r\n"
  "NT: upnp:rootdevice\r\n"
  "NTS: ssdp:alive\r\n";

static const char *_ssdp_packet_template =
  "%s"						// _ssdp_response_template / _ssdp_notify_template 
  "CACHE-CONTROL: max-age=%u\r\n"		// SSDP_INTERVAL
  "SERVER: Arduino/1.0 UPNP/1.1 %s/%s\r\n"	// _modelName, _modelNumber
  "USN: uuid:%s\r\n"				// _uuid
  "LOCATION: http://%u.%u.%u.%u:%u/%s\r\n"	// WiFi.localIP(), _port, _schemaURL
  "\r\n";

struct SSDPTimer {
  ETSTimer timer;
};

SSDPClass::SSDPClass() :
  _server(0),
  _port(80),
  _pending(false),
  _timer(new SSDPTimer)
{
}

SSDPClass::~SSDPClass(){
#ifdef DEBUG_SSDP
  DEBUG_SSDP.printf("SSDP DTOR\n");
#endif

  delete _timer;
}

bool SSDPClass::begin(UPnPDevice &dev){
  _pending = false;
  device = dev;
  
  uint32_t chipId = ESP.getChipId();
  //                      8 2 6 6  E X  D
  sprintf(device._uuid, "38323636-4558-4dda-9188-cda0e6%02x%02x%02x",
    (uint16_t) ((chipId >> 16) & 0xff),
    (uint16_t) ((chipId >>  8) & 0xff), 
    (uint16_t)   chipId        & 0xff  );

#ifdef DEBUG_SSDP
  DEBUG_SSDP.printf("SSDP UUID: %s\n", (char *)device._uuid);
#endif

  if (_server) {
    _server->unref();
    _server = 0;
  }

  _server = new UdpContext;
  _server->ref();

  ip_addr_t ifaddr;
  ifaddr.addr = WiFi.localIP();
  ip_addr_t multicast_addr;
  multicast_addr.addr = (uint32_t) SSDP_MULTICAST_ADDR;
  if (igmp_joingroup(&ifaddr, &multicast_addr) != ERR_OK ) {
    DEBUGV("SSDP failed to join igmp group");
    return false;
  }
  
  if (!_server->listen(*IP_ADDR_ANY, SSDP_PORT)) {
    return false;
  }

  _server->setMulticastInterface(ifaddr);
  _server->setMulticastTTL(SSDP_MULTICAST_TTL);
  _server->onRx(std::bind(&SSDPClass::_update, this));
  if (!_server->connect(multicast_addr, SSDP_PORT)) {
    return false;
  }

  _startTimer();

  return true;
}

void SSDPClass::_send(ssdp_method_t method) {
  char buffer[512];	// FIXME I've seen up to 280 but haven't calculated this
  uint32_t ip = WiFi.localIP();
  
  int len = snprintf(buffer, sizeof(buffer), 
    _ssdp_packet_template,
    (method == NONE)?_ssdp_response_template:_ssdp_notify_template,
    SSDP_INTERVAL,
    device._modelName, device._modelNumber,
    device._uuid,
    IP2STR(&ip), device.getPort(), device.getSchemaURL()
  );
#ifdef DEBUG_SSDPx
  DEBUG_SSDP.printf("SSDPClass::_send() : len %d\n", len);
#endif

  _server->append(buffer, len);

  ip_addr_t remoteAddr;
  uint16_t remotePort;
  if(method == NONE) {
    remoteAddr.addr = _respondToAddr;
    remotePort = _respondToPort;
#ifdef DEBUG_SSDPx
    DEBUG_SSDP.print("Sending Response to ");
#endif
  } else {
    // method == SEARCH or NOTIFY
    remoteAddr.addr = SSDP_MULTICAST_ADDR;
    remotePort = SSDP_PORT;
#ifdef DEBUG_SSDPx
    DEBUG_SSDP.print("Sending Notify to ");
#endif
  }
#ifdef DEBUG_SSDPx
  DEBUG_SSDP.print(IPAddress(remoteAddr.addr));
  DEBUG_SSDP.print(":");
  DEBUG_SSDP.println(remotePort);
#endif

  _server->send(&remoteAddr, remotePort);
}

/*
 * NOTIFY * HTTP/1.1
 * HOST:239.255.255.250:1900
 * CACHE-CONTROL:max-age=1800
 * LOCATION:http://192.168.1.1:8000/o8ee3npj36j/IGD/upnp/IGD.xml
 * SERVER:MediaAccess TG 789Ovn Xtream 10.A.0.D UPnP/1.0 (9C-97-26-26-44-DE)
 * NT:upnp:rootdevice
 * USN:uuid:c5eb6a02-c0b8-5afe-83da-3965c9516822::upnp:rootdevice
 * NTS:ssdp:alive
 * 
 * M-SEARCH * HTTP/1.1
 * HOST: 239.255.255.250:1900
 * MAN: "ssdp:discover"
 * MX: 10
 * ST: urn:schemas-upnp-org:device:InternetGatewayDevice:1
 * 
 */

// These are the headers detected by the code.
// Comment them out if we don't need them, will speed up the code.
struct {
  const char *header;
  char *found;
  int length;
} headers [] = {
#define M_SEARCH_HEADER 0
  { "M-SEARCH", 0, 0},
#define NOTIFY_HEADER 1
  { "NOTIFY", 0, 0},
  { "MAN:", 0, 0 },
  { "MX:", 0, 0 },
  { "ST:", 0, 0 },
  { "USN:", 0, 0 },
  { "NTS:", 0, 0 },
  // { "HOST:", 0, 0 },		// This is not interesting information
  { "LOCATION:", 0, 0 },
  { 0, 0, 0 }
};

// Called when a packet is received on the UDP socket
void SSDPClass::_update() {
  if(!_pending && _server->next()) {
    _pending = true;

    // This is picked up by _send()
    _respondToAddr = _server->getRemoteAddress();
    _respondToPort = _server->getRemotePort();

    int ssdplen = _server->getSize();

    // Allocate a byte extra before and after the message, this makes the rest of the code
    // work with fewer tests or code duplication.
    char *buffer = (char *)malloc(ssdplen+2);
    _server->read(buffer+1, ssdplen);	// FIXME return value ?
    buffer[0] = '\r';
    buffer[ssdplen+1] = 0;

    // Clear pointers
    for (int i=0; headers[i].header; i++) {
      headers[i].found = 0;
      if (headers[i].length == 0)
        headers[i].length = strlen(headers[i].header);
    }

    // Multi-step packet analysis : first cut into lines, detect headers
    for (char *p = buffer; *p; p++) {
      if (*p == '\r' || *p == '\n') {
        *p = 0;
      }
      char *q = p+1;
      
      for (int i=0; headers[i].header; i++)
	if (strncmp(headers[i].header, q, headers[i].length) == 0) {
	  headers[i].found = q;
	}
    }

    // Send a reply
    if (headers[M_SEARCH_HEADER].found)
      _send(NONE);
    else if (headers[NOTIFY_HEADER].found)
      RegisterNotify();

    // End of processing, throw away buffer
    free(buffer);

    _pending = false;
  }
}

void SSDPClass::EverySecond() {
  // No packet received, just periodically send out a NOTIFY
  if (_notify_time == 0 || (millis() - _notify_time) > (SSDP_INTERVAL * 1000L)) {
    _notify_time = millis();
    _send(NOTIFY);
  }
}

void SSDPClass::RegisterNotify() {
  // Currently not keeping track of devices telling us they're alive
}

void SSDPClass::_onTimerStatic(SSDPClass* self) {
  self->EverySecond();
}

// Call the _update method once every second.
void SSDPClass::_startTimer() {
  ETSTimer* tm = &(_timer->timer);
  const int interval = 1000;
  os_timer_disarm(tm);
  os_timer_setfn(tm, reinterpret_cast<ETSTimerFunc*>(&SSDPClass::_onTimerStatic), reinterpret_cast<void*>(this));
  os_timer_arm(tm, interval, 1 /* repeat */);
}

SSDPClass SSDP;
