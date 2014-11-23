#include	"netgraph.hpp"
#include	<exception>

namespace netGraph
{
	using namespace netTypes;
	using namespace netPrimitives;
	

	namespace global
	{
		netStringCell		visted;
	}

	std::ostream& operator<<(std::ostream& os,netNode& edge )
	{
		os<<"Address {"		<<edge.get_address()<<"}"	<<std::endl;
		os<<"\tRelevance : "<<edge.feedback.relevance	<<std::endl;
		os<<"\tTopic Hits: "<<edge.feedback.hits		<<std::endl;
		os<<"\tWhitespace: "<<edge.feedback.whitespace	<<std::endl;
		os<<"\tRank      : "<<edge.feedback.rank		<<std::endl;

		return os;
	}



	netNode::netNode( 
		const netURL			address, 
		const netFlags			feedback_config
	) :
		feedback_config(feedback_config),
		netBuffer(address)
	{
		feedback.analyzed			= false;
		edges	.next				= NULL;
		edges	.average_speed		= 0;
		edges	.children_analyzed	= 0;
	}

	


	netNode::netNode(
		netNode*&				next,
		const netFlags			feedback_config
	) :
		feedback_config(feedback_config),
		netBuffer((netBuffer*&)next)
	{
		feedback.analyzed			= true;
		feedback.links				= next->feedback.links; // copy links
		feedback.relevance			= next->feedback.relevance;
		feedback.hits				= next->feedback.hits;
		feedback.whitespace			= next->feedback.whitespace;
		feedback.rank				= next->feedback.rank;

		edges	.next				= NULL;
		edges	.average_speed		= 0;
		edges	.children_analyzed	= 0;

		global::visted.emplace_back(address);		
	}
		

	bool netNode::link_unvisted( const netString& address )
	{
		netSize min_end;
		netSize min_idx;
		for( netStringCell::const_iterator citr(global::visted.cbegin()); citr!=global::visted.cend(); ++citr )
		{
			if(citr->front()==address.front())
			{
				min_end = (citr->size() < address.size())?(citr->size()):(address.size());
				for( min_idx = 0; min_idx < min_end; min_idx++ )
				{
					if(address[min_idx]!=(*citr)[min_idx])
						break;
				}
				if(min_idx==min_end)
					return false;
			}
		}
		return true;
	}


	bool netNode::analyze(const netStringCell& topics)
	{
		if(feedback.analyzed)
			return true;

		if(!ready() || NET_FLAG_IS_SET(state_flags,netBuffer::timeout_failure))
		{
			return false;
		}
		else
		{
			feedback_service(topics);
		}
		return true;
	}



	void netNode::feedback_service(const netStringCell&	topics)
	{
		/// Harvest Links
		feedback.rank		=	netUtils::optimized::get_HREF(*this,feedback.links);

		/// Get a Hit-count for
		//feedback.whitespace =	netUtils::optimized::get_WSCount(*this);
		//feedback.hits		=	0;
		//for( netStringCell::const_iterator citr(topics.cbegin()); citr!=topics.cend(); ++citr )
		//	feedback.hits	+=	netUtils::optimized::get_TolkenCount(*this,*citr);


		/// Added to avoid traps*
		//feedback.hits		*=  netUtils::optimized::get_PCount(*this);

		/// Normalize Hit-count by whitespace count
		///	+ This is an estimate for the number of hits/total words
		//feedback.relevance	=	NET_GRAPH_UTIL_FN(((netFloat)feedback.rank)/((netFloat)feedback.whitespace));
		feedback.relevance	=	NET_GRAPH_UTIL_FN(((netFloat)feedback.rank)/2000.0f);

	}



	netSize	netNode::children_monitor()
	{
		netSize n_working(0);
		for( netEdge::iterator citr(edges.children.begin()); citr!=edges.children.end(); ++citr )
		{
			if(citr->ready())
			{
				if(citr->strip_service()) 
				{
					edges.average_speed += citr->get_meta().get_download_speed();
					edges.average_speed /= 2;
				}
			}
			else
				n_working += !citr->ready();
			
		}
		return n_working;
	}


	void netNode::crawl(const netStringCell& topics)
	{
		if(analyze(topics))
		{
			#ifdef NET_GRAPH_DEBUG
			std::cout	<< *this;
			#endif


			netStringCell temp = feedback.links;

			#ifdef NET_GRAPH_DEBUG
			std::cout	<<"-Pruning..."<<std::endl;
			#endif	

			feedback.links.clear();
			for( netStringCell::const_iterator citr(temp.cbegin()); citr!=temp.cend(); ++citr )
			{
				if(link_unvisted(*citr))
					feedback.links.emplace_back(*citr);
			}


			#ifdef NET_GRAPH_DEBUG
			std::cout	<<"-Establishing agents..."<<std::endl;
			#endif

			/// Agent Setup
			for( netStringCell::const_iterator citr(feedback.links.cbegin()); citr!=feedback.links.cend(); ++citr )
			{
				// Fixed Backoff
				while(children_monitor()==NET_GRAPH_MAX_CHILDREN);
				
				// Add new Child
				edges.children.emplace_back(*citr);

				#ifdef NET_GRAPH_DEBUG
				std::cout	<<"~Agent {"<<edges.children.back().address<<"} started."<<std::endl;;
				#endif
			}

			if(edges.children.empty())
			{
				#ifdef NET_GRAPH_DEBUG
				std::cout	<<"-Aborting."<<std::endl;
				#endif	
				return;
			}


			#ifdef NET_GRAPH_DEBUG
			std::cout	<<"-Starting analysis..."<<std::endl;
			#endif

			/// Agent Analysis
			for(;;)
			{
				for( netEdge::iterator citr(edges.children.begin()); citr!=edges.children.end(); ++citr )
				{
					if(citr->ready() && !citr->feedback.analyzed)
					{
						
						if(	(citr->get_size() > NET_GRAPH_DOWNLOAD_MINSIZE)
						&&	(citr->get_size() < NET_GRAPH_DOWNLOAD_MAXSIZE)
						){
							citr->analyze(topics);

							if( edges.max_relevance < citr->feedback.relevance )
							{
								edges.max_relevance = citr->feedback.relevance;
								edges.next			= &(*citr);
							}
						}


						if(++edges.children_analyzed>edges.children.size())
							return;

						#ifdef NET_GRAPH_DEBUG
						std::cout	
						<<"~Analyzing "
						<<edges.children_analyzed
						<<" of "
						<<edges.children.size()
						<<std::endl;
						#endif
					}
				}
			}
		}
	}
}