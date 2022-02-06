/*
 * The MIT License (MIT)
 *
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

#ifndef __PCF85063A_H__
#define __PCF85063A_H__

#include <time.h>
#include <Wire.h>

/* See https://www.nxp.com/docs/en/data-sheet/PCF85063A.pdf for a
 * description of the registers */

/* Type for holding PCF85063A control registers 1 and 2 */
typedef uint16_t PCF85063A_Regs;

/* Control register 1 */
#define PCF85063A_REG_EXT_TEST          (uint16_t)0x0080
#define PCF85063A_REG_STOP              (uint16_t)0x0020
#define PCF85063A_REG_SR                (uint16_t)0x0010
#define PCF85063A_REG_CIE               (uint16_t)0x0004
#define PCF85063A_REG_12_24             (uint16_t)0x0002
#define PCF85063A_REG_CAP_SEL           (uint16_t)0x0001

/* Control register 2 */
#define PCF85063A_REG_AIE               (uint16_t)0x8000
#define PCF85063A_REG_AF                (uint16_t)0x4000
#define PCF85063A_REG_MI                (uint16_t)0x2000
#define PCF85063A_REG_HMI               (uint16_t)0x1000
#define PCF85063A_REG_TF                (uint16_t)0x0800
#define PCF85063A_REG_COF               (uint16_t)0x0700

/* Timer mode register - not part of PCF85063A_Regs */
#define PCF85063A_REG_TCF               (uint16_t)0x08
#define PCF85063A_REG_TE                (uint16_t)0x04
#define PCF85063A_REG_TIE               (uint16_t)0x02
#define PCF85063A_REG_TI_TP             (uint16_t)0x01

#define PCF85063A_REG_GET(regs, reg) (!!((regs) & (reg)))
#define PCF85063A_REG_SET(regs, reg) do { (regs) |= (reg); } while(0)
#define PCF85063A_REG_CLEAR(regs, reg) do { (regs) &= ~(reg); } while(0)

class PCF85063A
{
  private:
    /**
     * Parse a BCD-encoded decimal into decimal.
     *
     * @param   bcd     The encoded decimal
     *
     * @return  Decoded decimal
     */
    uint8_t bcd_decode(uint8_t bcd);

    /**
     * Encode a decimal into BCD
     *
     * @param   dec     Decimal to encode
     *
     * @return  Encoded value
     */
    uint8_t bcd_encode(uint8_t dec);

  public:
    enum CountdownSrcClock { CNTDOWN_CLOCK_4096HZ   = 0,
                             CNTDOWN_CLOCK_64HZ     = 1,
                             CNTDOWN_CLOCK_1HZ      = 2,
                             CNTDOWN_CLOCK_1PER60HZ = 3 };

    PCF85063A();

    /**
     * Get current time of the RTC.
     *
     * @param   now     Current time is written here
     *
     * @return  True if clock source integrity was
     *          guaranteed
     */
    bool time_get(tm *now);

    /**
     * Set current time of the RTC.
     *
     * @param   new_time      New time to set
     *
     * @return  True if setting time succeeded
     */
    bool time_set(tm *new_time);

    /**
     * Reset the RTC.
     * NXP recommends doing this after powering on.
     *
     * @return  True if clock was reset successfully
     */
    bool reset();

    /**
     * Stop/resume the RTC.
     *
     * @param  stopped  Whether the clock should be stopped
     *                  or running
     *
     * @return  True if stop state was set successfully
     */
    bool stop(bool stopped);

    /**
     * Set the output frequency of the CLKOUT pin or disable
     * it.
     *
     * Valid frequencies (in Hz) are: 0 (drive CLKOUT to
     * high-Z), 1, 10, 1024, 2048, 4096, 8192, 16384, 32768.
     *
     * @param   freq    CLKOUT square wave frequency
     *
     * @return  True if the given frequency was valid
     */
    bool clkout_freq_set(uint16_t freq);

    /**
     * Configure the countdown timer.
     * When the timer expires, the control register will
     * be modified and an interrupt is generated, if enabled.
     *
     * The countdown period becomes: value / f, where f is
     * the frequency selected by the source_clock parameter.
     *
     * Setting value to 0 stops the timer.
     *
     * Calling this function will reset the countdown timer,
     * so the next interrupt after the call will occur after
     * the configured period (1/f).
     *
     * @param   enable        Enable (or disable) countdown timer
     * @param   source_clock  Source clock selection
     * @param   value         Countdown timer value
     * @param   int_enable    Enable interrupts when timer expires
     * @param   int_pulse     Interrupt generates a pulse. Otherwise,
     *                        the interrupt line follows TF flag in control
     *                        register 2
     *
     * @return  True if setting up countdown timer succeeded
     */
    bool countdown_set(bool enable, CountdownSrcClock source_clock,
                       uint8_t value, bool int_enable, bool int_pulse);

    /**
     * Get the current countdown timer value.
     *
     * @param   value  Countdown timer value out (see countdown_set()
     *                 comments for interpreting this)
     *
     * @return  True if value was read successfully
     */
    bool countdown_get(uint8_t *value);

    /**
     * Configure alarm.
     *
     * When the alarm triggers, the control register will be
     * modified and an interrupt is generated, if enabled.
     *
     * The alarm triggers when all enabled (not -1) variables
     * match the current time.
     *
     * @param   second    Second (0-59) on which the alarm should
     *                    trigger. Set to -1 to ignore seconds.
     *
     * @param   minute    Minute (0-59) on which the alarm should
     *                    trigger. Set to -1 to ignore minutes.
     *
     * @param   hour      Hour (0-23) on which the alarm should
     *                    trigger. Set to -1 to ignore hours.
     *
     * @param   day       Day (0-31) on which the alarm should
     *                    trigger. Set to -1 to ignore days.
     *
     * @param   weekday   Weekday (0-6) on which the alarm should
     *                    trigger. Set to -1 to ignore weekdays.
     *
     * @param   int_pulse Interrupt generates a signal on the INT pin.
     *                    The interrupt line follows AF flag in control
     *                    register 2
     *
     * @return  True if the given parameters were valid
     */
    bool alarm_set(int second, int minute, int hour, int day, int weekday, bool enable_int);

    /**
     * Configure alarm.
     *
     * When the alarm triggers, the control register will be
     * modified and an interrupt is generated, if enabled.
     *
     * The alarm triggers when all enabled (not -1) variables
     * match the current time.
     *
     * @param   new_time      New alarm time to set
     *
     * @param   int_pulse Interrupt generates a signal on the INT pin.
     *                    The interrupt line follows AF flag in control
     *                    register 2
     *
     * @return  True if the given parameters were valid
     */
    bool alarm_set(tm *nt, bool enable_int);

    /**
     * Read control registers.
     *
     * @param   regs    Buffer for reading registers
     *
     * @return  True if registers were read successfully
     */
    bool ctrl_get(PCF85063A_Regs *regs);

    /**
     * Write control registers.
     *
     * @param   regs        Register buffer
     * @param   mask_alarms Set AF/TF bits high. Their state will not be
     *                      affected as a result, so you can handle pending
     *                      alarms later
     *
     * @return  True if registers were written successfully
     */
    bool ctrl_set(PCF85063A_Regs regs, bool mask_alarms);

    /**
     * Read RAM register.
     *
     * @return  Ram contents if read successfully, -1 otherwise
     */
    int16_t ram_get();

    /**
     * Write RAM register.
     *
     * @return  True if registers were written successfully
     */
    bool ram_set(uint8_t ram);

};

#endif
