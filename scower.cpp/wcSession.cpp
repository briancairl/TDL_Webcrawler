#include "wcSession.hpp"
#include <cmath>


bool wcValidLink( wcString& link )
{
	if(		(link.size()<4UL)
	||		(link.size()>200UL)
	||		(link.find("http:") != 0UL)
	||		(link.rfind(".png")	<link.max_size())
	||		(link.rfind(".svg")	<link.max_size())
	||		(link.rfind(".ico")	<link.max_size())
	||		(link.rfind(".php")	<link.max_size())
	||		(link.rfind(".txt")	<link.max_size())
	||		(link.rfind(".csv")	<link.max_size())
	||		(link.rfind(".css")	<link.max_size())
	||		(link.rfind(".xml")	<link.max_size())
	||		(link.rfind("#")	<link.max_size())
	||		(link.rfind("&amp")	<link.max_size())
	){
		return false;
	}
	return true;
}



std::ostream& operator<<( std::ostream& os, wcPathPoint& path_pt )
{
	os<<path_pt.url			<<'\t'	;
	os<<path_pt.time		<<'\t'	;
	os<<path_pt.leading_cost<<'\t'	;
	os<<path_pt.relevance	<<'\t'	;
	os<<path_pt.RMSE		<<'\t'	;
	os<<path_pt.Vt					;
	return os;
}




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// wcSession
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////






wcSession::wcSession( 
	const wcString& name,
	const wcURL&	url, 
	const wcFloat&	discount,
	const wcSize&	max_steps,
	QV_T(1), 
	QV_T(2), 
	QV_T(3), 
	QV_T(4),
	QV_T(5), 
	QV_T(6), 
	QV_T(7), 
	QV_T(8)
) :
	session_name(name),
	cost_estimator(discount,1,1.0f,1),
	max_steps(max_steps),
	epoch(wcClock::now())
{
	QV_EVAL(1);
	QV_EVAL(2);
	QV_EVAL(3);
	QV_EVAL(4);
	QV_EVAL(5);
	QV_EVAL(6);
	QV_EVAL(7);
	QV_EVAL(8);
 

	/// Add First Agent
	agents.emplace_back(url);
	
	/// Dispatch the First Agent
	wcm::Dispatch(&agents.back(),this,&query_vector);

	/// Wait for the head process to terminate
	wcm::Terminate();
}


wcSession::wcSession( 
	const wcString&		name,
	const wcURL&		url, 
	const wcFloat&		lr,
	const wcSize&		max_steps,
	const wcStringCell& queries 
)
{
	///Copy Input Queries
	query_vector = queries;

	/// Add First Agent
	agents.emplace_back(url);
	
	/// Dispatch the First Agent
	wcm::Dispatch(&agents.back(),this,&query_vector);

	/// Wait for the head process to terminate
	wcm::Terminate();
}



wcSession::~wcSession()
{

}



void wcSession::log_point()
{
	if(!agents.empty())
	{
		explored_path.push_back(
			wcPathPoint(
				agents.front().tag.get_url(), 
				(wcClock::now()-epoch).count(), 
				agents.front().cost,
				agents.front().meta[1],
				cost_estimator.RMSE(),
				cost_estimator.VT()
			)	
		);
		wcGlobal::log_out<<explored_path.back()<<std::endl;
	}
}




bool wcSession::step( wcSize end_at )
{
	/// Check for an escape
	if(agents.empty()||(max_steps&&!(--max_steps)))	return false;


	/// Log last leader
	log_point();

	
	/// Dispatch from the leaders list of outbound links
	for( wcStringCell::iterator link = agents.front().links.begin(); link!=agents.front().links.end(); ++link)
	{
		if(--end_at)
		{
			if(wcValidLink(*link)&&unexplored(*link))
			{
				agents.emplace_back(link->c_str());
				wcm::Dispatch(&agents.back(),this,&query_vector);
			}
		}
		else
			break; 

		wcm::Cleanup();
	}


	/// Wait for all jobs to finish
	wcm::Terminate();
	
	
	/// Remove the leader
	agents.pop_front();


	/// Abort Condition (leader was the last surviving agent)
	if(agents.empty())
		return false;


	/// Estimate Costs
	for( wcAgentItr agent_itr = agents.begin(); agent_itr!=agents.cend(); ++agent_itr )
	{
		agent_itr->cost = cost_estimator.cost(agent_itr->meta);
	}


	/// Sort highest to front
	agents.sort();


	/// Update the estimator
	cost_estimator.update(agents.front().meta,0.1f);


	/// Update Pruning Threshold
	updateOptimalityThreadhold(agents.front());


	/// Prune Dead Nodes
	agents.remove_if(isSuboptimal);


	/// Exit if Agent-Queue is empty
	if(agents.empty())
		return false;


	std::cout<<"\n\n========================================================================"	<<std::endl;
	std::cout<<"Head is    :"<<agents.front().tag.get_url()										<<std::endl;
	std::cout<<"Head cost  :"<<agents.front().cost												<<std::endl;
	std::cout<<"Head meta  :"<<agents.front().meta.transpose()									<<std::endl;
	std::cout<<"Pool size  :"<<agents.size()													<<std::endl;
	std::cout<<"========================================================================\n\n"	<<std::endl;

	return true;
}




bool wcSession::unexplored( const wcString& url )
{
	for( wcStringCell::const_iterator uitr = explored_urls.begin(); uitr != explored_urls.end(); ++uitr )
	{
		if( url == *uitr )
			return false;
	}
	explored_urls.emplace_back(url);
	return true;
}

