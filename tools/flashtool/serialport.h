#ifndef __SERIALPORT_H__
#define __SERIALPORT_H__
//
// Serial port interface class
// 
// Part of flashtool
//
// (c) Stuart Wallace, 2011.
//

extern "C"
{
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>
};

#include <string>
#include <cstring>
#include <cstdint>
#include <cerrno>

#include "exception.h"


enum parity
{
	parityNone = 0,
	parityOdd,
	parityEven
};

#define DEFAULT_BAUD_RATE	 	(115200)
#define DEFAULT_WORD_SIZE		(8)
#define DEFAULT_PARITY			(parityNone)
#define DEFAULT_ASYNC_TIMEOUT	(3.0)			// seconds

#define ASYNC_SLEEP_INTERVAL	(1000)			// microseconds


void sigIoHandler(int status);

class SerialPort
{
public:
							SerialPort(const string device);
	virtual					~SerialPort();

	void					setBaudRate(const int rate);
	const int				getBaudRate(void) const;

	void					setWordSize(const int size);
	const int				getWordSize(void) const;

	void					setStopBits(const int bits);
	const int				getStopBits(void);

	void					setParity(const enum parity p);
	const enum parity		getParity(void) const;
	// TODO: flow control

	void					setAsyncTimeout(const double t);
	const double			getAsyncTimeout(void) const;

	void					setNonBlocking(void);
	void					setBlocking(void);

	const int				write(const char *what);
	const int				write(const uint8_t *what, const size_t len);
	
	const ssize_t			read(void *buf, const size_t len);
	const ssize_t			asyncRead(void *buf, const size_t len);

	void					drain(void);

protected:
	const int				baudConstToInt(const int B) const;
	const int				baudIntToConst(const int b) const;

	const int				sizeConstToInt(const int S) const;
	const int				sizeIntToConst(const int s) const;

	const int				stopConstToInt(const int S) const;
	const int				stopIntToConst(const int s) const;

	void					setAttr(void);

	const double			timeOfDay(void) const;

	int						m_fd;

	int						m_baudRate;
	int						m_wordSize;
	enum parity				m_parity;

	double					m_asyncTimeout;

	static bool				m_sigHandlerInstalled;
	static volatile bool	m_sigIoReceived;

	struct termios			m_termOld;
	struct termios			m_term;

	friend void sigIoHandler(int);
};


class SerialPortException : public Exception
{
public:
	SerialPortException(const char *what)
		: Exception(what) { }
};

#endif

