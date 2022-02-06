/*
 * The MIT License (MIT)
 *
 * based on Copyright (c) 2018 Jaakko Salo (jaakkos@gmail.com / jaakkos on Freenode)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "PCF85063A.h"

#define REG_CTRL1_ADDR                   0x00
#define REG_CTRL2_ADDR                   0x01
#define REG_OFFSET_ADDR                  0x02
#define REG_RAM_ADDR                     0x03
#define REG_TIME_DATE_ADDR               0x04
#define REG_ALARM_ADDR                   0x0B
#define REG_COUNTDOWN_TIMER_VALUE_ADDR   0x10
#define REG_COUNTDOWN_TIMER_MODE_ADDR    0x11

#define I2C_ADDR                         0x51

static bool i2c_read(uint8_t reg, uint8_t bytes, uint8_t *in)
{
  uint8_t nread = 0;
  int ibyte;

  Wire.beginTransmission(I2C_ADDR);

  size_t wret = Wire.write(reg);
  if (Wire.endTransmission() != 0 ||
      wret != 1)
  {
    return false;
  }

  Wire.requestFrom((uint8_t)I2C_ADDR, bytes);

  while (Wire.available() &&
         ((ibyte = Wire.read()) != -1) &&
         nread < bytes)
  {
    in[nread] = ibyte;
    nread++;
  }

  return nread == bytes;
}

static bool i2c_write(uint8_t reg, uint8_t bytes, uint8_t *out)
{
  Wire.beginTransmission(I2C_ADDR);

  size_t wret = Wire.write(reg);
  wret += Wire.write(out, bytes);

  if (Wire.endTransmission() != 0 ||
      wret != (bytes+1))
  {
    return false;
  }

  return true;
}

uint8_t
PCF85063A::bcd_decode(uint8_t bcd)
{
  return (bcd >> 4) * 10 + (bcd & 0x0F);
}

uint8_t
PCF85063A::bcd_encode(uint8_t dec)
{
  return ((dec / 10) << 4) | (dec % 10);
}

PCF85063A::PCF85063A()
{
  Wire.begin();
}

bool
PCF85063A::time_get(tm *now)
{
  uint8_t buf[7];

  if (!i2c_read(REG_TIME_DATE_ADDR, sizeof(buf), buf))
    return false;

  now->tm_sec   = bcd_decode(buf[0] & ~0x80);
  now->tm_min   = bcd_decode(buf[1] & ~0x80);
  now->tm_hour  = bcd_decode(buf[2] & ~0xC0); /* 24h clock */
  now->tm_mday  = bcd_decode(buf[3] & ~0xC0);
  now->tm_wday  = bcd_decode(buf[4] & ~0xF8);
  now->tm_mon   = bcd_decode(buf[5] & ~0xE0);
  now->tm_year  = bcd_decode(buf[6]);

  return !(buf[0] & 0x80);
}

bool
PCF85063A::time_set(tm *new_time)
{
  uint8_t buf[7];

  buf[0] = bcd_encode(new_time->tm_sec);
  buf[1] = bcd_encode(new_time->tm_min);
  buf[2] = bcd_encode(new_time->tm_hour);
  buf[3] = bcd_encode(new_time->tm_mday);
  buf[4] = bcd_encode(new_time->tm_wday);
  buf[5] = bcd_encode(new_time->tm_mon);
  buf[6] = bcd_encode(new_time->tm_year);

  return i2c_write(REG_TIME_DATE_ADDR, sizeof(buf), buf);
}

bool
PCF85063A::reset()
{
  uint8_t buf = 0x58;
  return i2c_write(REG_CTRL1_ADDR, 1, &buf);
}

bool
PCF85063A::stop(bool stopped)
{
  PCF85063A_Regs regs;

  if (!this->ctrl_get(&regs))
    return false;

  if (stopped) PCF85063A_REG_SET(regs, PCF85063A_REG_STOP);
  else         PCF85063A_REG_CLEAR(regs, PCF85063A_REG_STOP);

  return this->ctrl_set(regs, true);
}

bool
PCF85063A::clkout_freq_set(uint16_t freq)
{
  uint8_t COF;
  PCF85063A_Regs regs;

  switch (freq)
  {
    case 0:     COF = 7; break;
    case 1:     COF = 6; break;
    case 1024:  COF = 5; break;
    case 2048:  COF = 4; break;
    case 4096:  COF = 3; break;
    case 8192:  COF = 2; break;
    case 16384: COF = 1; break;
    case 32768: COF = 0; break;
    default: return false;
  }

  if (!this->ctrl_get(&regs))
    return false;

  PCF85063A_REG_CLEAR(regs, PCF85063A_REG_COF);
  regs |= ((uint16_t)COF) << 8;

  return this->ctrl_set(regs, true);
}

bool
PCF85063A::countdown_set(bool enable,
                         CountdownSrcClock source_clock,
                         uint8_t value,
                         bool int_enable,
                         bool int_pulse)
{
  uint8_t timer_reg[2] = {0};

  if (source_clock < 0 || source_clock > 3)
    return false;

  /* First disable the countdown timer */
  if (!i2c_write(REG_COUNTDOWN_TIMER_MODE_ADDR, 1, timer_reg+1))
    return false;

  /* Reconfigure timer */
  if (enable) timer_reg[1] |= PCF85063A_REG_TE;
  if (int_enable) timer_reg[1] |= PCF85063A_REG_TIE;
  if (int_pulse) timer_reg[1] |= PCF85063A_REG_TI_TP;
  timer_reg[1] |= source_clock << 3;
  timer_reg[0] = value;

  return i2c_write(REG_COUNTDOWN_TIMER_VALUE_ADDR, 2, timer_reg);
}

bool
PCF85063A::countdown_get(uint8_t *value)
{
  return i2c_read(REG_COUNTDOWN_TIMER_VALUE_ADDR, 1, value);
}

bool
PCF85063A::alarm_set(tm *nt, bool enable_int) {
  return alarm_set(nt->tm_sec, nt->tm_min, nt->tm_hour, nt->tm_mday, nt->tm_wday, enable_int);
}

bool
PCF85063A::alarm_set(int second, int minute, int hour, int day,
                     int weekday, bool enable_int)
{
  uint8_t buf[5];

  if ((second < 0 || second > 59) && second != -1) return false;
  if ((minute < 0 || minute > 59) && minute != -1) return false;
  if ((hour < 0 || hour > 23) && hour != -1) return false;
  if ((day < 0 || day > 31) && day != -1) return false;
  if ((weekday < 0 || weekday > 6) && weekday != -1) return false;

  buf[0] = second < 0 ? 0x80 : bcd_encode(second);
  buf[1] = minute < 0 ? 0x80 : bcd_encode(minute);
  buf[2] = hour < 0 ? 0x80 : bcd_encode(hour);
  buf[3] = day < 0 ? 0x80 : bcd_encode(day);
  buf[4] = weekday < 0 ? 0x80 : bcd_encode(weekday);

  if(enable_int) {
    PCF85063A_Regs regs = 0;
    ctrl_get(&regs);
  
    PCF85063A_REG_SET(regs, PCF85063A_REG_AIE);
    ctrl_set(regs, true);    
  }
  
  return i2c_write(REG_ALARM_ADDR, sizeof(buf), buf);
}

bool
PCF85063A::ctrl_get(PCF85063A_Regs *regs)
{
  uint8_t buf[2];

  if (!i2c_read(REG_CTRL1_ADDR, sizeof(buf), buf))
    return false;

  *regs = buf[0];
  *regs |= ((uint16_t)buf[1]) << 8;

  return true;
}

bool
PCF85063A::ctrl_set(PCF85063A_Regs regs, bool mask_alarms)
{
  uint8_t buf[2];
  int wrsz;

  if (mask_alarms)
    regs &= ~(PCF85063A_REG_AF | PCF85063A_REG_TF);

  buf[0] = regs & 0xFF;
  buf[1] = regs >> 8;

  return i2c_write(REG_CTRL1_ADDR, sizeof(buf), buf);
}

int16_t
PCF85063A::ram_get()
{
  uint8_t ram = 0;

  if(!i2c_read(REG_RAM_ADDR, sizeof(ram), &ram)) {
    return -1;
  }

  return ram;
}

bool
PCF85063A::ram_set(uint8_t ram)
{
  return i2c_write(REG_RAM_ADDR, sizeof(ram), &ram);
}
