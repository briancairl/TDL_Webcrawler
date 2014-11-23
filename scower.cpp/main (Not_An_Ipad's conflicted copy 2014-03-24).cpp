


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
#include "wcAnalysis.hpp"
#include "wcGraph.hpp"

/*


class netdictionary
{
#define NETDICT_ROOT			"dictionary.reference.com/browse/"			// put this in source. not to be configurable 
#define NETDICT_VER				1											// version number 
#define NETDICT_FULL_URL(url)	(std::string(NETDICT_ROOT)+url).c_str()		// URL relative to root. param "p" is a word to search
public:

	typedef struct 
	{
		std::string				part_of_speech;
		std::string				definition;
	} pull_content;

	typedef std::list<pull_content> pull_list;

	netdictionary( const char* word );
	~netdictionary();

private:
	
	ns::netstream				source;
	pull_list					results;

	bool						pull();
	bool						valid;
};




netdictionary::netdictionary( const char* word ) :
	source(NETDICT_FULL_URL(word))
{
	valid = pull();
}



netdictionary::~netdictionary()
{
}



bool netdictionary::pull()
{
#if	NETDICT_VER == 1
	high_resolution_clock::time_point t1(high_resolution_clock::now());
	netxmlbot::netxmlbot *lockbot, *posbot, *defbot;
	while(!source.eof())
	{
		lockbot = new netxmlbot::netxmlbot(source,NETXMLBOT_INHERIT_OFFSET,"div",NETXMLBOT_PARAM_PAIR("class","pbk"));
		std::streamoff offset = 0;
		if(lockbot->good())
		{	
			posbot = new netxmlbot::netxmlbot(source,NETXMLBOT_INHERIT_OFFSET,*lockbot,"span",NETXMLBOT_PARAM_PAIR("class","pg"));
			if(posbot->good())
			{
				results.emplace_back();
				posbot->get_content(source,results.back().part_of_speech);
				//std::cout<<results.back().part_of_speech<<std::endl;
				for(;;)
				{	
					defbot = new netxmlbot::netxmlbot(source,source.tellg(),*lockbot,"div",NETXMLBOT_PARAM_PAIR("class","dndata"));
					if(defbot->good())
					{
						results.back().definition.clear();
						defbot->get_content(source,results.back().definition);
						delete defbot;
					}
					else
					{
						delete defbot;
						break;
					}
				}
			}
			delete posbot;
		}
		delete lockbot;
	}
	std::cout<< duration_cast<milliseconds>(high_resolution_clock::now()-t1).count();
#endif
	return false;
}

*/




void main()
{

	//netPrimitives::globalInit("C:\\Users\\Brian-Home\\cache");
	/*
	netPrimitives::globalInit("C:\\users\\brian\\cache");

	netTypes::netStringCell topics;

	topics.emplace_back("trigonometry");

	netTypes::netSize  step(0);
	netGraph::netNode* nodes[2] =
	{
		new netGraph::netNode("http://en.wikipedia.org/wiki/Math"),
		NULL
	};


	while(1)
	{

		std::cout << "[#][Step:"<<step<<"]"<<std::endl;
		
		// Resolution Check
		while(!nodes[(step)%2]->ready())
		{
			if(nodes[(step)%2]->timedout())
			{
				delete nodes[(step)%2];
				std::cout << "[#][Timeout Failure!]"<<std::endl;
				_getch();
				return;
			}
		}

		// Harvest new nodes
		nodes[(step)%2]->crawl(topics);
		
		if(nodes[(step)%2]->get_next())
		{
			// New Active Node
			nodes[(step+1)%2] = new netGraph::netNode(nodes[(step)%2]->get_next());
				
			// Delete Last Block of Memory
			delete nodes[(step)%2];
				
			// Step Foward
			++step;

		}	
		else
		{
			delete nodes[(step)%2];
			std::cout << "[#][End of the road... D':]"<<std::endl;
			break;
		}
	}
	*/


	wcGlobal::init("C:\\Users\\Brian\\Desktop\\cache");
	
	
	wcStringCell	query_vector;
	query_vector.emplace_back("math");
	query_vector.emplace_back("science");
	query_vector.emplace_back("engineering");

	wcGraph			graph("math","http://en.wikipedia.org/wiki/math",query_vector);
//	wcGraph			graph("math");

	wcSize			size = 0;
	while( (graph.step(query_vector,wcMASK(2),100) == wcNoErr) && (size<100UL) )
	{
		++size; std::cout<<"[STEP] - "<<size<<std::endl;
	}

	_getch();
	return;
}