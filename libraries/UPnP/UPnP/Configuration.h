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
#ifndef _UPNP_CONFIGURATION_READER_H_
#define _UPNP_CONFIGURATION_READER_H_

enum ValueType {
  TYPE_NONE,
  TYPE_DEFAULT_INT,
  TYPE_DEFAULT_STRING,
  TYPE_INT,
  TYPE_STRING
};

class ConfigurationItem {
private:
  const char *name;
  enum ValueType type;
  int   ivalue;
  const char *svalue;
public:
  ConfigurationItem(const char *name, int value);
  ConfigurationItem(const char *name, const char *value);
  const char *GetName();
  int GetValue();
  char *GetStringValue();
  enum ValueType GetType();
  void SetValue(int v);
  void SetValue(char *v);
};

class Configuration {
private:
  const char        *name;
  ConfigurationItem **items;
  int               nitems;
public:
  Configuration(const char *name, ConfigurationItem *item ...);
  ConfigurationItem *GetItem(const char *itemname);
  const char *GetName();
  int GetValue(const char *name);
  char *GetStringValue(const char *name);
  bool configured(const char *name);
};
#endif /* _UPNP_CONFIGURATION_READER_H_ */
