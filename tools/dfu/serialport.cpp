#include "serialport.h"
//
// Serial port interface class
//
// Part of flashtool
//
// (c) Stuart Wallace, 2011.
//

bool SerialPort::m_sigHandlerInstalled = false;
volatile bool SerialPort::m_sigIoReceived = false;


SerialPort::SerialPort(const string device)
	: m_fd(0), m_baudRate(0), m_wordSize(0), m_asyncTimeout(DEFAULT_ASYNC_TIMEOUT)
{
	if((m_fd = ::open(device.c_str(), O_RDWR | O_NOCTTY)) < 0)
		throw SysException();

	::bzero(&m_term, sizeof(m_term));

	::tcgetattr(m_fd, &m_termOld);

	setBaudRate(DEFAULT_BAUD_RATE);
	setWordSize(DEFAULT_WORD_SIZE);
	setParity(DEFAULT_PARITY);

	m_term.c_cflag |= CLOCAL | CREAD | CRTSCTS;
	m_term.c_iflag = IGNPAR;
//	m_term.c_oflag = OPOST;
	m_term.c_oflag = 0;
	m_term.c_lflag = 0;

	m_term.c_cc[VTIME] = 0;
	m_term.c_cc[VMIN] = 1;

	setAttr();


	if(::fcntl(m_fd, F_SETOWN, ::getpid()) == -1)
		throw SysException();
}


SerialPort::~SerialPort()
{
	::tcsetattr(m_fd, TCSANOW, &m_termOld);
	::close(m_fd);
}


void SerialPort::setBaudRate(const int rate)
{
	::cfsetispeed(&m_term, baudIntToConst(rate));
	::cfsetospeed(&m_term, baudIntToConst(rate));

	setAttr();

	m_baudRate = rate;
}


const int SerialPort::getBaudRate(void) const
{
	return m_baudRate;
}


void SerialPort::setWordSize(const int size)
{
	m_term.c_cflag &= ~CSIZE;
	m_term.c_cflag |= sizeIntToConst(size);

	setAttr();

	m_wordSize = size;
}


const int SerialPort::getWordSize(void) const
{
	return m_wordSize;
}


void SerialPort::setParity(const enum parity p)
{
	m_term.c_cflag &= ~(PARENB | PARODD);
	switch(p)
	{
		case parityNone:
			break;

		case parityOdd:
			m_term.c_cflag |= PARODD;
		case parityEven:
			m_term.c_cflag |= PARENB;
			break;

		default:
			throw SerialPortException("Invalid parity specification");
	}

	setAttr();

	m_parity = p;
}


const enum parity SerialPort::getParity(void) const
{
	return m_parity;
}


void SerialPort::setAsyncTimeout(const double t)
{
	if(t < 0)
		throw SerialPortException("Cannot set negative timeout value");

	m_asyncTimeout = t;
}


const double SerialPort::getAsyncTimeout(void) const
{
	return m_asyncTimeout;
}


void SerialPort::setNonBlocking(void)
{
	int opts;
	if((opts = ::fcntl(m_fd, F_GETFL, 0)) == -1)
		throw SysException();

	if(::fcntl(m_fd, F_SETFL, opts | O_NONBLOCK))
		throw SysException();
}


void SerialPort::setBlocking(void)
{
	int opts;
	if((opts = ::fcntl(m_fd, F_GETFL, 0)) == -1)
		throw SysException();

	if(::fcntl(m_fd, F_SETFL, opts & ~O_NONBLOCK))
		throw SysException();
}


const int SerialPort::write(const char *what)
{
	return ::write(m_fd, what, ::strlen(what));
}


const int SerialPort::write(const uint8_t *what, const size_t len)
{
	return ::write(m_fd, what, len);
}


const ssize_t SerialPort::read(void *buf, const size_t len)
{
	return ::read(m_fd, buf, len);
}


const ssize_t SerialPort::asyncRead(void *buf, const size_t len)
{
	if(m_sigHandlerInstalled)
		throw SerialPortException("Cannot do async read on multiple ports");
	m_sigHandlerInstalled = true;

	// install signal handler
	struct sigaction sa;

	sa.sa_handler	= sigIoHandler;
	sa.sa_flags		= 0;
	sa.sa_restorer	= NULL;

	::sigemptyset(&sa.sa_mask);
	if(::sigaction(SIGIO, &sa, NULL))
		throw SysException();

	int flags;
	if((flags = ::fcntl(m_fd, F_GETFL, 0)) == -1)
		throw SysException();

	// Set asynchronous mode
	flags |= FASYNC;
	if(::fcntl(m_fd, F_SETFL, flags) == -1)
		throw SysException();

	double timeout_time = timeOfDay() + m_asyncTimeout;
	bool timeout = false;
	int res;
	for(;;)
	{
		if(m_sigIoReceived)
		{
			res = ::read(m_fd, buf, len - 1);
			m_sigIoReceived = false;
			break;
		}
		else
		{
			::usleep(ASYNC_SLEEP_INTERVAL);
			if(timeOfDay() > timeout_time)
			{
				timeout = true;
				res = 0;
				break;
			}
		}
	}
	((char *) buf)[res] = '\0';

	// Unset asynchronous mode
	flags &= ~FASYNC;
	if(::fcntl(m_fd, F_SETFL, FASYNC) == -1)
		throw SysException();

	// Uninstall signal handler
	sa.sa_handler = SIG_DFL;
	if(::sigaction(SIGIO, &sa, NULL))
		throw SysException();

	m_sigHandlerInstalled = false;
	return res;
}


void SerialPort::drain(void)
{
	if(::tcdrain(m_fd))
		throw SysException();
}


// baudConstToInt() - convert a baud-rate constant value (as defined in asm/termbits.h), e.g. B9600, into
//                    an integer, e.g. 9600.
//
const int SerialPort::baudConstToInt(const int B) const
{
	switch(B)
	{
		case B0:		return 0;
		case B50:		return 50;
		case B75:		return 75;
		case B110:		return 110;
		case B134:		return 134;
		case B150:		return 150;
		case B200:		return 200;
		case B300:		return 300;
		case B600:		return 600;
		case B1200:		return 1200;
		case B1800:		return 1800;
		case B2400:		return 2400;
		case B4800:		return 4800;
		case B9600:		return 9600;
		case B19200:	return 19200;
		case B38400:	return 38400;
		case B57600:	return 57600;
		case B115200:	return 115200;
		case B230400:	return 230400;
		case B460800:	return 460800;
		case B500000:	return 500000;
		case B576000:	return 576000;
		case B921600:	return 921600;
		case B1000000:	return 1000000;
		case B1152000:	return 1152000;
		case B1500000:	return 1500000;
		case B2000000:	return 2000000;
		case B2500000:	return 2500000;
		case B3000000:	return 3000000;
		case B3500000:	return 3500000;
		case B4000000:	return 4000000;
		default:
			throw new SerialPortException("Invalid baud rate");
	}
}


// baudIntToConst() - convert an integer baud rate, e.g. 9600, into the corresponding constant from asm/termbit.h,
//                    e.g. B9600.
//
const int SerialPort::baudIntToConst(const int b) const
{
	switch(b)
	{
		case 0:			return B0;
		case 50:		return B50;
		case 75:		return B75;
		case 110:		return B110;
		case 134:		return B134;
		case 150:		return B150;
		case 200:		return B200;
		case 300:		return B300;
		case 600:		return B600;
		case 1200:		return B1200;
		case 1800:		return B1800;
		case 2400:		return B2400;
		case 4800:		return B4800;
		case 9600:		return B9600;
		case 19200:		return B19200;
		case 38400:		return B38400;
		case 57600:		return B57600;
		case 115200:	return B115200;
		case 230400:	return B230400;
		case 460800:	return B460800;
		case 500000:	return B500000;
		case 576000:	return B576000;
		case 921600:	return B921600;
		case 1000000:	return B1000000;
		case 1152000:	return B1152000;
		case 1500000:	return B1500000;
		case 2000000:	return B2000000;
		case 2500000:	return B2500000;
		case 3000000:	return B3000000;
		case 3500000:	return B3500000;
		case 4000000:	return B4000000;
		default:
			throw SerialPortException("Invalid baud rate");
	}
}


// sizeConstToInt() - convert a constant word size as specified in asm/termbits.h, e.g. CS8, into the
//                    corresponding integer, e.g. 8.
//
const int SerialPort::sizeConstToInt(const int s) const
{
	switch(s)
	{
		case CS5:	return 5;
		case CS6:	return 6;
		case CS7:	return 7;
		case CS8:	return 8;
		default:
			throw SerialPortException("Invalid word size constant");
	}
}


// sizeIntToConst() - convert an integer word size, e.g. 8, into the corresponding constant defined in
//                    asm/termbits.h, e.g. CS8.
//
const int SerialPort::sizeIntToConst(const int S) const
{
	switch(S)
	{
		case 5:	return CS5;
		case 6:	return CS6;
		case 7:	return CS7;
		case 8:	return CS8;
		default:
			throw SerialPortException("Invalid word size");
	}

}


// stopConstToInt() - convert a constant specifying the number of stop bits, as defined in asm/termbits.h, e.g.
//                    CSTOPB, to the corresponding integer number of stop bits, e.g. 2.
//
const int SerialPort::stopConstToInt(const int S) const
{
	switch(S)
	{
		case 0:			return 1;
		case CSTOPB:	return 2;
		default:
			throw SerialPortException("Invalid stop bit constant");
	}
}


// stopIntToConst() - convert an integer number of stop bits, e.g. 2, to the corresponding constant as defined in
//                    asm/termbits.h, e.g. CSTOPB.
//
const int SerialPort::stopIntToConst(const int s) const
{
	switch(s)
	{
		case 1:			return 0;
		case 2:			return CSTOPB;
		default:
			throw SerialPortException("Invalid number of stop bits");
	}
}


void SerialPort::setAttr(void)
{
	if(::tcflush(m_fd, TCIFLUSH))
		throw SysException();

	if(::tcsetattr(m_fd, TCSANOW, &m_term))
		throw SysException();
}


const double SerialPort::timeOfDay(void) const
{
	struct timeval tv;
	if(::gettimeofday(&tv, NULL))
		throw SysException();

	return ((double) tv.tv_usec / 1000000.0) + tv.tv_sec;
}


void sigIoHandler(int status)
{
	SerialPort::m_sigIoReceived = true;
}
