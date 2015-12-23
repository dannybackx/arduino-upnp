/*
 * WebClient.cpp - send a HTTP query to another device
 *   Only built to be used for UPnP NOTIFY.
 *
 * Copyright (c) 2015 Danny Backx. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include <Arduino.h>
#include "WiFiClient.h"
#include "UPnP/WebClient.h"
#include <ESP8266WiFi.h>

#define DISABLE_NETWORK

/*
 * Select only one of these lines :
 */
// #undef DEBUG_OUTPUT
#define DEBUG_OUTPUT Serial

WebClient::WebClient() {
  wc = NULL;
}

WebClient::~WebClient() {
  wc->stop();
  delete wc;
}

bool WebClient::connect(const char *url) {
#ifdef DEBUG_OUTPUT
  DEBUG_OUTPUT.printf("WC::connect(%s)\n", url);
#endif

  // Insist on having a correct URL, and HTTP as protocol
  if (strncmp(url, "http://", 7) != 0)
    return false;

  // Look for the parts of the URL
  const char *p, *port = NULL, *host = url+7, *path = NULL;
  for (p = host; *p; p++)
    if (*p == ':' && port == NULL) {
      port = p+1;
    } else if (*p == '/' && path == NULL) {
      path = p;
    }

  // Save and copy the parts of the URL
  int len;
  if (port) {	// Port points just after the colon
    len = (int)(port-host);
    this->host = (char *)malloc(len);
    strncpy(this->host, host, len-1);
    this->host[len-1] = 0;
  } else {	// Path points to the slash
    len = (int)(path-host);
    this->host = (char *)malloc(len+1);
    strncpy(this->host, host, len);
    this->host[len] = 0;
  }
  
  this->port = 80;
  if (port)
    this->port = atoi(port);

  // No need to reallocate this
  this->path = (char *)path;

  // Use it.
  // return connect("192.168.1.176", 1234, "");
#ifdef DEBUG_OUTPUT
  DEBUG_OUTPUT.printf("WC::connect(%s,%d,%s)\n", this->host, this->port, this->path);
#endif
  return connect(this->host, this->port, this->path);
}

bool WebClient::connect(const char *host, const char *path) {
  connect(host, 80, path);
}

bool WebClient::connect(const char *host, uint16_t port, const char *path) {
  IPAddress ip;
  if (WiFi.hostByName(host, ip) <= 0)
    return false;

  this->path = path;
  return connect(ip, port);
}

bool WebClient::connect(IPAddress ip, uint16_t port) {
  int success = 0;
  if (wc == NULL)
    wc = new WiFiClient();

  success = wc->connect(ip, port);
#ifdef DEBUG_OUTPUT
  DEBUG_OUTPUT.printf("wc->connect => %d, wc %p\n", success, wc);
#endif

  return success;
}

void WebClient::setMethod(enum HTTPMethod) {
}

char *WebClient::send(const char *mime, const char *msg) {
#ifdef DEBUG_OUTPUTx
  DEBUG_OUTPUT.printf("WebClient::send(%s)\n", msg);
#endif
#if 0
  if (wc->status() == WL_CONNECTED)
    wc->write(msg, strlen(msg));
  else {
#ifdef DEBUG_OUTPUTx
    DEBUG_OUTPUT.printf("wc::send : not connected %d\n", wc->status());
#endif
  }
#else
    wc->write(msg, strlen(msg));
#endif
}

char *WebClient::send(char *msg) {
#ifdef DEBUG_OUTPUT
  DEBUG_OUTPUT.printf("WebClient::send(%f)\n", msg);
#endif
#if 0
#endif
}
