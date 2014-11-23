#include	"wcTag.hpp"


#define		WC_TAG_ALNUM_REPLACE_CHAR		'_'



std::ostream& operator<<( std::ostream& os, const wcTag& tag )
{
#if		WC_TAG_DISP_FULL
	os<<"{ "<<tag.tag<<" from "<<tag.url<<" }";
#elif	WC_TAG_DISP_REDUCED
	os<<tag.url;
#endif
	return os;
}



wcTag::wcTag()
{

}



wcTag::wcTag( const wcURL& url ) :
	tag(url),
	url(url)
{
	auto_gen_tag();
	auto_gen_filenames();
}



wcTag::~wcTag()
{

}



void wcTag::operator= ( const wcString& url )
{
	this->tag = url;
	this->url = url;
	auto_gen_tag();
	auto_gen_filenames();
}



void wcTag::auto_gen_tag()
{
	for( wcString::iterator tag_itr = tag.begin(); tag_itr != tag.end(); ++tag_itr )
	{
		if(!wcVALCHAR(*tag_itr))
			*tag_itr = WC_TAG_ALNUM_REPLACE_CHAR;
	}
}


void wcTag::auto_gen_filenames()
{
	cache_filename	+= wcGlobal::get_cache_root();
	text_filename	+= wcGlobal::get_content_root();
	map_filename	+= wcGlobal::get_state_root();
	cache_filename	+= tag;
	text_filename	+= tag;
	map_filename	+= tag;
	cache_filename	+= WC_TAG_CACHEFILE_EXT;
	text_filename	+= WC_TAG_TEXTFILE_EXT;
	map_filename	+= WC_TAG_MAPFILE_EXT;
}