#ifndef __PROGRAMMER_H__
#define __PROGRAMMER_H__
//
// Serial port interface/driver for Flash ROM programmer
// 
// Part of flashtool
//
// (c) Stuart Wallace, 2011.
//

#include <string>
#include <cstring>
#include <iostream>		// FIXME: remove
#include <algorithm>
#include <cstdint>

extern "C"
{
#include <sys/time.h>
#include <unistd.h>
};

#include "serialport.h"
#include "exception.h"

using namespace std;

#define PROGRAMMER_IDENTITY 			("\x08\x08\x19\x78\x00")
#define PROGRAMMER_IDENTITY_LEN			(5)
#define PROGRAMMER_DEFAULT_RX_TIMEOUT	(1.0)

#define PROGRAMMER_SECTOR_ERASE_TIMEOUT	(8.0)		// seconds
#define PROGRAMMER_CHIP_ERASE_TIMEOUT	(64.0)		// seconds


class Programmer
{
public:
						Programmer(const string device);
	virtual				~Programmer();

	const bool			identifyProgrammer();
	const bool			identifyROM(uint8_t& manufacturer_id, uint8_t& device_id, uint8_t& continuation_id);

	const bool			eraseSector(const uint8_t sector);
	const bool			eraseROM(void);

	const uint8_t *		read(uint32_t offset, uint32_t length);
	const bool			write(const uint8_t *data, const uint32_t offset, const uint32_t length);

protected:
	const bool			doCommand(const uint8_t* cmd, const uint32_t cmdlength, uint8_t* response, uint32_t responselength, const double rxTimeout);
	const double		getTimeOfDay(void) const;

	string				m_device;
	SerialPort			m_port;
};


class ProgrammerException : public Exception
{
public:
	ProgrammerException(const char *what)
		: Exception(what) { }
};

#endif

