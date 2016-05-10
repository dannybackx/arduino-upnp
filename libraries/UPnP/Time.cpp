/*
 * This provides real time info, based on one of several sources.
 * 
 * Copyright (c) 2016 Danny Backx
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
#include <UPnP/Time.h>
#include <UPnP/Configuration.h>

extern "C" {
#include <sntp.h>
#include <time.h>
}

#include <Wire.h>

#define	DEBUG	Serial

// const static int max_count = 10;
const static int initial_delay = 1000;
// const static int timezone = +1;

int max_count, timezone;
char *ntp1, *ntp2;

Time::Time() {
  // DEBUG.println("Time CTOR");
  timeSource = TIME_SOURCE_NONE;
}

Time::~Time() {
#ifdef DEBUG
  DEBUG.println("Time DTOR");
#endif  
}

void Time::begin() {
  config = new Configuration("Time",
    new ConfigurationItem("maxcount", 10),
    new ConfigurationItem("ntp1", "ntp.scarlet.be"),
    new ConfigurationItem("ntp2", "ntp.belnet.be"),
    new ConfigurationItem("timezone", 1),
    NULL);

  max_count = config->GetValue("maxcount");
  ntp1 = (char *)config->GetStringValue("ntp1");
  ntp2 = (char *)config->GetStringValue("ntp2");
  timezone = config->GetValue("timezone");

#if 0
  //UPnPService::begin(config);
  //alarmpin = config->GetValue("pin");
#endif

  sntp_init();
  sntp_setservername(0, ntp1);
  sntp_setservername(1, ntp2);
  (void)sntp_set_timezone(timezone);

  // Wire.begin();
}

/*
 * Wait for a correct time, and report it.
 * Don't wait longer than ... for it.
 */
time_t Time::getTime() {
  time_t t;
  int count = 0;

  t = sntp_get_current_timestamp();
  while (t < 0x1000) {
    count++;
    if (count > max_count)
      return 0;
    delay(initial_delay);
    t = sntp_get_current_timestamp();
  }
  return t;
}

#if 0
#define	DS3232RTC_I2C_ADDR	0x68

/* Addresses in DS3232 */
#define	DS3232_ADDR_RTC_SECONDS	0x00
#define	DS3232_ADDR_RTC_MINUTES	0x01

#define	DS1307_CLOCKHALT	7
#define	DS3232_12HRCLOCK	6
#define	DS3232_CENTURY		7
#endif
