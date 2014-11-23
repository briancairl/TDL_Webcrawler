


//#include "netxmlbot.hpp"
//#include "netBuffer.hpp"
//#include "netGraph.hpp"
#include <list>
#include <string>
#include <chrono>
#include <iostream>
#include <fstream>
#include "conio.h"

using namespace std::chrono;
high_resolution_clock::time_point t1;

#include "wcCache.hpp"
#include "wcGlobal.hpp"
#include "wcSession.hpp"

#define	ROOT_TO_HOME(name)	"C:\\Users\\Brian-Home\\Documents\\"name
#define	ROOT_TO_RUNN(name)	"C:\\Users\\Brian\\Documents\\"name
void main()
{

	wcGlobal::init(ROOT_TO_HOME("cache_IE6"));
	
	wcSession session("test3","http://en.wikipedia.org/wiki/Jesus",0.1,5000,"matrix","eigenvalue","gradient");

	while(session.step(500));

	return;
}