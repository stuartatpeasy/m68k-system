#ifndef __ARGS_H__
#define __ARGS_H__
//
// Command-line argument-parsing code
// 
// Part of flashtool
//
// (c) Stuart Wallace, 2011.
//

#include <map>
#include <string>
#include <algorithm>
#include <cstring>
#include <cstdlib>

#include "exception.h"

#define ARGUMENT_PREFIX		"--"

using namespace std;


struct parameter
{
	const char *name;
	bool val_required;			// false = value must not be given; true = value must be given
	bool is_verb;				// this param is a "verb", i.e. exactly one must be specified
};

typedef map<string, string> strmap;


class Args
{
public:
								Args(const struct parameter *pVocab);
	virtual						~Args();

	void						parse(const int argc, char **argv);

	const string&				verb(void) const;
	const bool					has(const string& arg) const;
	const bool					isNumeric(const string& arg);
	const unsigned long			getUnsigned(const string& arg);

	const string				operator()(const string& arg);

protected:
	string						m_verb;
	strmap						m_args;

	const struct parameter *	m_pVocab;
};


class ArgException : public Exception
{
public:
			ArgException(const string& what)
				: Exception(what) { };
};

#endif

