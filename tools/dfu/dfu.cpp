//
// DFU - Device Firmware Updater for m68k-system
//
//
// (c) Stuart Wallace, 2015.
//

#include "dfu.h"


struct parameter g_params[] =
{
//	 						arg?	verb?
	{"device",				true,	false},

	{"program",				false,	true},
	{"help",				false,	true},

	{"infile",				true,	false},
	{"verbose",				false,	false},

	{NULL,					false,	false}
};


const char * const g_pHelpString =
	"Commands\n"
	"\n"
	"    --program              Program the specified file into the ROM.\n"
	"        --infile <file>    \n"
	"\n"
	"    --help                 Display this text.\n"
	"\n"
	"Optional arguments for all commands\n"
	"\n"
	"    --device <device>      Connect to the programmer on device <device>\n"
	"                           (e.g. \"/dev/ttyS0\").\n"
	"\n"
	"    --verbose              Be verbose.\n"
	"\n"
	"\n"
	"Stuart Wallace <stuartw@atom.net>, June 2015.\n";


list<string> findSerialDevices()
{
	DIR *pDev;
	struct dirent *pEntry;
	list<string> entries;

	if((pDev = ::opendir(DEVICE_DIR)) == NULL)
		throw SysException();

	int cur_errno = errno;

	while(1)
	{
		pEntry = ::readdir(pDev);
		if(pEntry == NULL)
		{
			if(errno != cur_errno)
				throw SysException();
			break;
		}

		if((pEntry->d_name != NULL) && (::strstr(pEntry->d_name, "ttyS") == pEntry->d_name))
			entries.push_back(string(DEVICE_DIR) + "/" + pEntry->d_name);
	}

	if(::closedir(pDev))
		throw SysException();

	entries.sort();

	return entries;
}


int main(int argc, char **argv)
{
	string progname = *argv;
	try
	{
		Args args(g_params);
		args.parse(argc, argv);

		string device;
		strmap::const_iterator itDevice;

		if(args.verb() == "help")
		{
			cerr << "Syntax: " << progname << " <command> [<option> ...]\n\n" << g_pHelpString << endl;
			return 0;
		}

		if(!args.has("device"))
		{
			list<string> devices = findSerialDevices();
			for(list<string>::const_iterator it = devices.begin(); it != devices.end(); ++it)
			{
				if(args.has("verbose"))
					cerr << "Looking for target on " << *it << endl;
				try
				{
					Target p(*it);
					if(p.find())
					{
						if(args.has("verbose"))
							cerr << "Found target on " << *it << endl;
						device = *it;
						break;
					}
				}
				catch(Exception e) { /* swallow exceptions during programmer detection */ }
			}
		}
		else
		{
			if(args.has("verbose"))
				cerr << "Using device " << args("device") << endl;
			Target t(args("device"));

			if(t.find())
				device = args("device");
		}

		if(device.empty())
			throw AppException("Programmer not found");

        if(args.verb() != "program")
            throw AppException("No operation specified");

        if(!args.has("infile"))
            throw AppException("No input file specified");

        const string fn = args("infile");
        struct stat stat_buf;

        if(::stat(fn.c_str(), &stat_buf) == -1)
            throw new AppException("Cannot stat() input file");

        uint8_t *buf = new uint8_t[stat_buf.st_size];

        FILE *fp = ::fopen(fn.c_str(), "r");
        if(!fp)
            throw new AppException("Cannot open input file");

        if(::fread(buf, stat_buf.st_size, 1, fp) <= 0)
            throw new AppException("Failed to read input file");

        Target t(device);

        t.program(buf, stat_buf.st_size);

        ::fclose(fp);
        delete[] buf;

		return 0;
	}
	catch(ArgException e)
	{
		cerr << progname << ": " << e.what() << endl;
		return 1;
	}
	catch(SysException e)
	{
		cerr << progname << ": " << e.what() << endl;
		return 2;
	}
	catch(AppException e)
	{
		cerr << progname << ": " << e.what() << endl;
		return 3;
	}
}

