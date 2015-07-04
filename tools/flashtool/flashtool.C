//
// Flashtool application class
// 
// Part of flashtool
//
// (c) Stuart Wallace, 2011.
//

#include "flashtool.h"


struct parameter g_params[] =
{
//	 						arg?	verb?
	{"device",				true,	false},

	{"read",				false,	true},
	{"identify",			false,	true},
	{"program",				false,	true},
	{"erase",				false,	true},
	{"help",				false,	true},

	{"offset",				true,	false},
	{"length",				true,	false},
	{"infile",				true,	false},
	{"outfile",				true,	false},
	{"verbose",				false,	false},
	{"verify",				false,	false},

	{NULL,					false,	false}
};


const char * const g_pHelpString =
	"Commands\n"
	"\n"
	"    --program              Program the specified file into the ROM.\n"
	"        --infile <file>    Programming will begin at offset 0 unless the\n"
	"        [--offset <num>]   the '--offset' arg is supplied.  Length may be\n"
	"        [--length <num>]   constrained with the '--length' arg. The\n"
	"        [--verify]         '--verify' option adds a verification step\n"
	"\n"
	"    --read                 Read the contents of the ROM.  If the\n"
	"        [--outfile <file>] '--outfile' arg is present, write data to the\n"
	"        [--offset <num>]   specified file.  The '--offset' and '--length'\n"
	"        [--length <num>]   args permit the read range to be constrained.\n"
	"\n"   
	"    --identify             Read ROM chip identity data\n"
	"\n"
	"    --erase                Erase ROM chip.\n"
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
	"Stuart Wallace <stuartw@atom.net>, June 2011.\n";


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
					cerr << "Looking for programmer at " << *it << endl;
				try
				{
					Programmer p(*it);
					if(p.identifyProgrammer())
					{
						if(args.has("verbose"))
							cerr << "Found programmer at " << *it << endl;
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
			Programmer p(args("device"));
	
			if(p.identifyProgrammer())
				device = args("device");
		}
	
		if(device.empty())
			throw AppException("Programmer not found");

		//
		// Command handlers
		//
		Programmer p(device);
		if(args.verb() == "identify")
		{
			uint8_t manufacturer_id, device_id, continuation_id;
			if(!p.identifyROM(manufacturer_id, device_id, continuation_id))
				throw AppException("identify operation failed");

			cout << "Manufacturer ID 0x" << setfill('0') << setw(2) << hex << (int) manufacturer_id << endl
				 << "Device ID       0x" << (int) device_id << endl
				 << "Continuation ID 0x" << (int) continuation_id << endl;
		}
		else if(args.verb() == "read")
		{
			uint32_t offset = 0, length = 524288;

			if(args.has("offset"))
			{
				if(!args.isNumeric("offset"))
					throw ArgException("Invalid offset");

				offset = args.getUnsigned("offset");
			}

			if(args.has("length"))
			{
				if(!args.isNumeric("length"))
					throw ArgException("Invalid length");

				length = args.getUnsigned("length");
			}

			if(!length)
				throw ArgException("Zero length specified");

			cerr << "Reading " << length << " byte(s) from offset " << offset << endl;
			const uint8_t *data = p.read(offset, length);
			if(data)
			{
				if(args.has("outfile"))
				{
					if(args.has("verbose"))
						cerr << "Writing data to '" << args("outfile") << "'" << endl;

					ofstream outFile(args("outfile"));
					if(!outFile.good())
						throw AppException("Cannot open output file");

					outFile.write((const char *) data, length);
					outFile.flush();
					if(outFile.bad())
						throw AppException("Write failed");
				}
				else
				{
					if(args.has("verbose"))
						cerr << "Writing data to stdout" << endl;
						
					cout.write((const char *) data, length);
					cout.flush();
					if(cout.bad())
						throw AppException("Write failed");
				}
			}
			else
				throw AppException("Data read failed");

			delete[] data;
		}
		else if(args.verb() == "erase")
		{
			if(!p.eraseROM())
				throw AppException("Chip erase failed");
		}
		else if(args.verb() == "program")
		{
			uint32_t offset = 0, length = 0;

			if(!args.has("infile"))
				throw ArgException("Missing 'infile' argument");

			if(args.has("offset"))
			{
				if(!args.isNumeric("offset"))
					throw ArgException("Invalid offset");

				offset = args.getUnsigned("offset");
			}

			struct stat inFile;
			if(::stat(args("infile").c_str(), &inFile))
				throw SysException(args("infile"));

			if(args.has("length"))
			{
				if(!args.isNumeric("length"))
					throw ArgException("Invalid length");

				length = args.getUnsigned("length");
			
				if(length > (uint32_t) inFile.st_size)
					throw ArgException("Insufficient data in input file");
			}
			else length = inFile.st_size;

			uint8_t *data = new uint8_t[length];

			ifstream ifData(args("infile"));
			if(ifData.good())
			{
				ifData.read((char *) data, length);

				if(ifData.fail() || ifData.eof() || ((uint32_t) ifData.gcount() != length))
					throw AppException("Incomplete read occurred");

				if(args.has("verbose"))
					cerr << "Writing " << length << " bytes from file '" << args("infile") << "' to offset " << offset << endl;

				if(!p.write(data, offset, length))
					throw AppException("Data write failed");

				if(args.has("verify"))
				{
					// Verify programming operation
					const uint8_t *verifyData = p.read(offset, length);
					if(verifyData)
					{
						if(::memcmp(data, verifyData, length))
							throw AppException("Verify failed");
						else if(args.has("verbose"))
							cerr << "Data verified" << endl;
					}
					else
						throw AppException("Data read failed");

				}
			}
			else throw AppException("Failed to open input file");
		}
		else throw AppException("Unimplemented command");

		return 0;
	}
	catch(ArgException e)
	{
		cerr << progname << ": " << e.what() << endl;
		return 1;
	}
	catch(SysException e)
	{
		cerr << "SysException: " << e.what() << endl;
		return 2;
	}
	catch(AppException e)
	{
		cerr << "AppException: " << e.what() << endl;
		return 3;
	}
}

