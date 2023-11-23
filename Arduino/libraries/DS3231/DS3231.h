/*
  DS3231.h - Header file for the DS3231 Real-Time Clock
  Version: 1.0.0
  (c) 2020 Parker's Lab. All right reserved
  https://blog.naver.com/essemparker
  
  This library has been made to easily use the DS3231 RTC with an Arduino.
  This is free software; you can redistribute it and/or modify it.
*/

#ifndef DS3231_h
#define DS3231_h

#if ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

// *** Hardwarespecific defines ***
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#ifndef TWI_FREQ
  #define TWI_FREQ 400000L
#endif

#define FORMAT_SHORT  1
#define FORMAT_LONG   2

#define FORMAT_DMY  1
#define FORMAT_YMD  2
#define FORMAT_MDY  3

#define MONDAY_DS      1
#define TUESDAY_DS     2
#define WEDNESDAY_DS   3
#define THURSDAY_DS    4
#define FRIDAY_DS      5
#define SATURDAY_DS    6
#define SUNDAY_DS      7

#define SQW_1Hz     0
#define SQW_1kHz    1
#define SQW_4kHz    2
#define SQW_8kHz    3

class DateTime
{
  public:
    uint16_t  year;
    uint8_t   month;
    uint8_t   day;
    uint8_t   dow;
    uint8_t   hour;
    uint8_t   minute;
    uint8_t   second;
    DateTime();
};

class DS3231
{
  public:
    DS3231(uint8_t data_pin, uint8_t sclk_pin);
    DateTime getDateTime();
    void  begin();
    void  setDate(uint16_t year, uint8_t month, uint8_t day);
    void  setTime(uint8_t hour, uint8_t minute, uint8_t second);
    void  sendSystemTime(uint8_t seconds=0);

    char  *getDateStamp(uint8_t yformat=FORMAT_LONG, uint8_t dformat=FORMAT_YMD, char divider='.');
    char  *getMonthStamp(uint8_t format=FORMAT_LONG);
    char  *getDOWStamp(uint8_t format=FORMAT_LONG);
    char  *getTimeStamp(uint8_t format=FORMAT_LONG);
    long  getUnixTime(DateTime t);
    float getTemperature();

    void  output32kHz(bool enable);
    bool  is32kHzOutput(void);
    void  enableSQW(bool enable);
    bool  isSQWMode(void);
    void  setSQWRate(int freq);
    uint8_t getSQWRate(void);

  private:
    uint8_t _scl_pin;
    uint8_t _sda_pin;
    uint8_t _burstArray[7];
    boolean _use_hw;

    void    _setDayOfWeek();
    void    _sendStart(byte addr);
    void    _sendStop();
    void    _sendAck();
    void    _sendNack();
    void    _waitForAck();
    uint8_t _readByte();
    void    _writeByte(uint8_t value);
    void    _burstRead();
    uint8_t _readRegister(uint8_t reg);
    void    _writeRegister(uint8_t reg, uint8_t value);
    uint8_t _decode(uint8_t value);
    uint8_t _decodeH(uint8_t value);
    uint8_t _decodeY(uint8_t value);
    uint8_t _encode(uint8_t vaule);
#if defined(__arm__)
  Twi *twi;
#endif
};
#endif
