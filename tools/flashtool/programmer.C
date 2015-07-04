//
// Serial port interface/driver for Flash ROM programmer
// 
// Part of flashtool
//
// (c) Stuart Wallace, 2011.
//

#include "programmer.h"


Programmer::Programmer(const string device)
	: m_device(device), m_port(device)
{

}


Programmer::~Programmer()
{

}


const bool Programmer::doCommand(const uint8_t* cmd, const uint32_t cmdlength, uint8_t* response, uint32_t responselength, const double rxTimeout = PROGRAMMER_DEFAULT_RX_TIMEOUT)
{
	m_port.write(cmd, cmdlength);

	m_port.setNonBlocking();
	double rxWaitStart = getTimeOfDay();
	while(responselength)
	{
		uint8_t buf[256];
		ssize_t bytes_read = m_port.read(buf, 256);

		if((rxWaitStart + rxTimeout) < getTimeOfDay())
			throw ProgrammerException("Command timeout");

		if(bytes_read == -1)
		{
			if(errno == EAGAIN)
			{
				::usleep(1000);			// TODO: review this delay
				continue;
			}
			else
			{
				m_port.setBlocking();
				return false;			// read failed
			}
		}

		::memcpy(response, buf, bytes_read);
		response += bytes_read;
		responselength -= bytes_read;
	}

	m_port.setBlocking();
	return true;
}


// identifyProgrammer() - returns true if a valid identity could be read from the device; false otherwise.
//
const bool Programmer::identifyProgrammer(void)
{
	uint8_t response[5];

	return doCommand((const uint8_t *) "?", 1, response, 5) &&
				!::memcmp(PROGRAMMER_IDENTITY, response, PROGRAMMER_IDENTITY_LEN);
}


const bool Programmer::identifyROM(uint8_t& manufacturer_id, uint8_t& device_id, uint8_t& continuation_id)
{
	uint8_t response[4];

	if(doCommand((const uint8_t *) "I", 1, response, 4) && (response[3] == 0))
	{
		manufacturer_id = response[0];
		device_id = response[1];
		continuation_id = response[2];

		return true;
	}
	return false;
}


const bool Programmer::eraseROM(void)
{
	uint8_t response;
	return doCommand((const uint8_t *) "C", 1, &response, 1, PROGRAMMER_CHIP_ERASE_TIMEOUT) && (response == 'Z');
}


const bool Programmer::eraseSector(const uint8_t sector)
{
	char command[3];
	uint8_t response;

	if(sector > 7)
		return false;

	::sprintf(command, "E%u", sector);
	return doCommand((const uint8_t *) command, 2, &response, 1, PROGRAMMER_SECTOR_ERASE_TIMEOUT) && (response == 'Z');
}


const uint8_t* Programmer::read(uint32_t offset, uint32_t length)
{
	uint8_t *pdata = new uint8_t[(length + 255) & 0xffffff00];
	uint32_t bytes_read = 0;

	if(!length)
		return NULL;

	while(bytes_read < length)
	{
		char command[6];
		::sprintf(command, "R%c%c%c%c", ((offset + bytes_read) & 0xff000000) >> 24,
										((offset + bytes_read) & 0xff0000) >> 16,
										((offset + bytes_read) & 0xff00) >> 8,
										((offset + bytes_read) & 0xff));

		uint8_t buf[256];
		if(doCommand((const uint8_t *) command, 5, buf, 256))
		{
			::memcpy(pdata + bytes_read, buf, 256);
			bytes_read += 256;
		}
		else return NULL;
	}

	return pdata;
}


const bool Programmer::write(const uint8_t* data, const uint32_t offset, const uint32_t length)
{
	char program_buffer[266];
	uint32_t bytes_sent = 0;

	// determine which sectors need to be erased
	// assumes 64KiB sectors
	for(uint8_t sector = offset >> 16; sector <= ((offset + length) >> 16); ++sector)
		if(!eraseSector(sector))
			return false;

	while(bytes_sent < length)
	{
		::sprintf(program_buffer, "P%c%c%c%c", 	((offset + bytes_sent) & 0xff000000) >> 24,
												((offset + bytes_sent) & 0xff0000) >> 16,
												((offset + bytes_sent) & 0xff00) >> 8,
												 (offset + bytes_sent) & 0xff);

		::memset(program_buffer + 5, 0xff, 256);
		::memcpy(program_buffer + 5, data + bytes_sent, min((uint32_t) 256, length - bytes_sent)); 

		// compute checksum
		uint32_t checksum = 0;;
		for(int i = 0; i < 256; ++i)
			checksum += (uint8_t) program_buffer[i + 5];

		::sprintf(program_buffer + 261, "%c%c%c%c", (checksum & 0xff000000) >> 24,
													(checksum & 0xff0000) >> 16,
													(checksum & 0xff00) >> 8,
													checksum & 0xff);

		// send command
		uint8_t response;
		if(!doCommand((const uint8_t *) program_buffer, 265, &response, 1) || (response != 'Z'))
			return false;
		bytes_sent += 256;
	}

	return true;
}


const double Programmer::getTimeOfDay(void) const
{
	struct timeval tv;

	if(::gettimeofday(&tv, NULL) == -1)
		throw SysException();

	return (double) tv.tv_sec + ((double) tv.tv_usec / 1000000.0);
}

