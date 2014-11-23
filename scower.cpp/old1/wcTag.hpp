#ifndef WC_TAG_HPP
#define WC_TAG_HPP
	
	#include "wcGlobal.hpp"
	#include "wcTypes.hpp"
	#include "wcDef.hpp"

	#include <iostream>

	#define		WC_TAG_DISP_FULL		0
	#define		WC_TAG_DISP_REDUCED		1
	#define		WC_TAG_CACHEFILE_EXT	".CCF"
	#define		WC_TAG_MAPFILE_EXT		".MAP"
	#define		WC_TAG_TEXTFILE_EXT		".TXT"
	#define		WC_TAG_GRAPHFILE_EXT	".GRP"

	class wcTag
	{
	friend 
		std::ostream&			operator<<( std::ostream& os, const wcTag& tag );
	private:
		wcString				cache_filename;
		wcString				text_filename;
		wcString				map_filename;
		wcString				tag;
		wcURL					url;
		void					auto_gen_tag();
		void					auto_gen_filenames();
	public:
		wcTag();
		wcTag( const wcURL& url );
		~wcTag();

		void					operator= ( const wcString& url );
		inline bool				operator==( const wcTag& other )	{ return tag==other.tag;} 
		inline bool				operator==( const wcString& other )	{ return tag==other;}
		inline bool				operator!=( const wcTag& other )	{ return tag!=other.tag;} 
		inline bool				operator!=( const wcString& other )	{ return tag!=other;}
		inline const wcString&	get_tag()							{ return tag; }
		inline const wcString&	get_url()							{ return url; }
		inline const wcString&	get_cache_filename()				{ return cache_filename; }
		inline const wcString&	get_text_filename()					{ return text_filename; }
		inline const wcString&	get_map_filename()					{ return map_filename; }
	};

#endif
