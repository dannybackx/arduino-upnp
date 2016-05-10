/*
 * Interface to the DS3231 chip (real time clock, oscillator, temperature sensor).
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
#include <Arduino.h>
#include <time.h>

#define	DS3232RTC_I2C_ADDR	0x68

/* Addresses in DS3232 */
#define	DS3232_ADDR_RTC_SECONDS	0x00
#define	DS3232_ADDR_RTC_MINUTES	0x01

#define	DS1307_CLOCKHALT	7
#define	DS3232_12HRCLOCK	6
#define	DS3232_CENTURY		7

class DS3231 {
public:
  DS3231();
  ~DS3231();
private:
  byte ReadRange(byte addr, byte *values, byte n);
  byte ReadRTC(struct tm &tm);
  uint8_t dec2bcd(uint8_t n);
  uint8_t bcd2dec(uint8_t n);

public:
  void begin();
  void test();
  void SetRTC(time_t now);
  void GetTemperature();
};
