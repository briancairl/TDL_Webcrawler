/// @file	netxmlbot.cpp
///	@brief 
///	@{
///			NETXMLBOT is hierarchal XML search automoton compatible with all std::istream 
///			and std::istream derived stream types.
///	@}
#include "netxmlbot.hpp"

namespace netxmlbot
{
	using namespace netTypes;
	using namespace netPrimitives;


	///	@brief	Default constructor
	///
	/// @param	stream	xml-source stream. This stream is an STL compliane input stream or
	///					std::istream derivative.
	/// @param	tag		section tage
	netxmlbot::netxmlbot( netBuffer& stream, const netSize& offset, const char* tag, const char* paramlist) :
		tag(tag), paramlist(paramlist), is_valid(0), cursor(-1), limits(true)
	{
		/// Set Stream Position to first element in parent's contents
		if(offset!=0)
			stream.set_loc(offset);

		/// Match against supplied tolken
		is_valid = find_pair(stream); 
	}



	///	@brief	Parent-Child constructor.
	///			This constructor can be used if a node is to be found withing the bounds
	///			of a previously found node.
	///
	/// @param	stream	xml-source stream. This stream is an STL compliant input stream or
	///					std::istream derivative.
	/// @param	tag		section tage
	netxmlbot::netxmlbot( netBuffer& stream, const netSize& offset, const netxmlbot& parent, const char* tag, const char* paramlist ) :
		tag(tag), paramlist(paramlist), is_valid(0), cursor(-1), limits(parent.contents)
	{
		/// Set Stream Position to first element in parent's contents
		if(offset==0)
			limits.set_from_beg(stream);
		else
			stream.set_loc(offset);		

		/// Match against supplied tolken
		is_valid = find_pair(stream); 
	}



	/// @brief	Flag corresponding is_valid output state
	/// @return	TRUE if the input XML file was well formed and the automoton found the specified
	///			particle before the end of the stream was reached.
	bool netxmlbot::good()
	{
		return is_valid;
	}



	/// @brief	Flag corresponding to non-zero-size content
	///	@return	TRUE if there is content surrounded by the particles corresponding to the tags
	///			supplied to the automoton.
	bool netxmlbot::has_content()
	{
		return contents.is_nonzero_width();
	}



	/// @brief	Flag corresponding to non-zero-size parameter list
	///	@return	TRUE if an open-tag particle for the supplied tag contains parameters
	bool netxmlbot::has_params()
	{
		return open_tag>tag.size();
	}



	/// @brief	Checks if next stream sequence is the tag-tolken
	///
	/// @param	stream	xml-source stream. This stream is an STL compliant input stream or
	///					std::istream derivative.
	/// @return	TRUE if the tolken is matched
	bool netxmlbot::match_tolken( netBuffer& stream )
	{
		for( netString::const_iterator tag_itr=tag.cbegin() ; tag_itr!=tag.cend(); ++tag_itr )
		{
			if( stream.next() != *tag_itr )
				return false;
		}
		return true;
	}



	/// @brief	Checks if next stream sequence is the param list
	///
	/// @param	stream	xml-source stream. This stream is an STL compliant input stream or
	///					std::istream derivative.
	/// @return	TRUE if the tolken is matched
	bool netxmlbot::match_paramlist( netBuffer& stream )
	{
		if(!paramlist.empty())
		{
			stream.next(); // skip ws
			for( netString::const_iterator par_itr=paramlist.cbegin() ; par_itr!=paramlist.cend(); ++par_itr )
			{
				if( stream.next() != *par_itr )
					return false;
			}
		}
		return true;
	}


	/// @brief	Gets a well-formed XML open tag and return its primitive type
	///
	/// @param	stream	xml-source stream. This stream is an STL compliant input stream or
	///					std::istream derivative.
	/// @return	xOpenTag -OR- xInlineTag upon success; xBadTag if the stream has ended -OR- the document is ill-formed
	netxmltype netxmlbot::find_open_tag( netBuffer& stream )
	{
		while(NETXMLBOT_SEARCH_VALID(stream))
		{
			if(stream.next() == '<')
			{
				if(match_tolken(stream) && match_paramlist(stream))
				{
					open_tag.set_beg(stream.get_loc()+1);
				} 
				else
				{
					continue;
				}
				for(;;)
				{
					switch(stream.next())
					{
						case -1:
						case '<':
							return xBadTag;
						case '/':
							open_tag.set_end(stream.get_loc()-2);
							if(stream.next()=='>')
								return xInlineTag;
							else
								continue;
						case '>':
							open_tag.set_end(stream.get_loc()-2);
							return xOpenTag;
						default:
							continue;
					}
				}
			}
		}
		return xBadDoc;
	}



	/// @brief	Gets a well-formed XML closing tag
	///
	/// @param	stream	xml-source stream. This stream is an STL compliant input stream or
	///					std::istream derivative.
	/// @return	xCloseTag upon success
	netxmltype netxmlbot::find_close_tag( netBuffer& stream )
	{
		// Contents will start at the end of the open-tag
		contents.set_beg(stream.get_loc());

		std::streampos temp_pos;
		size_t h_depth(1);
		while(NETXMLBOT_SEARCH_VALID(stream))
		{
			temp_pos = stream.get_loc()-1;
			if(stream.next() == '<')
			{
				if(stream.next() == '/')
				{
					if(!(--h_depth) && match_tolken(stream))
					{
						close_tag.set_beg(stream.get_loc());
						contents.set_end(temp_pos);
						for(;;)
						{
							switch(stream.next())
							{
								case -1:
									return xBadTag;
								case '>':
									close_tag.set_end(stream.get_loc()-1);
									return xCloseTag;
								default:
									continue;
							}
						}
					}
				}
				else
				{
					++h_depth;
				}
			}
		}
		return xBadDoc;
	}



	/// @brief	Gets a well-formed XML pair attributed to specified tolken
	///
	/// @param	stream	xml-source stream. This stream is an STL compliant input stream or
	///					std::istream derivative.
	/// @return TRUE if a valid pair is found
	bool netxmlbot::find_pair( netBuffer& stream )
	{
		switch(find_open_tag(stream))
		{
			case xOpenTag:
				return find_close_tag(stream)==xCloseTag;
			case xInlineTag:
				return true;

			case xBadDoc:
			case xBadTag:
			default:
				return false;
		}
	}



	/// @brief	Copies node contents in-stream to a string
	///
	/// @param	stream	xml-source stream. This stream is an STL compliant input stream or
	///					std::istream derivative.
	/// @param	outstr	string to modify with output (by append to back)
	///	@return outstr
	netString& netxmlbot::get_content( netBuffer& stream, netString& outstr )
	{
		if(stream.has_content())
		{
			contents.set_from_beg(stream);
			while( contents.is_within(stream.get_loc()) )
				outstr.push_back(stream.next());
		}
		return outstr; 
	}



	/// @brief	Copies node parameters in-stream to a string
	///
	/// @param	stream	xml-source stream. This stream is an STL compliant input stream or
	///					std::istream derivative.
	/// @param	outstr	string to modify with output (by append to back)
	///	@return outstr
	netString& netxmlbot::get_param( netBuffer& stream, netString& outstr )
	{
		if(stream.has_content())
		{
			open_tag.set_from_beg(stream);
			while( open_tag.is_within(stream.get_loc()) ) 
				outstr.push_back(stream.next());
		}
		return outstr; 
	}

};