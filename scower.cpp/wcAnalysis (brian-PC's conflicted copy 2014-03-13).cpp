#include	"wcAnalysis.hpp"


wcPageAnalyzer::wcPageAnalyzer( wcCache& cache, const wcStringCell& query_vector ) 
{
	make_base(cache);
	perf_link_get(cache);
	perf_body_get(cache);
	perf_quality_analysis(cache);
	perf_relevance_analysis(cache,query_vector);

	#if !WC_NO_TEXT_DUMP
	perf_text_dump(cache);
	#endif
}



wcPageAnalyzer::~wcPageAnalyzer() 
{
}




void wcPageAnalyzer::perf_body_get(wcCache& cache) 
{
	cache.set_beg();
	bool opened(false);
	while(cache.buffer_good())
	{
		if(cache.next()=='<')
		{
			switch(cache.next())
			{
			case '/':
				// </p>
				if( opened && (cache.next()=='p') && (cache.next()=='>') )
				{
					opened = false;
					text_ranges.back().second = cache.get_loc()-4; // length of </p>
				}	break;
			case 'p':
				// <p>
				if( !opened && (cache.next()=='>') )
				{
					opened = true;
					text_ranges.emplace_back(cache.get_loc(),0);
				}	break;
			}
		}
	}
}




void wcPageAnalyzer::perf_quality_analysis( wcCache& cache )
{
	wcSize text_ulen(0);
	for( wcRangeVector::const_iterator itr = text_ranges.cbegin(); itr != text_ranges.cend(); ++itr )
	{
		text_ulen += itr->second-itr->first;
	}
	results.quality = ((wcFloat)(text_ulen))/((wcFloat)(cache.get_size()));
}




void	wcPageAnalyzer::perf_relevance_analysis( wcCache& cache, const wcStringCell& query_vector )
{
	wcSize hit_count	= 0;
	wcSize ws_count		= 0;
	wcPos  idx			= 0;
	results.relevance	= 0;

	if( !text_ranges.empty() && !query_vector.empty() )
	{
		/// Perform Space (Word) Count 
		for( wcRangeVector::const_iterator itr = text_ranges.cbegin(); itr != text_ranges.cend(); ++itr )
		{
			cache.set_loc(const_cast<wcPos&>(itr->first));
			while( cache.get_loc() < itr->second )
			{
				if( cache.next() == ' ' )
					++ws_count;
			}
		}

		/// Perform Query Match Count 
		for( wcStringCell::const_iterator query = query_vector.begin(); query != query_vector.end(); ++query )
		{
			for( wcRangeVector::const_iterator itr = text_ranges.cbegin(); itr != text_ranges.cend(); ++itr )
			{
				cache.set_loc(const_cast<wcPos&>(itr->first));
				while( cache.get_loc() < itr->second )
				{
					if( cache.next() == query->front() )
					{
						for( idx = 1; idx < query->size(); idx++ )
							if( (*query)[idx] != cache.next() ) break;
				
						if( idx == query->size() )
							++hit_count;
					}
				}
			}
		}

		results.relevance = (ws_count) ? ((wcFloat)hit_count)/((wcFloat)ws_count) : (1.0f);
	}
}




void wcPageAnalyzer::perf_text_dump( wcCache& cache )
{
	std::ofstream	dump_file(cache.get_tag().get_text_filename().c_str());
	
	if(dump_file.is_open())
	{
		bool open(false);
		cache.set_beg();
		for( wcRangeVector::const_iterator itr = text_ranges.cbegin(); itr != text_ranges.cend(); ++itr )
		{
			cache.set_loc(const_cast<wcPos&>(itr->first));
			while( cache.get_loc() < itr->second )
			{
				++cache;
				if(cache.curr()=='<')
					open = true;
				else 
				if(cache.curr()=='>')
					open = false;
				else 
				if(!open)
					dump_file<<cache.curr();
			}	
			dump_file<<std::endl;
		}
	}
}



void wcPageAnalyzer::make_base(wcCache& cache)
{
	bool got_dot(false);
	for( wcString::const_iterator itr = cache.get_tag().get_url().begin(); itr!=cache.get_tag().get_url().end(); ++itr )
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




void wcPageAnalyzer::perf_link_get(wcCache& cache) 
{
	const char			href_tok[6] = "href=";
	const size_t		href_size   = 5;
	bool				href_found  = false;
	char				temp		= -1;

	cache.set_beg();
	while(cache.buffer_good())
	{
		if(cache.next()==(*href_tok))
		{
			href_found = true;
			for( size_t idx(1); idx < href_size; idx++ )
			{
				if(cache.next()!=href_tok[idx])
				{
					href_found = false;
					break;
				}
			}
			if(href_found)
			{
				results.urls.emplace_back();
				
				cache++;
				while(href_found)
				{	
					switch(cache.curr())
					{
					case -1:
					case '\"':
					case '\'':
						href_found = false;
						if(!results.urls.back().empty())
						{
							switch(results.urls.back().front())
							{
								case '/':
									if(results.urls.back().size()>=2 && results.urls.back()[1]=='/')
										results.urls.back() = "http:" + results.urls.back();
									else
										results.urls.back() = base + results.urls.back();
									break;
								case '#':
								case '{':
									results.urls.pop_back();
							}
						} else
							results.urls.pop_back();

						break;
					default:
						results.urls.back().push_back(cache.curr());
					}
					cache++;
				}
			}
		}
	}
}
