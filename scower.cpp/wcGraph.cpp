#include	"wcGraph.hpp"
#include	"wcSession.hpp"
#include	<mutex>



static	wcmMonitorList					wcm_procmonitor;
static 	wcFloat							optimality_treshold = WC_SUBOPTIMALITY_SATURATION;

namespace wcm
{



	bool	Prune( wcmMonitor& mon ) 
	{
		if(isRemovable(*mon.agent)) 
		{ 
			wcDELETE(mon.thread); return true; 
		}
		return false;
	}



	void	Cleanup()
	{
		wcm_procmonitor.remove_if(Prune);
		while(wcm_procmonitor.size() >= WC_GRAPH_DISPATCH_LIMIT)
			wcm_procmonitor.remove_if(Prune);
	}



	void	Terminate()
	{
		while(!wcm_procmonitor.empty() )
			wcm_procmonitor.remove_if(Prune);
	}



	void	Dispatch( wcAgent* agent, wcSession* session, wcStringCell* query_vector )
	{
		if( agent->dispatch_needed && !agent->dispatch_done)
		{
			wcm_procmonitor.emplace_back();
			wcm_procmonitor.back().agent  = agent;
			wcm_procmonitor.back().thread = new wcThread(&wcAgent::perform,agent,session,query_vector);
			wcm_procmonitor.back().thread->detach();	
		}
	}
}





//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// wcAgent
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


std::ostream&	operator<<(std::ostream& os, wcAgent& node )
{
	/// Tagging Info
	os<<node.tag		<<std::endl;

	/// Meta Data
	os<<node.meta[0]	<<std::endl;
	os<<node.meta[1]	<<std::endl;
	os<<node.meta[2]	<<std::endl;

	/// Links
	os<< node.links.size() << std::endl;
	for( wcStringCell::const_iterator pl_itr = node.links.cbegin(); pl_itr!= node.links.cend(); ++pl_itr )
		os<<(*pl_itr)<<std::endl;

	return os;
}



std::istream&	operator>>(std::istream& is, wcAgent& node )
{
	/// Tagging Info
	is>>node.tag;

	/// Meta Data
	is>>node.meta[0];
	is>>node.meta[1];
	is>>node.meta[2];

	/// Link Data
	wcSize nfeed; is>>nfeed;
	if(nfeed > 5000UL)
		return is;

	while(nfeed--)
	{
		node.links.emplace_back();

		is>>node.links.back();
	}
	return is;
}




wcAgent::wcAgent( const wcURL& url ) :
	tag(url), 
	dispatch_needed(true),
	dispatch_done(false),
	meta(0,0,0),cost(0)
{
	std::ifstream map_in(tag.get_map_filename());
	if( map_in.is_open() )
	{
		dispatch_needed	= false;
		dispatch_done   = true;
		map_in>>*this;
		map_in.close();
	}
}





wcAgent::~wcAgent()
{	
}



void wcAgent::create_map()
{
	std::ofstream map_out(tag.get_map_filename());
	if( map_out.is_open() )
	{
		map_out << *this;
		map_out.close();
	}
}


void wcAgent::perform( wcSession* session, wcStringCell* query_vector )
{
	if(dispatch_needed&&!dispatch_done)
	{
		wcCache			cache(tag);
		wcPageAnalyzer	analyzer(*session,&cache,*query_vector,meta,links);	
		
		create_map();
	} 
	dispatch_done	= true;
}


bool						isSuboptimal( wcAgent& agent )					{	return agent.cost > optimality_treshold; }
bool						isRemovable( wcAgent& agent )					{	return agent.dispatch_done;  }
void						updateOptimalityThreadhold( wcAgent& agent )	{	optimality_treshold+=(agent.cost *0.001f)*(agent.cost-optimality_treshold); }