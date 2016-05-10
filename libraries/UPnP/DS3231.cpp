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
#include <UPnP/DS3231.h>
#include <Wire.h>

#define	DEBUG	Serial

DS3231::DS3231() {
  // DEBUG.println("DS3231 CTOR");
}

DS3231::~DS3231() {
#ifdef DEBUG
  DEBUG.println("DS3231 DTOR");
#endif  
}

void DS3231::begin() {
  // Wire.begin();
}

/* Deal with Wire-based communication to DS3232 */
byte DS3231::ReadRange(byte addr, byte *values, byte n)
{
  Wire.beginTransmission(DS3232RTC_I2C_ADDR);
  Wire.write(addr);
  if (byte e = Wire.endTransmission())
    return e;
  Wire.requestFrom((uint8_t)DS3232RTC_I2C_ADDR, n);
  for (byte i=0; i<n; i++)
    values[i] = Wire.read();
  return 0;
}

/*
 * Reads the current time from the RTC and returns it in a struct tm.
 * Returns zero if successful (based on I2C status).
 */
byte DS3231::ReadRTC(struct tm &tm)
{
  // Assume Wire.begin(); already called
  unsigned char values[7];
  int i;

  // Get 7 bytes as described in the DS3231 datasheet
  char error = ReadRange(0, values, 7);
  Serial.printf("RTC return %d\n", error);
  for (i=0; i<7; i++)
    Serial.printf(" %d ", values[i]);
  Serial.printf("\t\t\t(");
  for (i=0; i<7; i++)
    Serial.printf(" %2x ", values[i]);
  Serial.printf(")\n");

  // Convert from hardware registers into struct tm
  tm.tm_sec = bcd2dec(values[0]);
  tm.tm_min = bcd2dec(values[1]);
  tm.tm_hour = bcd2dec(values[2] & 0x3F);	// Assumes 24hr clock
  int ampm = values[2] & 0x40;
  if (ampm)
    tm.tm_hour += 12;
  tm.tm_mday = bcd2dec(values[4]);
  int century = (values[5] & 0x80) >> 7;
  tm.tm_mon = bcd2dec(values[5] & 0x1F);
  tm.tm_year = century * 100 + bcd2dec(values[6]);	// FIXME Century ?
  
  return error;
}

uint8_t DS3231::dec2bcd(uint8_t n)
{
  uint8_t a, b;
  a = n / 10;
  b = n % 10;
  return a * 16 + b;
}

uint8_t DS3231::bcd2dec(uint8_t n)
{
  uint8_t a, b;

  a = n >> 4;
  b = n & 0x0F;
  return a * 10 + b;
}

void DS3231::test()
{
  Serial.println("Time::test()");

  struct tm tm;
  char error = ReadRTC(tm);

  Serial.printf("%s\n", asctime(&tm));
  // SetRTC();
  GetTemperature();
}

void DS3231::SetRTC(time_t now)
{
  // Get the official stuff
  struct tm *tmnow = localtime(&now);
  Serial.printf("%d %d %d xx %d %d %d\n",
    tmnow->tm_sec, tmnow->tm_min, tmnow->tm_hour, tmnow->tm_mday, tmnow->tm_mon, tmnow->tm_year);

  // Set the time
  Wire.beginTransmission(DS3232RTC_I2C_ADDR);
  Wire.write((uint8_t)DS3232_ADDR_RTC_SECONDS);	// Address
  Wire.write(dec2bcd(tmnow->tm_sec));
  Wire.write(dec2bcd(tmnow->tm_min));
  Wire.write(dec2bcd(tmnow->tm_hour));
  Wire.write(dec2bcd(tmnow->tm_wday));
  Wire.write(dec2bcd(tmnow->tm_mday));
  Wire.write(dec2bcd(tmnow->tm_mon) + ((tmnow->tm_year > 99) ? 0x80 : 0));
  Wire.write(dec2bcd(tmnow->tm_year - 100));
  byte werror = Wire.endTransmission();
  Serial.printf("RTC write error code %d\n", werror);
}

void DS3231::GetTemperature()
{
  unsigned char values[4];

  // values[0] = 0x11;	// Address
  char error = ReadRange(0x11, values, 2);

  float temp = values[0] + (((float)values[1]) / 256);
  Serial.printf("DS3231 temperature %d %d (error %d) -> ", values[0], values[1], error);
  Serial.println(temp);
}
