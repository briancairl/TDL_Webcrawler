#ifndef		NETGRAPH_HPP
#define		NETGRAPH_HPP

#include	<iostream>
#include	<cmath>

#include	"netBuffer.hpp"
#include	"netTypedefs.hpp"
#include	"netUtils.hpp"

#include	"netxmlbot.hpp"

namespace netGraph
{
	using namespace netTypes;
	using namespace netPrimitives;
	
	#define NET_GRAPH_DEBUG	
	#define	NET_GRAPH_DEFAULT_FEEDBACK			(NET_FLAG_MASK(relevance)|NET_FLAG_MASK(rank))
	#define NET_GRAPH_MAX_CHILDREN				10
	#define NET_GRAPH_DOWNLOAD_MINSIZE			2000
	#define NET_GRAPH_DOWNLOAD_MAXSIZE			1000000
	
	#define NET_RAND							((netFloat)(rand()%1000)/1000.0f)
	#define NET_GRAPH_UTIL_FN(x)				(NET_RAND*0.5f + x*0.5f)


	class netNode : public netBuffer
	{
	typedef 
		std::list<netNode> netEdge;
	friend 
		std::ostream& operator<<(std::ostream& os,netNode& edge );
	public:

		typedef enum
		{
			relevance = 0,
			distance,
			rank
		} netFeedbackTypes;

		netNode( 
			const netURL		address, 
			const netFlags		feedback_config = NET_GRAPH_DEFAULT_FEEDBACK
		);
		
		netNode(
			netNode*&			next,
			const netFlags		feedback_config = NET_GRAPH_DEFAULT_FEEDBACK
		);
		
		~netNode()
		{}

		bool					link_unvisted( const netString& address );
		bool					analyze(const netStringCell& topics);
		void					crawl(const netStringCell& topics);

		inline netNode*&		get_next()		{ return edges.next; }
	private:

		const netFlags			feedback_config;
		void					feedback_service(const netStringCell& topics);
		struct
		{
			bool				analyzed;
			netFloat			relevance;
			netFloat			distance;
			netSize				rank;
			netSize				whitespace;
			netSize				hits;
			netURLlist			links;
		} feedback;

		netSize					children_monitor();
		struct
		{
			netNode*			next;
			netFloat			average_speed;
			netFloat			max_relevance;
			netEdge				children;
			netSize				children_analyzed;
		} edges;
	};

}

#endif