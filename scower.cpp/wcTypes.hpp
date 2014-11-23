#ifndef		WC_TYPES_HPP
#define		WC_TYPES_HPP

#include	<list>
#include	<thread>
#include	<string>
#include	<vector>
#include	<cmath>
#include	<chrono>
#include	<eigen/Dense>

typedef	std::string							wcURL;
typedef	const char*							wcPath;
typedef	char*								wcBuffer;
typedef	std::string							wcString;
typedef	std::list<wcString>					wcStringCell;
typedef	unsigned long						wcSize;
typedef	unsigned long						wcPos;
typedef	unsigned int 						wcFlags;
typedef std::thread							wcThread;
typedef float								wcFloat;
typedef	std::pair<wcPos,wcPos>				wcRange;
typedef	std::vector<wcRange>				wcRangeVector;

typedef Eigen::Vector3f						wcMetaVector;

typedef std::chrono::high_resolution_clock	wcClock;
typedef	wcClock::time_point					wcTime;

#endif
