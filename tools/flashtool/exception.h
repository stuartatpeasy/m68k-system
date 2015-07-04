#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__
//
// Various exception classes
// 
// Part of flashtool
//
// (c) Stuart Wallace, 2011.
//

#include <string>
#include <cstring>
#include <cerrno>

using namespace std;


//
// class Exception: generic exception abstraction
//
class Exception
{
protected:
						Exception()
							: m_code(0), m_what("(unknown)") { };

						Exception(const string& what)
							: m_code(0), m_what(what) { };

						Exception(const string& what, const int code)
							: m_code(code), m_what(what) { };

public:
		virtual			~Exception() { };

		const string	what(void) const { return m_what; };

protected:
	int					m_code;
	string				m_what;
};


//
// SysException: exception abstraction for system-level problems
//
class SysException : public Exception
{
public:
						SysException()
							: Exception(::strerror(errno), errno) { };

						SysException(const string& what)
							: Exception()
						{
							m_what = what + ": " + ::strerror(errno);
							m_code = errno;
						}
};


#endif

