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

#include <sntp.h>
#include <time.h>

const static int max_count = 10;
const static int initial_delay = 1000;

Time::Time()
{
}

Time::~Time() {
#ifdef DEBUG
  DEBUG.println("Time DTOR");
#endif  
}

void Time::begin() {
#if 0
  config = new Configuration("Time",
    new ConfigurationItem("code", "1234"),
    // new ConfigurationItem(mail, 1),			// FIXME
    new ConfigurationItem("from", ""),
    new ConfigurationItem("to", ""),
    NULL);
  //UPnPService::begin(config);
  alarmpin = config->GetValue("pin");
#endif

#ifdef DEBUG
  DEBUG.printf("Time::begin (pin %d)\n", alarmpin);
#endif

  sntp_init();
  sntp_setservername(0, "ntp.scarlet.be");
  sntp_setservername(1, "ntp.belnet.be");
  (void)sntp_set_timezone(+1);
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
