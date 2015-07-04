//
// Command-line argument-parsing code
// 
// Part of flashtool
//
// (c) Stuart Wallace, 2011.
//

#include "args.h"


Args::Args(const struct parameter *pVocab)
	: m_pVocab(pVocab)
{

}


Args::~Args()
{

}


void Args::parse(const int argc, char **argv)
{
	string progname = *argv;
	while(*++argv)
	{
		if(::strstr(*argv, ARGUMENT_PREFIX) == *argv)
		{
			const struct parameter *pParam = m_pVocab;
			do
			{
				if(!::strcmp(pParam->name, *argv + ::strlen(ARGUMENT_PREFIX)))
				{
					if(pParam->is_verb)
					{
						if(!m_verb.empty())
							throw ArgException("Only one action may be specified");

						m_verb = pParam->name;
						break;
					}

					if(pParam->val_required)
					{
						if(!*++argv)
							throw ArgException(string("Option '") + *--argv + "' requires an argument");

						m_args[pParam->name] = *argv;
					}
					else m_args[pParam->name] = "";
					break;
				}
			} while((++pParam)->name);

			if(!pParam->name)
				throw ArgException(string("Unrecognised option '") + *argv + "'");
		}
		else
			throw ArgException(string("Syntax error near '") + *argv + "'");
	}

	if(m_verb.empty())
		throw ArgException("No action specified.  Try '--help'.");
}


const string& Args::verb(void) const
{
	return m_verb;
}


const bool Args::has(const string& arg) const
{
	return m_args.find(arg) != m_args.end();
}


const bool Args::isNumeric(const string& arg)
{
	if(!has(arg))
		return false;

	string val = m_args[arg];

	// decimal?
	if(val.find_first_not_of("0123456789") == string::npos)
		return true;

	// no.  maybe hex.
	transform(val.begin(), val.end(), val.begin(), ::tolower);
	return (val.find("0x") == 0) && (val.find_first_not_of("0123456789abcdef", 2) == string::npos);
}


const unsigned long Args::getUnsigned(const string& arg)
{
	if(has(arg))
	{
		string val = m_args[arg];

		if(val.find_first_not_of("0123456789") == string::npos)
			return ::strtol(val.c_str(), NULL, 10);
		
		transform(val.begin(), val.end(), val.begin(), ::tolower);
		if((val.find("0x") == 0) && (val.find_first_not_of("0123456789abcdef", 2) == string::npos))
			return ::strtol(val.c_str() + (2 * sizeof(char)), NULL, 16);
	}	
	return 0;
}


const string Args::operator()(const string& arg)
{
	if(has(arg))
		return m_args[arg];
	
	return "";
}
