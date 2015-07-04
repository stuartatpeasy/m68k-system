#ifndef _FLASHTOOL_H_
#define _FLASHTOOL_H_
//
// Flashtool application class
// 
// Part of flashtool
//
// (c) Stuart Wallace, 2011.
//

#include <iostream>
#include <fstream>
#include <list>
#include <iomanip>

#include <cstdio>
#include <cstring>
#include <cstdint>

extern "C"
{
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
};

#include "args.h"
#include "programmer.h"
#include "exception.h"

#define DEVICE_DIR			"/dev"

using namespace std;


list<string> findSerialDevices(void);


class AppException : public Exception
{
public:
			AppException(const string& what)
				: Exception(what) { };
};

#endif

