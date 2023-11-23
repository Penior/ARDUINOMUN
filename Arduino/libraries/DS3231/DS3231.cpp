/*
  DS3231.cpp - Header file for the DS3231 Real-Time Clock
  Version: 1.0.0
  (c) 2020 Parker's Lab. All right reserved
  https://blog.naver.com/essemparker
  
  This library has been made to easily use the DS3231 RTC with an Arduino.
  This is free software; you can redistribute it and/or modify it.
*/

#include "DS3231.h"

#if ARDUINO >= 100
#include "Arduino.h"
#else
#include "WProgram.h"
#endif

#define DS3231_ADDR_R   0xD1
#define DS3231_ADDR_W   0xD0
#define DS3231_ADDRESS  0x68

#define REG_SECOND  0x00
#define REG_MINUTE  0x01
#define REG_HOUR    0x02
#define REG_DOW     0x03
#define REG_DAY     0x04
#define REG_MONTH   0x05
#define REG_YEAR    0x06
#define REG_CONTROL 0x0e
#define REG_STATUS  0x0f
#define REG_AGING   0x10
#define REG_TEMPM   0x11
#define REG_TEMPL   0x12

#define SECONDS_1970_TO_2000 946684800

static const uint8_t endOfMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};

/* Public */

DateTime::DateTime() {
	this->year   = 2020;
	this->month  = 3;
	this->day    = 1;
	this->hour   = 12;
	this->minute = 0;
	this->second = 0;
	this->dow    = 7;
}

DS3231::DS3231(uint8_t data_pin, uint8_t sclk_pin) {
	_sda_pin = data_pin;
	_scl_pin = sclk_pin;
}

void DS3231::begin() {
  if ((_sda_pin == SDA) and (_scl_pin == SCL)) {
    _use_hw = true;
    // activate internal pullups for twi.
    digitalWrite(SDA, HIGH);
    digitalWrite(SCL, HIGH);

    // initialize twi prescaler and bit rate
    cbi(TWSR, TWPS0);
    cbi(TWSR, TWPS1);
    TWBR = ((F_CPU / TWI_FREQ) - 16) / 2;

    // enable twi module, acks, and twi interrupt
    TWCR = _BV(TWEN) | _BV(TWIE)/* | _BV(TWEA)*/;
  } else {
    _use_hw = false;
    pinMode(_scl_pin, OUTPUT);
  }
}

DateTime DS3231::getDateTime() {
	DateTime t;
	_burstRead();
	t.second = _decode(_burstArray[0]);
	t.minute = _decode(_burstArray[1]);
	t.hour   = _decodeH(_burstArray[2]);
	t.dow    = _burstArray[3];
	t.day    = _decode(_burstArray[4]);
	t.month  = _decode(_burstArray[5]);
	t.year   = _decodeY(_burstArray[6])+2000;
	return t;
}

void DS3231::setTime(uint8_t hour, uint8_t minute, uint8_t second) {
	if (((hour>=0) && (hour<24)) && ((minute>=0) && (minute<60)) && ((second>=0) && (second<60))) {
	  _writeRegister(REG_HOUR, _encode(hour));
		_writeRegister(REG_MINUTE, _encode(minute));
		_writeRegister(REG_SECOND, _encode(second));
	}
}

void DS3231::setDate(uint16_t year, uint8_t month, uint8_t day) {
	if (((day>0) && (day<=31)) && ((month>0) && (month<=12)) && ((year>=2000) && (year<3000))) {
	  year -= 2000;
		_writeRegister(REG_YEAR, _encode(year));
		_writeRegister(REG_MONTH, _encode(month));
		_writeRegister(REG_DAY, _encode(day));
		_setDayOfWeek();
	}
}

uint8_t convert2digit(const char* p) {
  uint8_t v = 0;
  if ('0' <= *p && *p <= '9') v = *p - '0';
  return 10 * v + *++p - '0';
}

void DS3231::sendSystemTime(uint8_t seconds) {
  // System date is "Mar  6 2020" and time is "12:34:56"
  const char* date = __DATE__;
  const char* time = __TIME__;
  uint8_t  m;
  switch (date[0]) { //Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec
    case 'J': m = (date[1] == 'a') ? 1 : ((date[2] == 'n') ? 6 : 7); break;
    case 'F': m = 2;  break;
    case 'A': m = date[2] == 'r' ? 4 : 8; break;
    case 'M': m = date[2] == 'r' ? 3 : 5; break;
    case 'S': m = 9;  break;
    case 'O': m = 10; break;
    case 'N': m = 11; break;
    case 'D': m = 12; break;
  }
  uint8_t  d = convert2digit(date + 4);
  uint16_t y = convert2digit(date + 9);
  uint8_t hh = convert2digit(time);
  uint8_t mm = convert2digit(time + 3);
  uint8_t ss = convert2digit(time + 6);
  if (ss + seconds >= 60) {
    ss -= 60;
    mm++;
    if (mm >= 60) {
      mm -= 60;
      hh++;
    }
  } else {
    ss += seconds;
  }
  DS3231::setDate(2000 + y, m, d);
  DS3231::setTime(hh, mm,ss);
}

char *DS3231::getTimeStamp(uint8_t format) {
	static char output[] = "xxxxxxxx";
	DateTime t;
	t=getDateTime();
	if (t.hour<10) output[0]=48;
	else output[0]=char((t.hour / 10)+48);
	output[1]=char((t.hour % 10)+48);
	output[2]=58;
	if (t.minute<10) output[3]=48;
	else output[3]=char((t.minute / 10)+48);
	output[4]=char((t.minute % 10)+48);
	output[5]=58;
	if (format==FORMAT_SHORT)
		output[5]=0;
	else {
	  if (t.second<10) output[6]=48;
  	else output[6]=char((t.second / 10)+48);
  	output[7]=char((t.second % 10)+48);
  	output[8]=0;
	}
	return (char*)&output;
}

char *DS3231::getDateStamp(uint8_t yformat, uint8_t dformat, char divider) {
	static char output[] = "xxxxxxxxxx";
	int yr, offset;
	DateTime t;
	t=getDateTime();
	switch (dformat) {
	  case FORMAT_DMY:
			if (t.day<10) output[0]=48;
			else output[0]=char((t.day / 10)+48);
			output[1]=char((t.day % 10)+48);
			output[2]=divider;
			if (t.month<10) output[3]=48;
			else output[3]=char((t.month / 10)+48);
			output[4]=char((t.month % 10)+48);
			output[5]=divider;
			if (yformat==FORMAT_SHORT) {
			  yr=t.year-2000;
				if (yr<10) output[6]=48;
				else output[6]=char((yr / 10)+48);
				output[7]=char((yr % 10)+48);
				output[8]=0;
			} else {
			  yr=t.year;
				output[6]=char((yr / 1000)+48);
				output[7]=char(((yr % 1000) / 100)+48);
				output[8]=char(((yr % 100) / 10)+48);
				output[9]=char((yr % 10)+48);
				output[10]=0;
			}
			break;
		case FORMAT_YMD:
			if (yformat==FORMAT_SHORT) offset=0;
			else offset=2;
			if (yformat==FORMAT_SHORT) {
			  yr=t.year-2000;
				if (yr<10) output[0]=48;
				else output[0]=char((yr / 10)+48);
				output[1]=char((yr % 10)+48);
				output[2]=divider;
			} else {
			  yr=t.year;
				output[0]=char((yr / 1000)+48);
				output[1]=char(((yr % 1000) / 100)+48);
				output[2]=char(((yr % 100) / 10)+48);
				output[3]=char((yr % 10)+48);
				output[4]=divider;
			}
			if (t.month<10) output[3+offset]=48;
			else output[3+offset]=char((t.month / 10)+48);
			output[4+offset]=char((t.month % 10)+48);
			output[5+offset]=divider;
			if (t.day<10) output[6+offset]=48;
			else output[6+offset]=char((t.day / 10)+48);
			output[7+offset]=char((t.day % 10)+48);
			output[8+offset]=0;
			break;
		case FORMAT_MDY:
			if (t.month<10) output[0]=48;
			else output[0]=char((t.month / 10)+48);
			output[1]=char((t.month % 10)+48);
			output[2]=divider;
			if (t.day<10) output[3]=48;
			else output[3]=char((t.day / 10)+48);
			output[4]=char((t.day % 10)+48);
			output[5]=divider;
			if (yformat==FORMAT_SHORT) {
			  yr=t.year-2000;
				if (yr<10) output[6]=48;
				else output[6]=char((yr / 10)+48);
				output[7]=char((yr % 10)+48);
				output[8]=0;
			} else {
			  yr=t.year;
				output[6]=char((yr / 1000)+48);
				output[7]=char(((yr % 1000) / 100)+48);
				output[8]=char(((yr % 100) / 10)+48);
				output[9]=char((yr % 10)+48);
				output[10]=0;
			}
			break;
	}
	return (char*)&output;
}

char *DS3231::getDOWStamp(uint8_t format) {
	char *output = "xxxxxxxxxx";
	char *daysFull[]  = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
	char *daysShort[] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
	DateTime t;
	t=getDateTime();
	if (format == FORMAT_SHORT) output = daysShort[t.dow-1];
	else output = daysFull[t.dow-1];
	return output;
}

char *DS3231::getMonthStamp(uint8_t format) {
	char *output= "xxxxxxxxx";
	char *monthFull[]  = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
	char *monthShort[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	DateTime t;
	t=getDateTime();
	if (format == FORMAT_SHORT) output = monthShort[t.month-1];
	else output = monthFull[t.month-1];
	return output;
}

long DS3231::getUnixTime(DateTime t) {
	uint16_t dc;
	dc = t.day;
	for (uint8_t i = 0; i<(t.month-1); i++)
    dc += endOfMonth[i];
	if ((t.month > 2) && (((t.year-2000) % 4) == 0))
	  ++dc;
	dc = dc + (365 * (t.year-2000)) + (((t.year-2000) + 3) / 4) - 1;
	return ((((((dc * 24L) + t.hour) * 60) + t.minute) * 60) + t.second) + SECONDS_1970_TO_2000;
}

void DS3231::output32kHz(bool enable) {
  uint8_t _reg = _readRegister(REG_STATUS);
  _reg &= 0b11110111;
  _reg |= (enable << 3);
  _writeRegister(REG_STATUS, _reg);
}

bool DS3231::is32kHzOutput(void) {
  uint8_t _reg = _readRegister(REG_STATUS);
  _reg &= 0b00001000;
  _reg >>= 3;
  return _reg;
}

void DS3231::enableSQW(bool enable) {
  uint8_t _reg = _readRegister(REG_CONTROL);
  _reg &= 0b11111011;
  _reg |= (!enable << 2);
  _writeRegister(REG_CONTROL, _reg);
}

void DS3231::setSQWRate(int freq) {
  uint8_t _reg = _readRegister(REG_CONTROL);
  _reg &= 0b11100111;
  _reg |= (freq << 3);
  _writeRegister(REG_CONTROL, _reg);
}

uint8_t DS3231::getSQWRate(void) {
  uint8_t _reg = _readRegister(REG_CONTROL);
  _reg &= 0b00011000;
  _reg >>= 3;
  return _reg;
}

bool DS3231::isSQWMode(void) {
  uint8_t _reg;
  _reg = _readRegister(REG_CONTROL);
  _reg &= 0b00000100;
  _reg >>= 2;
  return !_reg;
}

float DS3231::getTemperature() {
	uint8_t _msb = _readRegister(REG_TEMPM);
	uint8_t _lsb = _readRegister(REG_TEMPL);
	return (float)_msb + ((_lsb >> 6) * 0.25f);
}

/* Private */
void DS3231::_setDayOfWeek() {
  int dow;
  byte mArr[12] = {6,2,2,5,0,3,5,1,4,6,2,4};
  DateTime _t = getDateTime();
  dow = (_t.year % 100);
  dow = dow*1.25;
  dow += _t.day;
  dow += mArr[_t.month-1];
  if (((_t.year % 4)==0) && (_t.month<3))
    dow -= 1;
  while (dow>7)
    dow -= 7;
  _writeRegister(REG_DOW, dow);
}

void	DS3231::_sendStart(byte addr) {
	pinMode(_sda_pin, OUTPUT);
	digitalWrite(_sda_pin, HIGH);
	digitalWrite(_scl_pin, HIGH);
	digitalWrite(_sda_pin, LOW);
	digitalWrite(_scl_pin, LOW);
	shiftOut(_sda_pin, _scl_pin, MSBFIRST, addr);
}

void	DS3231::_sendStop() {
	pinMode(_sda_pin, OUTPUT);
	digitalWrite(_sda_pin, LOW);
	digitalWrite(_scl_pin, HIGH);
	digitalWrite(_sda_pin, HIGH);
	pinMode(_sda_pin, INPUT);
}

void	DS3231::_sendNack() {
	pinMode(_sda_pin, OUTPUT);
	digitalWrite(_scl_pin, LOW);
	digitalWrite(_sda_pin, HIGH);
	digitalWrite(_scl_pin, HIGH);
	digitalWrite(_scl_pin, LOW);
	pinMode(_sda_pin, INPUT);
}

void	DS3231::_sendAck() {
	pinMode(_sda_pin, OUTPUT);
	digitalWrite(_scl_pin, LOW);
	digitalWrite(_sda_pin, LOW);
	digitalWrite(_scl_pin, HIGH);
	digitalWrite(_scl_pin, LOW);
	pinMode(_sda_pin, INPUT);
}

void	DS3231::_waitForAck() {
	pinMode(_sda_pin, INPUT);
	digitalWrite(_scl_pin, HIGH);
	while (digitalRead(_sda_pin)==HIGH) {}
	digitalWrite(_scl_pin, LOW);
}

uint8_t DS3231::_readByte() {
	pinMode(_sda_pin, INPUT);
	uint8_t value = 0;
	uint8_t currentBit = 0;
	for (int i = 0; i < 8; ++i)
	{ digitalWrite(_scl_pin, HIGH);
		currentBit = digitalRead(_sda_pin);
		value |= (currentBit << 7-i);
		delayMicroseconds(1);
		digitalWrite(_scl_pin, LOW);
	}
	return value;
}

void DS3231::_writeByte(uint8_t value) {
	pinMode(_sda_pin, OUTPUT);
	shiftOut(_sda_pin, _scl_pin, MSBFIRST, value);
}

uint8_t	DS3231::_decode(uint8_t value) {
	uint8_t decoded = value & 127;
	decoded = (decoded & 15) + 10 * ((decoded & (15 << 4)) >> 4);
	return decoded;
}

uint8_t DS3231::_decodeH(uint8_t value) {
  if (value & 128)
    value = (value & 15) + (12 * ((value & 32) >> 5));
  else
    value = (value & 15) + (10 * ((value & 48) >> 4));
  return value;
}

uint8_t	DS3231::_decodeY(uint8_t value) {
	uint8_t decoded = (value & 15) + 10 * ((value & (15 << 4)) >> 4);
	return decoded;
}

uint8_t DS3231::_encode(uint8_t value) {
	uint8_t encoded = ((value / 10) << 4) + (value % 10);
	return encoded;
}

void DS3231::_burstRead() {
  if (_use_hw) { // Send start address
    TWCR = _BV(TWEN) | _BV(TWEA) | _BV(TWINT) | _BV(TWSTA); // Send START
    while ((TWCR & _BV(TWINT)) == 0) {};                    // Wait for TWI to be ready
    TWDR = DS3231_ADDR_W;
    TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);              // Clear TWINT to proceed
    while ((TWCR & _BV(TWINT)) == 0) {};                    // Wait for TWI to be ready
    TWDR = 0;
    TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);              // Clear TWINT to proceed
    while ((TWCR & _BV(TWINT)) == 0) {};                    // Wait for TWI to be ready

    // Read data starting from start address
    TWCR = _BV(TWEN) | _BV(TWEA) | _BV(TWINT) | _BV(TWSTA); // Send rep. START
    while ((TWCR & _BV(TWINT)) == 0) {};                    // Wait for TWI to be ready
    TWDR = DS3231_ADDR_R;
    TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);              // Clear TWINT to proceed
    while ((TWCR & _BV(TWINT)) == 0) {};                    // Wait for TWI to be ready
    for (int i=0; i<7; i++) {
      TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);            // Send ACK and clear TWINT to proceed
      while ((TWCR & _BV(TWINT)) == 0) {};                  // Wait for TWI to be ready
      _burstArray[i] = TWDR;
    }
    TWCR = _BV(TWEN) | _BV(TWINT);                          // Send NACK and clear TWINT to proceed
    while ((TWCR & _BV(TWINT)) == 0) {};                    // Wait for TWI to be ready

    TWCR = _BV(TWEN)| _BV(TWINT) | _BV(TWSTO);              // Send STOP
  } else {
    _sendStart(DS3231_ADDR_W);
    _waitForAck();
    _writeByte(0);
    _waitForAck();
    _sendStart(DS3231_ADDR_R);
    _waitForAck();
    for (int i=0; i<7; i++) {
      _burstArray[i] = _readByte();
      if (i<6) _sendAck();
      else _sendNack();
    }
    _sendStop();
  }
}

uint8_t DS3231::_readRegister(uint8_t reg) {
  uint8_t readValue=0;
  if (_use_hw) {  // Send start address
    TWCR = _BV(TWEN) | _BV(TWEA) | _BV(TWINT) | _BV(TWSTA); // Send START
    while ((TWCR & _BV(TWINT)) == 0) {};                    // Wait for TWI to be ready
    TWDR = DS3231_ADDR_W;
    TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);              // Clear TWINT to proceed
    while ((TWCR & _BV(TWINT)) == 0) {};                    // Wait for TWI to be ready
    TWDR = reg;
    TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);              // Clear TWINT to proceed
    while ((TWCR & _BV(TWINT)) == 0) {};                    // Wait for TWI to be ready

    // Read data starting from start address
    TWCR = _BV(TWEN) | _BV(TWEA) | _BV(TWINT) | _BV(TWSTA); // Send rep. START
    while ((TWCR & _BV(TWINT)) == 0) {};                    // Wait for TWI to be ready
    TWDR = DS3231_ADDR_R;
    TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);              // Clear TWINT to proceed
    while ((TWCR & _BV(TWINT)) == 0) {};                    // Wait for TWI to be ready
    TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);              // Send ACK and clear TWINT to proceed
    while ((TWCR & _BV(TWINT)) == 0) {};                    // Wait for TWI to be ready
    readValue = TWDR;
    TWCR = _BV(TWEN) | _BV(TWINT);                          // Send NACK and clear TWINT to proceed
    while ((TWCR & _BV(TWINT)) == 0) {};                    // Wait for TWI to be ready

    TWCR = _BV(TWEN)| _BV(TWINT) | _BV(TWSTO);              // Send STOP
  } else {
    _sendStart(DS3231_ADDR_W);
    _waitForAck();
    _writeByte(reg);
    _waitForAck();
    _sendStart(DS3231_ADDR_R);
    _waitForAck();
    readValue = _readByte();
    _sendNack();
    _sendStop();
  }
  return readValue;
}

void DS3231::_writeRegister(uint8_t reg, uint8_t value) {
  if (_use_hw) { // Send start address
    TWCR = _BV(TWEN) | _BV(TWEA) | _BV(TWINT) | _BV(TWSTA); // Send START
    while ((TWCR & _BV(TWINT)) == 0) {};                    // Wait for TWI to be ready
    TWDR = DS3231_ADDR_W;
    TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);              // Clear TWINT to proceed
    while ((TWCR & _BV(TWINT)) == 0) {};                    // Wait for TWI to be ready
    TWDR = reg;
    TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);              // Clear TWINT to proceed
    while ((TWCR & _BV(TWINT)) == 0) {};                    // Wait for TWI to be ready
    TWDR = value;
    TWCR = _BV(TWEN) | _BV(TWINT) | _BV(TWEA);              // Clear TWINT to proceed
    while ((TWCR & _BV(TWINT)) == 0) {};                    // Wait for TWI to be ready

    TWCR = _BV(TWEN)| _BV(TWINT) | _BV(TWSTO);              // Send STOP
  } else {
    _sendStart(DS3231_ADDR_W);
    _waitForAck();
    _writeByte(reg);
    _waitForAck();
    _writeByte(value);
    _waitForAck();
    _sendStop();
  }
}
