#ifndef		NETUTILS_HPP
#define		NETUTILS_HPP

#include	"netTypedefs.hpp"
#include	"netBuffer.hpp"
#include	<cctype>

namespace netUtils
{
	using namespace netTypes;
	using namespace netPrimitives;


	namespace utility
	{
		netString&	cleanupXML( netString& snippet );

		netString	get_BaseAddress( const netString& address );
	}




	/// @brief	A set of dedicated-task optimized functions
	namespace optimized 
	{
		netSize		get_HREF( netBuffer& buffer, netStringCell& out, bool resolve_rels=true );

		netSize		get_PCount( netBuffer& buffer );

		netSize		get_TolkenCount( netBuffer& buffer, const netString& tolken );

		netSize		get_WSCount( netBuffer& buffer );

	}

}

#endif