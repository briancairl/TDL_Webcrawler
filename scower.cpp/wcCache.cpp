#include "wcCache.hpp"


namespace wcMasks
{

	const wcFlags any_failure						= 
		wcMASK(wcCacheState::connection_failure)	|
		wcMASK(wcCacheState::resproxy_failure)		|
		wcMASK(wcCacheState::reshost_failure)		|
		wcMASK(wcCacheState::timeout_failure)		;

	const wcFlags header_retrieve_success			= 
		wcMASK(wcCacheState::header_retrieved)		|
		wcMASK(wcCacheState::service_pending)		;

	const wcFlags file_retrieve_success				= 
		wcMASK(wcCacheState::file_exists)			|
		wcMASK(wcCacheState::service_terminated)	;

}


/// @brief 	Cache-buffer write function for web-page pulls
///
///	@param	contents 	generic stream input contents
///	@param 	size		size of contents (noraml count)
///	@param 	nmemb		size of word in content stream
///	@param	userp		generic user output container
///
///	@return	size in bytes of content written to the stream
size_t curl_write(void* contents, size_t size, size_t nmemb, void* userp) 
{
	return static_cast<wcCache*>(userp)->write((wcBuffer)contents,(size * nmemb));
}




wcCache::wcCache( const wcTag& tag ) :
	tag_ref((wcTag*)(&tag)),
	size(0), 
	loc(0), 
	mem(NULL),
	state_flags(NULL),
	#if WC_DISABLE_CACHELOAD
	capacity(0)
	#else
	capacity(WC_CACHE_BUFFER_CAP)
	#endif
{
#if WC_DISABLE_THREAD
	cache_resolution_service();
#else
	if(wcGlobal::is_init())
	{
		cache_resolution_thread = new wcThread(&wcCache::cache_resolution_service,this);
		cache_resolution_thread->detach();
	}
	else
		abort();
#endif 
}



wcCache::~wcCache()
{
	#if !WC_DISABLE_THREAD
	while(wcIS_SET(state_flags,writing));
	#endif

	#if !WC_DISABLE_CACHEDUMP
	cache_dump();
	#endif

	#if !WC_DISABLE_THREAD
	free_service();
	#endif
	
	clear();
}


#if !WC_DISABLE_THREAD
bool wcCache::free_service()
{
	if((cache_resolution_thread==NULL) || wcIS_SET(state_flags,writing))
		return false;

	if(cache_resolution_thread)
	{
		delete cache_resolution_thread;
		cache_resolution_thread = NULL;
		

		wcSET(state_flags,service_terminated);
	}
	return (cache_resolution_thread==NULL);
}
#endif



void wcCache::cache_resolution_service()
{

	#if !WC_DISABLE_CACHELOAD
	mem	= (wcBuffer)malloc(capacity*sizeof(char));
	if(mem == NULL)
		wcSET(state_flags,heap_error);
	else
	{
	#endif

		wcSET(state_flags,writing);
		#if !WC_DISABLE_CACHELOAD
		if(cache_load())
		{
			state_flags |= wcMasks::file_retrieve_success;
		}
		else
		{
		#endif

			// If the service was not terminated, retrieve page content
			if(cache_curl_get_content())
			{
				wcSET(state_flags,content_retrieved);
			}

		#if !WC_DISABLE_CACHELOAD
		}
		#endif
		wcCLEAR(state_flags,writing);
		wcSET(state_flags,written);

	#if !WC_DISABLE_CACHELOAD
	}
	#endif
}



wcSize wcCache::write(const wcBuffer data, const wcSize& ulen)
{
	size_t new_size(size + ulen);
	if( new_size > capacity )
	{
		capacity += ulen + WC_CACHE_GROW_PREEMPTION;
		mem	= (char*)realloc(mem,capacity);
			
		if(mem == NULL)
		{
			wcSET(state_flags,heap_error);
			//abort();
			return 0;
		}
	}
	
	if( mem != NULL )
	{
		memcpy(&(mem[size]), data, ulen);
		size = new_size;
		return ulen;
	}
	return 0;
}



void wcCache::clear()
{
	if(mem && wcIS_CLEAR(state_flags,locked))
	{
		free(mem);
		mem = NULL;
	}
}



bool wcCache::cache_load()
{
	FILE*	cache_file(fopen(tag_ref->get_cache_filename().c_str(),"r"));
	char	cache_buffer[180];
	bool	cache_good(false);

	if(cache_file)
	{
		wcSET(state_flags,file_exists);
		cache_good = true;
		while(!feof(cache_file))
			write(cache_buffer,fread(cache_buffer,sizeof(char),180,cache_file));

		fclose(cache_file);
	}
	return cache_good;
}




void wcCache::cache_dump()
{
	if(mem&&(!(state_flags&wcMasks::any_failure))&&wcIS_CLEAR(state_flags,file_exists)&&(size>0))
	{
		FILE*	cache_file= NULL; 
		if(!fopen_s(&cache_file,tag_ref->get_cache_filename().c_str(),"w+"))
		{
			fwrite(mem,sizeof(char),size,cache_file);
			fclose(cache_file);
		}
	}
}




bool wcCache::cache_curl_get_header()
{
	CURL*		curlh(curl_easy_init());
	
	if(curlh) 
	{
		clear();
		curl_easy_setopt(curlh, CURLOPT_URL, 			tag_ref->get_url().c_str()	);
		curl_easy_setopt(curlh, CURLOPT_WRITEFUNCTION, 	curl_write					); 
		curl_easy_setopt(curlh, CURLOPT_WRITEHEADER,    this						);
		curl_easy_setopt(curlh, CURLOPT_USERAGENT, 		WC_LIBCURL_AGENT			);
		curl_easy_setopt(curlh, CURLOPT_TIMEOUT,		WC_LIBCURL_TIMEOUT			); 
		curl_easy_setopt(curlh, CURLOPT_NOPROGRESS,		1L							);

		switch(curl_easy_perform(curlh))
		{
			case CURLE_COULDNT_CONNECT:
				wcSET(state_flags,connection_failure);
				break;
			case CURLE_COULDNT_RESOLVE_HOST:
				wcSET(state_flags,reshost_failure);
				break;
			case CURLE_COULDNT_RESOLVE_PROXY:
				wcSET(state_flags,resproxy_failure);
				break;
			case CURLE_OPERATION_TIMEDOUT:
				wcSET(state_flags,timeout_failure);
				break;
			case CURLE_OK:
				break;
			default:
				break;
		}
			
		curl_easy_cleanup(curlh); 

		return true;
	}
	return false;
}



bool wcCache::cache_curl_get_content()
{
	CURL*		curlh(curl_easy_init());
	
	if(curlh) 
	{
		clear();
		curl_easy_setopt(curlh, CURLOPT_URL, 			tag_ref->get_url().c_str()	);
		curl_easy_setopt(curlh, CURLOPT_WRITEFUNCTION, 	curl_write					); 
		curl_easy_setopt(curlh, CURLOPT_WRITEDATA,    	this						);
		curl_easy_setopt(curlh, CURLOPT_USERAGENT, 		WC_LIBCURL_AGENT			);
		curl_easy_setopt(curlh, CURLOPT_TIMEOUT_MS,		WC_LIBCURL_TIMEOUT   		); 
		curl_easy_setopt(curlh, CURLOPT_NOPROGRESS,		1L							);
		curl_easy_setopt(curlh, CURLOPT_FOLLOWLOCATION, 1L							);

		switch(curl_easy_perform(curlh))
		{
			case CURLE_COULDNT_CONNECT:
				wcSET(state_flags,connection_failure);
				break;
			case CURLE_COULDNT_RESOLVE_HOST:
				wcSET(state_flags,reshost_failure);
				break;
			case CURLE_COULDNT_RESOLVE_PROXY:
				wcSET(state_flags,resproxy_failure);
				break;
			case CURLE_OPERATION_TIMEDOUT:
				wcSET(state_flags,timeout_failure);
				break;
			case CURLE_OK:
				break;
			default:
				break;
		}
			
		curl_easy_cleanup(curlh); 

		return true;
	}
	return false;
}