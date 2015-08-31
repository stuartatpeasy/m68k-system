//
// Serial port interface/driver for running a DFU operation on an m68k-system board
//
// Part of dfu
//
// (c) Stuart Wallace, 2015.
//

#include "target.h"

using std::cout;
using std::endl;
using std::flush;

Target::Target(const string device)
	: m_device(device), m_port(device)
{ }


Target::~Target()
{ }


const bool Target::doCommand(const char *cmd, const ssize_t cmdlength, char *response, ssize_t responselength)
{
	m_port.write((const uint8_t *) cmd, cmdlength);
	m_port.drain();

	m_port.setNonBlocking();

    int offset = 0;
    double rxWaitStart = getTimeOfDay();
	while(offset < responselength)
    {
        char buf[32];
        ssize_t nread = m_port.read(buf, sizeof(buf));

		if((rxWaitStart + TARGET_DEFAULT_RX_TIMEOUT) < getTimeOfDay())
			throw ProgrammerException("Command timeout");

        if(nread == -1)
        {
            if(errno == EAGAIN)
            {
                ::usleep(1000);
                continue;
            }
            else
            {
                m_port.setBlocking();
                return false;           // read failed
            }
        }

        ::memcpy(response + offset, buf, std::min<int>(responselength - offset, nread));
        offset += nread;
    }

	m_port.setBlocking();
	return true;
}


// Convenience variant of doCommand() - computes command length on the caller's behalf.
const bool Target::doCommand(const char *cmd, char *response, ssize_t responselength)
{
    return doCommand(cmd, strlen(cmd), response, responselength);
}


// Send a command; check that the response matches what was expected.
const bool Target::command(const char *cmd, const char *expectedResponse)
{
    char response[256];
    const size_t expectedResponseLen = ::strlen(expectedResponse);

    if(expectedResponseLen >= sizeof(response))
        return false;

//    printf("Sending: '%s'\n", cmd);
    if(!doCommand(cmd, response, expectedResponseLen))
        return false;
/*
    printf("Received:\n");
    for(int i = 0; i < 50; ++i)
        printf("%02x ", (unsigned char) response[i]);
    printf("\n");

    for(int i = 0; i < 50; ++i)
        printf("%c", isprint(response[i]) ? response[i] : '.');
    printf("\n");
*/
    return memcmp(response, expectedResponse, expectedResponseLen) == 0;
}


// identifyProgrammer() - returns true if a valid identity could be read from the device; false otherwise.
//
const bool Target::find(void)
{
    if(!command("\n", "\x0a$ "))
        return false;

    if(!command("echo Hello, World!\n", "echo Hello, World!\nHello, World!\n$ "))
        return false;

    // At this point we probably are talking to a suitable target.
    return true;
}


const bool Target::program(const uint8_t *buffer, const size_t len)
{
    char cmd[128], response[128];

    if(!command("serial echo off\n", "serial echo off\n$ "))
        throw ProgrammerException("Unable to set serial line discipline on target");

    sprintf(cmd, "dfu %u\n", (unsigned int) len);
    sprintf(response, "Send %u bytes", (unsigned int) len);
    if(!command(cmd, response))
        throw ProgrammerException("Failed to initiate DFU process on target");

    cout << "Sending " << len << " bytes: ";
    int blocksize = 1024;
    for(size_t offset = 0; offset < len; offset += blocksize)
    {
        if(offset + blocksize > len)
            blocksize = len - offset;

        if(m_port.write(buffer + offset, blocksize) != blocksize)
            throw ProgrammerException("Failed to send data to target - write() failed");

        cout << "." << flush;
    }
    cout << endl;

    return true;
}


const double Target::getTimeOfDay(void) const
{
	struct timeval tv;

	if(::gettimeofday(&tv, NULL) == -1)
		throw SysException();

	return (double) tv.tv_sec + ((double) tv.tv_usec / 1000000.0);
}


SerialPort& Target::port(void)
{
    return m_port;
}
