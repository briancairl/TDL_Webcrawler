#ifndef WC_GRAPH_HPP
#define WC_GRAPH_HPP

	#include "wcAnalysis.hpp"
	#include "wcCache.hpp"
	#include "wcTypes.hpp"
	#include "wcDef.hpp"
	#include "wcTag.hpp"	

	#include <iostream>


	/// @note wcSession objects manage wcAgent objects
	class	wcSession;
	class	wcAgent;

	typedef	struct wcmxMonitor
	{
		wcThread*	thread;
		wcAgent*	agent;
	} wcmMonitor;
	typedef	std::list<wcmMonitor>			wcmMonitorList;

	#define WC_GRAPH_USE_DEBUG_TOOLS		1
	#define WC_GRAPH_USE_DEBUG_TOOLS_EXT	0
	#define WC_GRAPH_DISABLE_STATE_LOAD		0
	#define WC_GRAPH_DISPATCH_LIMIT			5
	#define	WC_SUBOPTIMALITY_SATURATION		1.0f
	

	namespace wcm
	{
		void	Dispatch( ::wcAgent* agent, ::wcSession* session, ::wcStringCell* query_vector );
		void	Cleanup();
		void	Terminate();
	};


	




	class	wcAgent
	{
	friend  class wcSession;
	friend	std::ostream&		operator<<( std::ostream& os, wcAgent& node );
	friend	std::istream&		operator>>( std::istream& is, wcAgent& node );
	friend	void				wcm::Dispatch( wcAgent* agent, wcSession* session, wcStringCell* query_vector );
	friend	bool				isSuboptimal( wcAgent& agent );
	friend	bool				isRemovable( wcAgent& agent );
	friend	void				updateOptimalityThreadhold( wcAgent& agent );
	private:
		wcTag					tag;
		wcMetaVector			meta;
		wcFloat					cost;
		wcStringCell			links;

		bool					dispatch_done;
		bool					dispatch_needed;
		void					create_map();

	public:

		wcAgent( const wcURL& url );
		~wcAgent();
		
		void					perform( wcSession* session, wcStringCell* query_vector );

		inline bool				operator==( const wcAgent& agent )	{ return this->cost==agent.cost; }
		inline bool				operator!=( const wcAgent& agent )	{ return this->cost==agent.cost; }
		inline bool				operator<=( const wcAgent& agent )	{ return this->cost<=agent.cost; }
		inline bool				operator>=( const wcAgent& agent )	{ return this->cost>=agent.cost; }
		inline bool				operator< ( const wcAgent& agent )	{ return this->cost< agent.cost; }
		inline bool				operator> ( const wcAgent& agent )	{ return this->cost> agent.cost; }
	};


	void						updateOptimalityThreadhold( wcAgent& agent );
	bool						isSuboptimal( wcAgent& agent );
	bool						isRemovable( wcAgent& agent );

	typedef std::list<wcAgent>	wcTeam;
	typedef wcTeam::iterator	wcAgentItr;

#endif
