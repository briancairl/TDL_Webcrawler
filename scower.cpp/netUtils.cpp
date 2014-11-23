#include "netutils.hpp"

namespace netUtils
{
	using namespace netTypes;
	
	namespace utility
	{

		netString& cleanupXML( netString& snippet )
		{
			bool		opened(false);
			netString	otmp;
			otmp.reserve(snippet.size());
			for( netString::iterator itr(snippet.begin()) ; itr!=snippet.end(); ++itr )
			{
				switch(*itr)
				{
				case '<':
					opened = true;
					break; 
				case '>':
					opened = false;
				}
				if(opened)
					otmp.push_back(*itr);
			}
			snippet = otmp;
			return snippet;
		}


		netString	get_BaseAddress( const netString& address )
		{
			netString	retval;
			bool		got_dot(false);
			for( netString::const_iterator itr = address.begin(); itr!=address.end(); ++itr )
			{
				switch(*itr)
				{
				case '/':
					if(got_dot)
						return retval;
					else
						retval += *itr;
					break;
				case '.':
					got_dot = true;
				default:
					retval += *itr;
					break;
				}
			}
			return retval;
		}

	}






	namespace optimized 
	{

		/// @brief Function optimized for harvesting page-links
		netSize get_HREF( netBuffer& buffer, netStringCell& out, bool resolve_rels  )
		{
			const char			href_tok[6] = "href=";
			const size_t		href_size   = 5;
			static netString	bass_addr;
			bool				href_found(false);
			char				temp(-1);


			if(resolve_rels)
				bass_addr = utility::get_BaseAddress(buffer.get_address());

			buffer.set_loc(0);
			while(buffer.good())
			{
				if(buffer.next()==href_tok[0])
				{
					href_found = true;
					for( size_t idx(1); idx < href_size; idx++ )
					{
						if(buffer.next()!=href_tok[idx])
						{
							href_found = false;
							break;
						}
					}
					if(href_found)
					{
						out.emplace_back();
						buffer.next();	// dummy-read {",'}
						while(href_found)
						{
							temp = buffer.next();
							switch(temp)
							{
							case -1:
							case '\"':
							case '\'':
								href_found = false;

								/// Resolve relative links
								if(resolve_rels)
								{
									if(!out.back().empty())
									{
										switch(out.back().front())
										{
											case '/':
												if(out.back().size()>=2 && out.back()[1]=='/')
													out.back() = "http:" + out.back();
												else
													out.back() = bass_addr + out.back();
												break;
											case '#':
											case '{':
												out.pop_back();
										}
									} else {
										out.pop_back();
									}
								} else {
									out.pop_back();
								}

								break;
							default:
								out.back().push_back(temp);
							}
						}
					}
				}
			}
			return out.size();
		}



		/// @brief Function optimized for harvesting page-links
		netSize get_PCount( netBuffer& buffer )
		{
			const char			p_tok[4] = "<p>";
			const size_t		p_size   = 2;
			bool				p_found(false);
			size_t				count(0);


			buffer.set_loc(0);
			while(buffer.good())
			{
				if(buffer.next()==p_tok[0])
				{
					p_found = true;
					for( size_t idx(1); idx < p_size; idx++ )
					{
						if(buffer.next()!=p_tok[idx])
						{
							p_found = false;
							break;
						}
					}
					count += p_found;
				}
			}
			return count;
		}



		netSize get_TolkenCount( netBuffer& buffer, const netString& tolken )
		{
			netSize hit_count(0);
			bool	hit(false);
			netSize idx;

			buffer.set_loc(0);
			while(buffer.good())
			{
				if(buffer.next()==tolken[0])
				{
					hit = true;
					for( idx = 1; idx < tolken.size(); idx++ )
					{
						if(buffer.next()!=tolken[idx])
						{
							hit = false;
							break;
						}
					}
				}
				if(hit)
					++hit_count;

				hit = false;
			}
			return hit_count;
		}



		netSize get_WSCount( netBuffer& buffer )
		{
			netSize hit_count(0);
			buffer.set_loc(0);
			while(buffer.good())
			{
				if(buffer.next()==' ')
					++hit_count;
			}
			return hit_count;
		}
	}
}