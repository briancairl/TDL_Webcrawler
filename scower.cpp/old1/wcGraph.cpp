#include	"wcGraph.hpp"



bool wcAnchorFilter( const wcString& anchor )
{
return	!(anchor.rfind(".PNG")<anchor.max_size()) 
&&		!(anchor.rfind(".SVG")<anchor.max_size())
&&		!(anchor.rfind(".ICO")<anchor.max_size())
&&		!(anchor.rfind(".png")<anchor.max_size())
&&		!(anchor.rfind(".svg")<anchor.max_size())
&&		!(anchor.rfind(".ico")<anchor.max_size());
}



bool wcDispatchComplete( wcDispatch& dispatch )
{
	if(dispatch.first->dispatch_done)
	{
		#if WC_GRAPH_USE_DEBUG_TOOLS 
		std::cout<<"[D] : "<<dispatch.first->tag<<std::endl;
		#endif

		
		if(dispatch.second)
		{
			delete dispatch.second;
			dispatch.second = NULL;
		}

		return true;
	}
	else
	{
		return false;
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// wcNode
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define WC_WS1					"\t"
#define WC_WS2					"\t\t"
#define	WC_NODE_TAG_REWRAP(str) ("["+str+"]")
#define	WC_NODE_TAG_UNWRAP(str) str.erase(str.begin()); str.pop_back(); 



std::ostream&	operator<<(std::ostream& os, wcNode& node )
{
	os<<"{"<<std::endl;
	os<<WC_WS1<<node.tag.get_url()		<<std::endl;
	os<<WC_WS1<<node.meta.p_quality		<<std::endl;
	os<<WC_WS1<<node.meta.p_relevence	<<std::endl;
	os<<WC_WS1<<node.meta.p_density		<<std::endl;
	os<<WC_WS1<<node.edge.size()		<<std::endl;
	for( wcEdgeList::const_iterator itr = node.edge.cbegin(); itr!= node.edge.cend(); ++itr )
	{
		os<<WC_WS2<<WC_NODE_TAG_REWRAP(itr->first->tag.get_tag())<<std::endl;
		os<<WC_WS2<<itr->second<<std::endl;
	}
	os<<"}"<<std::endl;
	return os;
}



std::istream&	operator>>(std::istream& is, wcNode& node )
{
	wcString	temp;
	wcSize		nfeed = 0;

	while(temp!="{" && is.good() )
	{
		is>>temp;
	}
	if(is.good())
	{
		is>>temp;	node.tag = temp;
		is>>node.meta.p_quality;
		is>>node.meta.p_relevence;
		is>>node.meta.p_density; 
		is>>nfeed;
		while(nfeed--)
		{
			node.placeholders.emplace_back();
			is>>node.placeholders.back().first;		
			is>>node.placeholders.back().second;

			WC_NODE_TAG_UNWRAP(node.placeholders.back().first);
		}
		is>>temp; //}
	}
	return is;
}









wcNode::wcNode( const wcURL& url ) :
	tag(url), 
	cache(NULL)
{

}



wcNode::wcNode( std::istream& is ) :
	tag(), 
	cache(NULL)	
{
	is>>(*this);
}



wcNode::~wcNode()
{
	if(cache)			delete cache;
	if(analysis)		delete analysis;
}



void wcNode::operator+=( wcNode& other )
{
	edge.emplace_back(&other,WC_DEFAULT_EDGE);
}



void wcNode::perform( const wcStringCell& query_vector )
{
	if(!cache)
	{
		cache = new wcCache(tag);
		if(cache)
		{
			analysis			= new wcPageAnalyzer(*cache,query_vector);
			
			/// Grab analysis results
			meta.p_density		= analysis->results.density;
			meta.p_quality		= analysis->results.quality;
			meta.p_relevence	= analysis->results.relevance;

			/// Add any found links to the placeholder list so that they can be resolved later
			wcFloat rank_init = (1.0f/analysis->results.urls.size());
			for( wcStringCell::const_iterator uitr = analysis->results.urls.begin(); uitr != analysis->results.urls.end(); ++uitr )
			{
				#if !WC_GRAPH_NO_PREFILTER
				if(wcAnchorFilter(*uitr))
				#endif
					placeholders.emplace_back(*uitr,rank_init);
			}


			/// Delete the analyzer and the cache to save memory
			delete analysis;	analysis = NULL;
			delete cache;		cache	 = NULL;

			/// Mark dispacth as complete
			dispatch_done = true;
		}
	}
}



wcNodeStat wcNode::reassimilate( wcGraph& graph )
{
	for( wcPlaceHolderList::const_iterator plh = placeholders.cbegin(); plh != placeholders.cend(); ++plh )
	{
		for( wcNodeList::const_iterator node = graph.nodes.cbegin(); node!= graph.nodes.cend(); ++node )
		{
			if((*node)->tag.get_tag()==plh->first)
			{
				edge.emplace_back(const_cast<wcNode_s>(*node),plh->second);
				break;
			}
		}
	}
	return (placeholders.size()==edge.size())?(wcNoErr):(wcReassimilationFailure);
}



wcNode_s wcNode::max_edge( wcFlags target )
{
	wcNode_s	temp(0);
	wcFloat		value(0);
	for( wcEdgeList::const_iterator cedge = edge.cbegin(); cedge != edge.cend(); ++cedge )
	{

		if( (	(wcIS_SET(target,0) && (cedge->first->meta.p_quality	> value ))
			||	(wcIS_SET(target,1) && (cedge->first->meta.p_density	> value ))
			||	(wcIS_SET(target,2) && (cedge->first->meta.p_relevence	> value ))
			)
		&&	(cedge->first->tag!=tag)
		){
			
			value = cedge->first->meta.p_quality;
			temp  = cedge->first;
		}	
	}
	return temp;
}



void wcNode::dispatch_next( wcGraph& graph, const wcStringCell& query_vector )
{
	if(placeholders.empty())
		return;
	else
	{
		/// Check that this URL does not have an associated node in the 
		if(!graph.exists(placeholders.front().first))
		{
			/// Add new node to the master graph (by URL)
			graph.nodes.emplace_back( new wcNode(placeholders.front().first.c_str()) );
		
			/// Add the node to the dispatch list
			dispatch_list.emplace_back(
				graph.nodes.back(),
				new wcThread( &wcNode::perform, graph.nodes.back(), query_vector )
			);
			dispatch_list.back().second->detach();

			/// Give this node a pointer to the newly added node
			edge.emplace_back(graph.nodes.back(),WC_DEFAULT_EDGE);
		} 
		#if WC_GRAPH_USE_DEBUG_TOOLS 
		else
		{
			std::cout<<"[W] : Dispatch for -"<<placeholders.front().first<<" avoided."<<std::endl;
		}
		#endif	

		/// Remove its name from the "pending" list
		placeholders.pop_front();
	}
}





void wcNode::explore( wcGraph& graph, const wcStringCell& query_vector, wcSize n )
{
	if( !n || (n > placeholders.size()) )
		n = placeholders.size();

	for(;;)
	{
		if( dispatch_list.empty() && !n )
			break;

		while( (dispatch_list.size() < WC_GRAPH_DISPATCH_LIMIT) && n )
		{
			dispatch_next(graph,query_vector); --n;
		}
		dispatch_list.remove_if(wcDispatchComplete);
	}

	// Force-Clean the dispacth list
	for( wcDispatchList::iterator clitr = dispatch_list.begin(); clitr!=dispatch_list.end(); ++clitr )
	{
		delete clitr->second;
	}
}




#if WC_GRAPH_USE_DEBUG_TOOLS 
void	wcNode::show_placeholders( std::ostream& os )
{
	for( wcPlaceHolderList::const_iterator plitr = placeholders.cbegin(); plitr!= placeholders.cend(); ++plitr )
	{
		os<<plitr->first<<std::endl;
	}
}

void	wcNode::show_edgetags( std::ostream& os )
{
	for( wcEdgeList::const_iterator edgeitr = edge.cbegin(); edgeitr!= edge.cend(); ++edgeitr )
	{
		os<<edgeitr->first->tag<<std::endl;
	}
}
#endif





//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// wcGraph
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


std::ostream&	operator<<( std::ostream& os, wcGraph& graph )
{
	os<<graph.nodes.size()<<std::endl;
	for( wcNodeList::const_iterator itr = graph.nodes.cbegin(); itr!= graph.nodes.cend(); ++itr )
	{
		os<<*(const_cast<wcNode_s>(*itr));
	}
	return os;
}



std::istream&	operator>>( std::istream& is, wcGraph& graph )
{
	wcSize	nfeed = 0;			
	is>>nfeed;
	while(nfeed--)
	{
		graph.nodes.emplace_back(new wcNode(is));
	}

	#if !WC_DISABLE_AUTO_REASSIMILATION
	for( wcNodeList::iterator itr = graph.nodes.begin(); itr!= graph.nodes.end(); ++itr )
	{
		if( (*itr)->reassimilate(graph) == wcReassimilationFailure )
		{
			#if WC_GRAPH_USE_DEBUG_TOOLS 
			std::cerr<<"[WC] Load File Corrupted"<<std::endl;
			#endif
			break;
		}
	}
	#endif

	return is;
}



wcGraph::wcGraph( const wcString& name, const wcURL& url, const wcStringCell& query_vector ) :
	name(name)
{
	// Check for a save state
	#if !WC_GRAPH_DISABLE_STATE_LOAD
	std::ifstream state_in( (wcGlobal::get_state_root()+name+WC_TAG_GRAPHFILE_EXT).c_str() );
	if( state_in.is_open() )
	{
		#if WC_GRAPH_USE_DEBUG_TOOLS 
		std::cout<<"[S] Loading "		<<name<<std::endl;
		#endif
		
		state_in>>*this;

		#if WC_GRAPH_USE_DEBUG_TOOLS 
		std::cout<<"[S] Done loading "	<<name<<std::endl;
		#endif
	}
	#endif

	nodes.emplace_back( new wcNode(url) );
	cursor = nodes.back();
	cursor->perform(query_vector);
}



wcGraph::~wcGraph()
{
	/// Dump graph-state information
	std::ofstream state_out( (wcGlobal::get_state_root()+name+WC_TAG_GRAPHFILE_EXT).c_str() );
	state_out<<*this;
	
	
	/// Delete all nodes
	for( wcNodeList::iterator itr = nodes.begin(); itr!= nodes.end(); ++itr )
	{
		if(*itr) 
			delete *itr;
	}
}




bool wcGraph::exists( const wcString& url )
{
	for( wcNodeList::const_iterator nitr = nodes.cbegin(); nitr!= nodes.cend(); ++ nitr )
	{
		if((*nitr)->tag == url)
			return true;
	}
	return false;
}




wcGraphStat wcGraph::step( const wcStringCell& query_vector, wcFlags flags, wcSize n ) 
{
	// Update Optimization Reference
	wcAlgorithm::wcOptimaRefUpdate(cursor->meta);

	#if WC_GRAPH_USE_DEBUG_TOOLS 
	wcAlgorithm::wcOptimaRefShow();
	#endif	

	// DL Edge data
	cursor->explore(*this,query_vector,n);

	
	if(cursor->edge.size())
	{
		wcLVector distances;

		for( wcNodeList::const_iterator nitr = nodes.cbegin(); nitr!= nodes.cend(); ++ nitr )
			distances.emplace_back(const_cast<wcNode_s>(*nitr),wcAlgorithm::wcOptimaDistance((*nitr)->meta));

		distances.sort();

		// Select the new cursor based on the most optimal node
		cursor = distances.front().first;
		if( distances.size() > 1 )
		{
			wcSize nbackup = distances.size()/10;
			for( wcLVector::const_iterator ditr = ++distances.cbegin(); ditr!= distances.cend(); ++ditr )
			{
				if(--nbackup)
					saves.push_back( ditr->first );
				else
					break;
			}
			
		}
	}
	else
	{
		/// Fall back of a savepoint if possible
		if( !saves.empty() )
		{
			cursor = saves.front();
			saves.pop_front();

			#if WC_GRAPH_USE_DEBUG_TOOLS 
			std::cout<<"[W] : ==================================================================\n";
			std::cout<<"[W] : Out of Links! Falling back to "<< cursor->tag<<std::endl;
			std::cout<<"[W] : ==================================================================\n";
			#endif	
		}
		else
			return wcNoGStep;
	}
	return wcNoGErr;
}