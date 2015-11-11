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
#include "Arduino.h"
#include "UPnP/UPnPDevice.h"

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

UPnPDevice::UPnPDevice() {
  _uuid[0] = '\0';
  _modelNumber[0] = '\0';
  _friendlyName[0] = '\0';
  _presentationURL[0] = '\0';
  _serialNumber[0] = '\0';
  _modelName[0] = '\0';
  _modelURL[0] = '\0';
  _manufacturer[0] = '\0';
  _manufacturerURL[0] = '\0';
  sprintf(_schemaURL, "ssdp/schema.xml");
}

UPnPDevice::~UPnPDevice() {
}

void UPnPDevice::setSchemaURL(const char *url) {
  strlcpy(_schemaURL, url, sizeof(_schemaURL));
}

void UPnPDevice::setHTTPPort(uint16_t port){
  _port = port;
}

void UPnPDevice::setName(const char *name){
  strlcpy(_friendlyName, name, sizeof(_friendlyName));
}

void UPnPDevice::setURL(const char *url){
  strlcpy(_presentationURL, url, sizeof(_presentationURL));
}

void UPnPDevice::setSerialNumber(const char *serialNumber){
  strlcpy(_serialNumber, serialNumber, sizeof(_serialNumber));
}

void UPnPDevice::setModelName(const char *name){
  strlcpy(_modelName, name, sizeof(_modelName));
}

void UPnPDevice::setModelNumber(const char *num){
  strlcpy(_modelNumber, num, sizeof(_modelNumber));
}

void UPnPDevice::setModelURL(const char *url){
  strlcpy(_modelURL, url, sizeof(_modelURL));
}

void UPnPDevice::setManufacturer(const char *name){
  strlcpy(_manufacturer, name, sizeof(_manufacturer));
}

void UPnPDevice::setManufacturerURL(const char *url){
  strlcpy(_manufacturerURL, url, sizeof(_manufacturerURL));
}

void UPnPDevice::setPort(uint16_t port) {
	_port = port;
}

uint16_t UPnPDevice::getPort() {
	return _port;
}

char *UPnPDevice::getSchemaURL() {
	return _schemaURL;
}

char *UPnPDevice::getPresentationURL() {
	return _presentationURL;
}

char *UPnPDevice::getSerialNumber() {
	return _serialNumber;
}

char *UPnPDevice::getModelName() {
	return _modelName;
}

char *UPnPDevice::getModelNumber() {
	return _modelNumber;
}

char *UPnPDevice::getModelURL() {
	return _modelURL;
}

char *UPnPDevice::getManufacturer() {
	return _manufacturer;
}

char *UPnPDevice::getManufacturerURL() {
	return _manufacturerURL;
}

char *UPnPDevice::getUuid() {
	return _uuid;
}

char *UPnPDevice::getFriendlyName() {
	return _friendlyName;
}
