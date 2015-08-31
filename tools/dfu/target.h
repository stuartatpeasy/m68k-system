#ifndef __PROGRAMMER_H__
#define __PROGRAMMER_H__
//
// Serial port interface/driver for running a DFU operation on an m68k-system board
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

#define TARGET_DEFAULT_RX_TIMEOUT	(1.0)


class Target
{
public:
						Target(const string device);
	virtual				~Target();

	const bool			find();
	const bool          program(const uint8_t *buffer, const size_t len);
	SerialPort&         port(void);

protected:
	const bool          doCommand(const char *cmd, const ssize_t cmdlength, char *response, ssize_t responselength);
	const bool          doCommand(const char *cmd, char *response, ssize_t responselength);
	const bool          command(const char *cmd, const char *expectedResponse);
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

