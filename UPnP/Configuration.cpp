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
#include <FS.h>

#undef	DEBUG_PRINT
// #define	DEBUG_PRINT Serial

#define LINE_LEN	80

static const char *filename = "/config.txt";
static const char *filemode = "r";

int UPnPService::ReadLine(File f) {
  if (line == NULL)
    line = (char *)malloc(LINE_LEN+1);

  int len = f.readBytesUntil('\n', line, LINE_LEN);
  if (len >= 0 && len <= LINE_LEN)
    line[len] = 0;

  return len;
}

void UPnPService::ReadConfiguration(const char *name, Configuration *config) {
#ifdef DEBUG_PRINT
  DEBUG_PRINT.printf("ReadConfiguration(%s)\n", name);
#endif

  // Open file
  File f = SPIFFS.open(filename, filemode);

  int len;
  do {
    // Read line
    len = ReadLine(f);
#ifdef DEBUG_PRINTx
    DEBUG_PRINT.printf("Read line %d {%s}\n", len, line);
#endif

    // Check it against every configuration item
    char *t1 = line, *t2 = NULL, *t3 = NULL;
    int i;

    // Scan the line for a colon, separate the fields on it.
    for (i=0; i<len && line[i]; i++)
      if (line[i] == ':') {
        line[i] = 0;
	t2 = line+i+1;
	break;
      }

    // Is the configuration line about this device ?
    if (strcmp(t1, name) != 0)
      continue;

    // Continue scanning for the next colon
    for (i++; i<len && line[i]; i++)
      if (line[i] == ':') {
        line[i] = 0;
	t3 = line+i+1;
	break;
      }

    // Configuration line matching a config item ?
    ConfigurationItem *item = config->GetItem(t2);
    if (item) {
	switch (item->GetType()) {
	  case TYPE_NONE:
	    break;
	  case TYPE_DEFAULT_INT:
	  case TYPE_INT:
	    item->SetValue(atoi(t3));
#ifdef DEBUG_PRINT
        DEBUG_PRINT.printf("Configuration match for %s %s {%d}\n", t1, t2, atoi(t3));
#endif
	    break;
	  case TYPE_DEFAULT_STRING:
	  case TYPE_STRING:
	    item->SetValue(t3);
#ifdef DEBUG_PRINT
        DEBUG_PRINT.printf("Configuration match for %s %s {%s}\n", t1, t2, t3);
#endif
	    break;
	}
    }
  } while (len > 0);

  // Close file
  f.close();

  // Return
  free(line);
  line = NULL;
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
  this->svalue = NULL;
}

ConfigurationItem::ConfigurationItem(const char *name, const char *value) {
#ifdef DEBUG_PRINT
  DEBUG_PRINT.printf("ConfigurationItem::ConfigurationItem(%s)\n", name);
#endif
  this->name = name;
  char *s = (char *)malloc(strlen(value)+1);
  strcpy(s, value);
  this->svalue = s;
  this->type = TYPE_DEFAULT_STRING;
}

ConfigurationItem *Configuration::GetItem(const char *itemname) {
#ifdef DEBUG_PRINTx
  DEBUG_PRINT.printf("ConfigurationItem::GetItem(%s)\n", itemname);
#endif
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
    return NULL;
  }
#ifdef DEBUG_PRINTx
    DEBUG_PRINT.printf("ConfigurationItem::GetValue(%s)\n", name);
#endif
  ConfigurationItem *ci = GetItem(name);
  if (ci == NULL) {
    return NULL;
#ifdef DEBUG_PRINTx
    DEBUG_PRINT.printf("ConfigurationItem::GetValue(%s) ci == NULL\n", name);
#endif
  }
  return ci->GetValue();
}

int ConfigurationItem::GetValue() {
  return ivalue;
}

void ConfigurationItem::SetValue(char *v) {
  if (svalue)
    free((void *)svalue);
  char *s = (char *)malloc(strlen(v) + 1);
  strcpy(s, v);
  svalue = s;
}

void ConfigurationItem::SetValue(int v) {
  ivalue = v;
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
