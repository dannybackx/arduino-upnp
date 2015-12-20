/*
 * Configuration file reader for UPnP (sub)classes
 *
 * Copyright (c) 2015 Danny Backx
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
#include "Arduino.h"
#include "UPnP.h"
#include "UPnP/Configuration.h"
#include <stdarg.h>

#undef	DEBUG_PRINT
// #define	DEBUG_PRINT Serial

void UPnPService::ReadConfiguration(const char *name, Configuration *config) {
#ifdef DEBUG_PRINT
  DEBUG_PRINT.printf("ReadConfiguration(%s)\n", name);
#endif
}

Configuration::Configuration(const char *name, ConfigurationItem *item ...) {
#ifdef DEBUG_PRINT
  DEBUG_PRINT.printf("Configuration::Configuration(%s)\n", name);
#endif
  va_list args;

  this->name = name;

  int cnt = 1;
  va_start(args, item);
  ConfigurationItem *p = item;
  while (p) {
    cnt++;
    p = va_arg(args, ConfigurationItem *);
  }
  va_end(args);

  items = (ConfigurationItem **)malloc(cnt * sizeof(ConfigurationItem *));
  int i=0;
  va_start(args, item);
  p = item;
  while (p) {
    items[i++] = p;
    p = va_arg(args, ConfigurationItem *);
  }
  va_end(args);
}

ConfigurationItem::ConfigurationItem(const char *name, int value) {
#ifdef DEBUG_PRINT
  DEBUG_PRINT.printf("ConfigurationItem::ConfigurationItem(%s)\n", name);
#endif
  this->name = name;
  this->ivalue = value;
  this->type = TYPE_DEFAULT_INT;
}

ConfigurationItem::ConfigurationItem(const char *name, const char *value) {
#ifdef DEBUG_PRINT
  DEBUG_PRINT.printf("ConfigurationItem::ConfigurationItem(%s)\n", name);
#endif
  this->name = name;
  this->svalue = value;
  this->type = TYPE_DEFAULT_STRING;
}

ConfigurationItem *Configuration::GetItem(const char *itemname) {
  for (int i=0; i<nitems; i++) {
    if (strcasecmp(itemname, items[i]->GetName()) == 0) {
      return items[i];
    }
  }
  return NULL;
}

const char *Configuration::GetName() {
  return name;
}

const char *ConfigurationItem::GetName() {
  return name;
}

int Configuration::GetValue(const char *name) {
  if (name == NULL) {
#ifdef DEBUG_PRINT
    DEBUG_PRINT.printf("ConfigurationItem::GetValue(%s)\n", name);
#endif
    return NULL;
  }
  ConfigurationItem *ci = GetItem(name);
  if (ci == NULL) {
    return NULL;
#ifdef DEBUG_PRINT
    DEBUG_PRINT.printf("ConfigurationItem::GetValue(%s) ci == NULL\n", name);
#endif
  }
  return ci->GetValue();
}

int ConfigurationItem::GetValue() {
  return ivalue;
}

const char *Configuration::GetStringValue(const char *name) {
  return GetItem(name)->GetStringValue();
}

const char *ConfigurationItem::GetStringValue() {
  return svalue;
}

enum ValueType ConfigurationItem::GetType() {
  return type;
}

bool Configuration::configured(const char *name) {
  if (name == NULL)
    return false;
  ConfigurationItem *i = GetItem(name);
  if (i == NULL)
    return false;
  switch (i->GetType()) {
#if 0
  case TYPE_NONE:
  case TYPE_DEFAULT_INT:
  case TYPE_DEFAULT_STRING:
    return false;
  case TYPE_INT:
  case TYPE_STRING:
    return true;
#else
  case TYPE_NONE:
    return false;
  case TYPE_DEFAULT_INT:
  case TYPE_DEFAULT_STRING:
  case TYPE_INT:
  case TYPE_STRING:
    return true;
#endif
  }
}
