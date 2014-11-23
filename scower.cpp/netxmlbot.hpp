/// @file	netxmlbot.hpp
///	@brief 
///	@{
///			NETXMLBOT is hierarchal XML search automoton compatible with all std::istream 
///			and std::istream derived stream types.
///	@}
#ifndef		NETXMLBOT_HPP
#define		NETXMLBOT_HPP

#include	"netTypedefs.hpp"
#include	"netBuffer.hpp"

namespace netxmlbot
{
	using namespace netPrimitives;
	using namespace netTypes;


	typedef enum
	{
		xBadTag,
		xBadDoc,
		xInlineTag,
		xOpenTag,
		xCloseTag,
		xChildTag
	} netxmltype;



	class netxmlrange
	{
	private:
		bool				inf;
		netPos 				beg;
		netPos				end;
	public:
		netxmlrange( bool _inf = false ) 
			: beg(0), end(0), inf(_inf)							{}
		~netxmlrange()											{}
		inline void			set_beg( const netSize& p0 )		{ beg = p0; }
		inline void			set_end( const netSize& p1 )		{ end = p1; }
		inline void			set_from_beg( netBuffer& stream )	{ stream.set_loc(beg); }
		inline void			set_from_end( netBuffer& stream )	{ stream.set_loc(end); }
		inline bool			is_zero_width()						{ return (beg==end); }
		inline bool			is_nonzero_width()					{ return (beg< end); }
		inline bool			is_valid_width()					{ return (beg<=end); }
		inline bool			is_within( const netSize& p)		{ return inf||((p>=beg)&&(p<=end)); }
		inline bool			operator>( const size_t& ulen )		{ return (size_t)(end-beg) < ulen; }
	};



	class netxmlbot
	{
	#define NETXMLBOT_SEARCH_VALID(stream)		((stream.good())&&(limits.is_within(stream.get_loc())))
	#define NETXMLBOT_PARAM_PAIR(par,val)		par"=\""val"\""
	#define	NETXMLBOT_INHERIT_OFFSET			0
	private:
		char				cursor;
		bool				is_valid;

		const netString		tag;
		const netString		paramlist;

		netxmlrange			limits;
		netxmlrange			open_tag;
		netxmlrange			contents;
		netxmlrange			close_tag;

		bool				match_tolken(netBuffer& stream);		
		bool				match_paramlist(netBuffer& stream );

		netxmltype			find_open_tag(netBuffer& stream);
		netxmltype 			find_close_tag(netBuffer& stream);
		bool				find_pair(netBuffer& stream);

	public:
		netxmlbot(netBuffer& stream, const netSize& offset, const char* tag, const char* paramlist="");
		netxmlbot(netBuffer& stream, const netSize& offset, const netxmlbot& parent, const char* tag,const char* paramlist="");
		~netxmlbot()		{}		


		netString&			get_content( netBuffer& stream, netString& outstr );
		netString&			get_param( netBuffer& stream, netString& outstr );

		bool				has_content();
		bool				has_params();
		bool				good();
	};

};


#endif
