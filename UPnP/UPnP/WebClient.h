/*
  WebClient.h

  Copyright (c) 2015 Danny Backx.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


#ifndef _INCLUDE_WEB_CLIENT_H_
#define _INCLUDE_WEB_CLIENT_H_

#include <functional>
#include "UPnP/HTTP.h"

class WebClient
{
public:
  WebClient();
  ~WebClient();

  void begin();

  bool connect(const char *url);
  bool connect(char *host, char *path);
  bool connect(char *host, uint16_t port, char *path);
  bool connect(IPAddress ip, uint16_t port, char *path);
  char *send(const char *mime, const char *msg);
  char *send(char *msg);
  void setMethod(enum HTTPMethod);

private:
  char *path;
  WiFiClient *wc;

#if 0
  WiFiServer  _server;

  WiFiClient  _currentClient;
  HTTPMethod  _currentMethod;
  String      _currentUri;

  size_t           _currentArgCount;
  RequestArgument* _currentArgs;
  HTTPUpload       _currentUpload;

  size_t           _contentLength;
  String           _responseHeaders;

  String           _hostHeader;

#endif
};
#endif // _INCLUDE_WEB_CLIENT_H_
