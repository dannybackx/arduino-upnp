/*
 * This is class that discovers UPnP capable sensors, and keeps track of them.
 *
 * Basically an SSDP client ;-)
 * This is used in the AlarmController to keep track of the sensor modules.
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

#include "UPnP.h"
#include "UPnP/UPnPService.h"
#include "UPnP/DiscoveryManager.h"
#include "UPnP/WebServer.h"

#include "WiFiUdp.h"
#include "lwip/udp.h"
#include "lwip/igmp.h"
#include "include/UdpContext.h"

// #undef DEBUG
#define DEBUG Serial

static const char *sensor_uuid_prefix = "38323636-4558-4dda-9188-cda0e6";

static const IPAddress SSDP_MULTICAST_ADDR(239, 255, 255, 250);
// #define SSDP_INTERVAL     1200
#define SSDP_PORT         1900
// #define SSDP_METHOD_SIZE  10
// #define SSDP_URI_SIZE     2
// #define SSDP_BUFFER_SIZE  64
#define SSDP_MULTICAST_TTL 1

static const char *ssdp_all_st = "ssdp:all";
static const char *upnp_rootdevice_st = "upnp:rootdevice";

DiscoveryManager::DiscoveryManager() : UPnPService() {
  packet = NULL;
  buffer = NULL;
  devices = NULL;
  ndevices = maxdevices = 0;
  udpctx = 0;
}

DiscoveryManager::~DiscoveryManager() {
#ifdef DEBUG
  DEBUG.println("DiscoveryManager DTOR");
#endif  
  if (devices)
    free(devices);
}

void DiscoveryManager::begin() {
  /*
  config = new Configuration("LED",
    new ConfigurationItem("pin", LED_PIN_DEFAULT),
    new ConfigurationItem("name", NULL),
    new ConfigurationItem("passive", LED_PASSIVE_DEFAULT),
    new ConfigurationItem("active", LED_ACTIVE_DEFAULT),
    NULL);
  UPnPService::begin(config);
  led = config->GetValue("pin");
  /* */

#ifdef DEBUG
  DEBUG.printf("DiscoveryManager::begin\n");
#endif

}

void DiscoveryManager::AddConfiguredServers() {
}

static char *searchMulticastSSDPtemplate =
  "M-SEARCH * HTTP/1.1\r\n"
  "HOST: 239.255.255.250:1900\r\n"
  "MAN: \"SSDP:discover\"\r\n"
  "MX: 3\r\n"
  "ST: %s\r\n"
  "USER-AGENT: Arduino UPnP/2.0 Danny Backx Motion Sensor Kit/0.1\r\n"
  "CPFN.UPNP.ORG: %s\r\n"
  "CPUUID.UPNP.ORG: %s\r\n"
  "\r\n";

static char *searchUnicastSSDPtemplate =
  "M-SEARCH * HTTP/1.1\r\n"
  "HOST: 239.255.255.250:1900\r\n"
  "MAN: \"SSDP:discover\"\r\n"
  "ST: %s\r\n"
  "USER-AGENT: Arduino UPnP/2.0 Danny Backx Motion Sensor Kit/0.1\r\n";

void DiscoveryManager::receivePacket() {
#ifdef DEBUG
    DEBUG.println("DiscoveryManager::receivePacket()");
#endif
}

bool DiscoveryManager::QuerySensors() {
  // return QuerySensors(ssdp_all_st);
  return QuerySensors(upnp_rootdevice_st);
}

bool DiscoveryManager::QuerySensors(const char *st) {
#ifdef DEBUG
    DEBUG.print("DiscoveryManager::QuerySensors(");
    DEBUG.print(st);
    DEBUG.println(")");
#endif
  packet = (char *)malloc(strlen(searchMulticastSSDPtemplate) + 45);
  sprintf(packet, searchMulticastSSDPtemplate, st, "My alarm controller", "FIXMEe");
  int len = strlen(packet);

  IPAddress multi, local;
  multi = SSDP_MULTICAST_ADDR;
  local = WiFi.localIP();

  udp.begin(SSDP_PORT);

  // Make sure we're in the right multicast group to receive messages
  if (udpctx) {
    udpctx->unref();
    udpctx = 0;
  }

  udpctx = new UdpContext;
  udpctx->ref();

  ip_addr_t ifaddr;
  ifaddr.addr = WiFi.localIP();
  ip_addr_t multicast_addr;
  multicast_addr.addr = (uint32_t) SSDP_MULTICAST_ADDR;
  if (igmp_joingroup(&ifaddr, &multicast_addr) != ERR_OK ) {
    DEBUGV("SSDP failed to join igmp group");
    return false;
  }
  if (!udpctx->listen(*IP_ADDR_ANY, SSDP_PORT)) {
    return false;
  }
  udpctx->setMulticastInterface(ifaddr);
  // left something out here
  if (!udpctx->connect(multicast_addr, SSDP_PORT)) {
    return false;
  }
  // End setting up multicast group

  // UPnP docs say transmit this three times in quick succession
  udp.beginPacketMulticast(multi, SSDP_PORT, local, 1);
  udp.write(packet, len);
  udp.endPacket();
  delay(50);
  udp.beginPacketMulticast(multi, SSDP_PORT, local, 1);
  udp.write(packet, len);
  udp.endPacket();
  delay(50);
  udp.beginPacketMulticast(multi, SSDP_PORT, local, 1);
  udp.write(packet, len);
  udp.endPacket();
}

#ifdef DEBUG
extern "C" {
#include <sntp.h>
}
static time_t last = -1;
static int cnt = 2;
#endif

void DiscoveryManager::periodic() {
  // Be quick about picking up packets and processing them, don't print too much
  // debugging info e.g. to Serial because that will cause packet loss.

  int len = udp.parsePacket();
  while (len) {
    IPAddress remoteIP = udp.remoteIP();
    uint16_t remotePort = udp.remotePort();

#ifdef DEBUGx
    DEBUG.printf("UDP packet received : len %d, from ", len);
    DEBUG.print(remoteIP);
    DEBUG.printf(", port %d\n", remotePort);
#endif
    buffer = (char *)malloc(len+1);
    udp.read(buffer, len);
    buffer[len] = 0;

    ProcessPacket(buffer);
    free(buffer);
    buffer = 0;

    len = udp.parsePacket();
  }

#ifdef DEBUG
  time_t newtime = sntp_get_current_timestamp();
  if (last < 0)
    last = newtime;
  if (newtime - last > 60 && cnt-- >= 0) {
    last = newtime;
    if (maxdevices == 0)
      Serial.println("No devices");
    else
      for (int i=0; i<maxdevices; i++) 
        if (devices[i].port) {
          if (devices[i].usn)
            Serial.printf("Device %d USN %s \n", i, devices[i].usn);
          else
            Serial.printf("Device %d location %s \n", i, devices[i].location);
          Serial.printf("  %u.%u.%u.%u %d %s\n",
	    devices[i].ip[0], devices[i].ip[1], devices[i].ip[2], devices[i].ip[3],
	    devices[i].port,
	    devices[i].friendlyname);
        }
  }
#endif
}

// These are the headers detected by the code.
// Comment them out if we don't need them, will speed up the code.
struct {
  const char *header;
  char *found;
  int length;
} headers [] = {
#define HEADER_M_SEARCH 0
  { "M-SEARCH", 0, 0},
#define HEADER_NOTIFY 1
  { "NOTIFY", 0, 0},
#define HEADER_HTTP_1_1 2
  { "HTTP/1.1 200 OK", 0, 0},
#define HEADER_MAN 3
  { "MAN:", 0, 0 },
#define HEADER_EXT 4
  { "EXT:", 0, 0 },
#define HEADER_MX 5
  { "MX:", 0, 0 },
#define HEADER_ST 6
  { "ST:", 0, 0 },
#define HEADER_USN 7
  { "USN:", 0, 0 },
#define HEADER_NTS 8
  { "NTS:", 0, 0 },
#define HEADER_LOCATION 9
  { "LOCATION:", 0, 0 },
#define HEADER_SERVER 10
  { "SERVER:", 0, 0 },
//{ "HOST:", 0, 0 },		// This is not interesting information
  { 0, 0, 0 }
};

void DiscoveryManager::ProcessPacket(char *packet) {
  // Per packet : initialize "found" marks to NULL
  for (int i=0; headers[i].header; i++) {
      headers[i].found = 0;

      // Happens only once : initialize the length field.
      if (headers[i].length == 0)
        headers[i].length = strlen(headers[i].header);
  }

  // Scan the packet, cut into lines, identify headers
  for (char *p=packet; *p; p++) {
    if (*p == '\r' || *p == '\n') {
      *p = 0;
      char *q = p+1;

      for (int i=0; headers[i].header; i++)
        if (strncmp(q, headers[i].header, headers[i].length) == 0)
	  headers[i].found = q;
    }
  }

  if (headers[HEADER_USN].found) {
    char *usn = headers[HEADER_USN].found + headers[HEADER_USN].length;

    // Skip initial blanks
    while (*usn && *usn == ' ') usn++;
    if (strncmp(usn, "uuid:", 5) == 0) {
      char *uuid = usn + 5;
      if (*uuid == ' ')
        ++uuid;		// Skip one space

      if (strncmp(uuid, sensor_uuid_prefix, strlen(sensor_uuid_prefix)) == 0) {
        AddDevice();
      } else {
#if 0
        Serial.print("Ignoring device 1 .. ");
        Serial.println(headers[HEADER_USN].found);
#endif
      }
    } else {
#if 0
      Serial.print("Ignoring device 2 .. ");
      Serial.println(headers[HEADER_USN].found);
#endif
    }
  } else {
#if 0
    Serial.print("Ignoring device 3 .. ");
    Serial.println(headers[HEADER_USN].found);
#endif
  }
#if 0
  if (headers[HEADER_LOCATION].found)
    Serial.printf("ProcessPacket(%s)\n", headers[HEADER_LOCATION].found + headers[HEADER_LOCATION].length);
#endif
}

// Add a devices, if not already present, based on content of "headers"
void DiscoveryManager::AddDevice() {
  // Look it up
  for (int i=0; i<maxdevices; i++)
    if (devices[i].usn && strcmp(devices[i].usn, headers[HEADER_USN].found + headers[HEADER_USN].length) == 0) {
      Serial.printf("DM: duplicate (%u.%u.%u.%u)\n",
	devices[i].ip[0], devices[i].ip[1], devices[i].ip[2], devices[i].ip[3]);
      return;
    }

  // Need to allocation an additional entry
  if (ndevices == maxdevices) {
    maxdevices += DISCOVER_DEVICES_INCREMENT;
    devices = (struct DiscoveredDevice *)realloc((void *)devices, maxdevices * sizeof(struct DiscoveredDevice));

    for (int i=ndevices; i<maxdevices; i++) {
      devices[i].location = NULL;
      devices[i].friendlyname = NULL;
      devices[i].usn = NULL;
      devices[i].upnptype = NULL;
      devices[i].ip = IPAddress(0,0,0,0);
      devices[i].port = 0;
    }
  }

  // Look for an empty spot in the array
  int i;
  for (i=0; i<maxdevices; i++)
    if (devices[i].port == 0)	// Because 0 is not a working port number
      break;

  // copy the data
  devices[i].ip = udp.remoteIP();
  devices[i].port = udp.remotePort();
  if (headers[HEADER_USN].found)
    devices[i].usn = strdup(headers[HEADER_USN].found + headers[HEADER_USN].length);
  if (headers[HEADER_LOCATION].found)
    devices[i].location = strdup(headers[HEADER_LOCATION].found + headers[HEADER_LOCATION].length);
  if (headers[HEADER_ST].found)
    devices[i].upnptype = strdup(headers[HEADER_ST].found + headers[HEADER_ST].length);
  if (headers[HEADER_SERVER].found)
    devices[i].friendlyname = strdup(headers[HEADER_SERVER].found + headers[HEADER_SERVER].length);

  ndevices++;

#ifdef DEBUG
  DEBUG.print("DM new [");
  DEBUG.print(i);
  DEBUG.print("] : ");
  DEBUG.println(udp.remoteIP());
#endif
}

// FIXME
void DiscoveryManager::RemoveDevice() {
}
