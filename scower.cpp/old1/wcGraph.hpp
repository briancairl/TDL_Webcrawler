#ifndef WC_GRAPH_HPP
#define WC_GRAPH_HPP

	#include "wcAnalysis.hpp"
	#include "wcCache.hpp"
	#include "wcTypes.hpp"
	#include "wcDef.hpp"
	#include "wcTag.hpp"	

	#include <iostream>
	

	#define WC_GRAPH_USE_DEBUG_TOOLS		1
	#define WC_GRAPH_USE_DEBUG_TOOLS_EXT	0
	
	#define WC_GRAPH_DISABLE_STATE_LOAD		0
	#define WC_GRAPH_DISPATCH_LIMIT			20
	#define	WC_DEFAULT_EDGE					1
	#define	WC_DISABLE_AUTO_REASSIMILATION	0
	#define WC_GRAPH_NO_PREFILTER			0
	
	class	wcGraph;
	class	wcNode;
	typedef	wcNode*							wcNode_s;
	typedef std::pair<wcNode_s,wcThread*>	wcDispatch;
	typedef std::pair<wcNode_s,wcFloat>		wcEdge;
	typedef std::pair<wcString,wcFloat>		wcPlaceHolder;
	typedef std::list<wcPlaceHolder>		wcPlaceHolderList;
	typedef std::list<wcEdge>				wcEdgeList;
	typedef std::list<wcNode_s>				wcNodeList;
	typedef std::list<wcDispatch>			wcDispatchList;

	class	wcL :
		public std::pair<wcNode_s,wcFloat>	
	{
	public: 
		bool operator< ( const wcL& other )	{ return second <	other.second; }
		bool operator> ( const wcL& other )	{ return second >	other.second; }
		bool operator==( const wcL& other )	{ return second ==	other.second; }		
		bool operator<=( const wcL& other )	{ return second <=	other.second; }
		bool operator>=( const wcL& other )	{ return second >=	other.second; }
		wcL( wcNode_s node , wcFloat val ): std::pair<wcNode_s,wcFloat>(node,val) {}
		~wcL() {}
	};
	typedef std::list<wcL>					wcLVector;


	typedef enum
	{
		wcNoErr = 0,
		wcReassimilationFailure
	} wcNodeStat;



	class wcNode
	{
	friend  class wcGraph;
	friend	std::ostream&		operator<<( std::ostream& os, wcNode& node );
	friend	std::istream&		operator>>( std::istream& is, wcNode& node );
	friend	bool				wcDispatchComplete( wcDispatch& dispatch );
	private:
		wcTag					tag;
		wcEdgeList				edge;
		wcMetaVector			meta;

		wcPlaceHolderList		placeholders;
		wcDispatchList			dispatch_list;
		void					dispatch_next( wcGraph& graph, const wcStringCell& query_vector );
		bool					dispatch_done;

		wcCache*				cache;
		wcPageAnalyzer*			analysis;
	public:
		wcNode( const wcURL& url );
		wcNode( std::istream& is );
		~wcNode();
		
		void					perform( const wcStringCell& query_vector );
		void					operator+=( wcNode& other );
		wcNodeStat				reassimilate( wcGraph& graph );
		void					explore( wcGraph& graph, const wcStringCell& query_vector, wcSize n = 0UL );
		wcNode_s				max_edge( wcFlags target );

		#if WC_GRAPH_USE_DEBUG_TOOLS 
		void					show_placeholders(std::ostream& os);
		void					show_edgetags(std::ostream& os);
		#endif
	};

	

	typedef enum
	{
		wcNoGErr = 0,
		wcNoGStep
	} wcGraphStat;


	class wcGraph
	{
	friend  class wcNode;
	friend	std::ostream&		operator<<( std::ostream& os, wcGraph& node );
	friend	std::istream&		operator>>( std::istream& is, wcGraph& node );
	private:
		wcString				name;
		wcNode_s				cursor;
		wcNodeList				nodes;
		wcNodeList				saves;
	public:
		wcGraph( const wcString& name );
		wcGraph( const wcString& name, const wcURL& url, const wcStringCell& query_vector );
		~wcGraph();

		bool					exists( const wcString& url );
		wcGraphStat				step( const wcStringCell& query_vector, wcFlags flags, wcSize n = 0UL );
		
		inline wcSize			size()		{ return nodes.size(); }
	};


#endif
