#ifndef		NETTYPEDEFS_HPP
#define		NETTYPEDEFS_HPP

#include	<cstdio>
#include	<cctype>
#include	<string>
#include	<list>
#include	<vector>
#include	<thread>
#include	<algorithm>
#include	<direct.h>
#include	<curl\curl.h>

namespace	netTypes
{
	typedef	std::string				netURL;
	typedef	std::list<netURL>		netURLlist;
	typedef	const char*				netPath;
	typedef	char*					netData;
	typedef	std::string				netString;
	typedef	std::list<netString>	netStringCell;
	typedef	unsigned long			netSize;
	typedef	unsigned long			netPos;
	typedef	const netSize			netOffset;
	typedef	unsigned int 			netFlags;
	typedef std::thread				netThread;
	typedef float					netFloat;
}
#endif