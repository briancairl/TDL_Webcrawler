#include	"wcAnalysis.hpp"
#include	"wcSession.hpp"



inline wcFloat nlrank_fn( const wcFloat& val, const wcFloat sens ) { return  (1.0f/(1.0f + expf(-sens*(val-0.5f)))); }



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
/// wcPageAnalyzer
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////


wcPageAnalyzer::wcPageAnalyzer( wcSession& session, wcCache* cache, const wcStringCell& query_vector,
	wcMetaVector& meta_o,	
	wcStringCell& url_o
){
	if(!cache->buffer_valid())
		return;

	make_base(cache);
	perf_body_get(cache);
	perf_link_get(cache,url_o,meta_o);
	perf_quality_analysis(cache,meta_o);
	perf_relevance_analysis(cache,query_vector,meta_o);

	#if !WC_NO_TEXT_DUMP
	perf_text_dump(cache,meta_o);
	#endif
}



wcPageAnalyzer::~wcPageAnalyzer() 
{
}




void wcPageAnalyzer::perf_body_get( wcCache* cache ) 
{
	cache->set_beg();
	bool opened(false);
	bool opened_within(false);
	while(cache->buffer_good())
	{
		if(cache->next()=='<')
		{
			switch(cache->next())
			{
			case '/':
				// </p>
				if( opened && (cache->next()=='p') && (cache->next()=='>') )
				{
					opened = false;
					text_ranges.back().second = cache->get_loc()-5UL; // length of </p>
				}	break;
			case 'p':
				// <p>
				if( !opened && (cache->next()=='>') )
				{
					opened = true;
					text_ranges.emplace_back(cache->get_loc()+1,0UL);
				}	break;
			}
		}
	}
}




void wcPageAnalyzer::perf_quality_analysis( wcCache* cache, wcMetaVector& meta_o )
{
	wcSize text_ulen(0);
	for( wcRangeVector::const_iterator itr = text_ranges.cbegin(); itr != text_ranges.cend(); ++itr )
	{
		text_ulen += itr->second-itr->first;
	}
	if(cache->get_size() && (text_ulen<cache->get_size()))
		meta_o[0] = ((wcFloat)(text_ulen))/((wcFloat)(cache->get_size()));
	else
		meta_o[0] = 0;
}




void	wcPageAnalyzer::perf_relevance_analysis( wcCache* cache, const wcStringCell& query_vector, wcMetaVector& meta_o )
{
	wcSize	hit_count	= 0;
	wcSize	ws_count	= 0;
	wcPos	idx			= 0;

	if( !text_ranges.empty() && !query_vector.empty() )
	{
		/// Perform Space (Word) Count 
		for( wcRangeVector::const_iterator itr = text_ranges.cbegin(); itr != text_ranges.cend(); ++itr )
		{
			cache->set_loc(const_cast<wcPos&>(itr->first));
			while( cache->get_loc() < itr->second )
			{
				if( cache->next() == ' ' )
					++ws_count;
			}
		}

		/// Perform Query Match Count 
		for( wcStringCell::const_iterator query = query_vector.begin(); query != query_vector.end(); ++query )
		{
			for( wcRangeVector::const_iterator itr = text_ranges.cbegin(); itr != text_ranges.cend(); ++itr )
			{
				cache->set_loc(const_cast<wcPos&>(itr->first));
				while( cache->get_loc() < itr->second )
				{
					if(cache->next() == query->front())
					{
						for( idx = 1; idx < query->size(); idx++ )
							if( (*query)[idx] != cache->next() ) break;
				
						if( idx == query->size() )
							++hit_count;
					}
				}
			}
		}

		if(ws_count&&hit_count)
		{
			meta_o[1] = ((wcFloat)hit_count)/((wcFloat)ws_count)*query_vector.size();
			meta_o[1] = (meta_o[1]>1.0f)?(1.0f):(meta_o[1]);
		}
		else
			meta_o[1] = 0;
	}
}




void wcPageAnalyzer::perf_text_dump( wcCache* cache, wcMetaVector& meta_o )
{
	if(meta_o[0]>0)
	{
		std::ofstream	dump_file(cache->get_tag()->get_text_filename().c_str());
		if(dump_file.is_open())
		{
			bool open(false);
			cache->set_beg();
			for( wcRangeVector::const_iterator itr = text_ranges.cbegin(); itr != text_ranges.cend(); ++itr )
			{
				cache->set_loc(const_cast<wcPos&>(itr->first));
				while( cache->get_loc() < itr->second )
				{
					cache->_inc();
					if(cache->curr()=='<')
						open = true;
					else 
					if(cache->curr()=='>')
						open = false;
					else 
					if(!open)
						dump_file<<cache->curr();
				}	
				dump_file<<std::endl;
			}
		}
	}
}



void wcPageAnalyzer::make_base(wcCache* cache)
{
	bool got_dot(false);
	wcString base_copy = cache->get_tag()->get_url();
	for( wcString::iterator itr = base_copy.begin(); itr!= base_copy.end(); ++itr )
	{
		switch(*itr)
		{
		case '/':
			if(got_dot)
				return;
			else
				base += *itr;
			break;
		case '.':
			got_dot = true;
		default:
			base += *itr;
			break;
		}
	}
}




void wcPageAnalyzer::perf_link_get(wcCache* cache, wcStringCell& url_o, wcMetaVector& meta_o ) 
{
	const char			href_tok[6] = "href=";
	const size_t		href_size   = 5;
	bool				href_found  = false;
	char				temp		= -1;

	url_o.clear();
	cache->set_beg();
	while(cache->buffer_good() && url_o.size() < 1000UL )
	{
		if(cache->next()==(*href_tok))
		{
			href_found = true;
			for( size_t idx(1); idx < href_size; idx++ )
			{
				if(cache->next()!=href_tok[idx])
				{
					href_found = false;
					break;
				}
			}
			if(href_found)
			{
				url_o.emplace_back();
				
				cache->_inc();
				while(href_found)
				{	
					switch(cache->curr())
					{
					case '\"':
					case '\'':
						href_found = false;
						if(!url_o.back().empty())
						{
							switch(url_o.back().front())
							{
								case '/':
									if(url_o.back().size()>=2 && url_o.back()[1]=='/')
										url_o.back() = "http:" + url_o.back();
									else
										url_o.back() = base + url_o.back();
									break;
								case '#':
								case '{':
									url_o.pop_back();
							}
						} else
							url_o.pop_back();

						break;
					default:
						if(cache->buffer_good() && !url_o.empty() )
							url_o.back().push_back(cache->curr());
						else
							return;
					}
					cache->_inc();
				}
			}
		}
	}
	meta_o[2] = 1.0f - expf( -(wcFloat)url_o.size()/WC_PA_LINKDEP ); 
}
