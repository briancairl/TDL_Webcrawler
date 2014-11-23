#include	"netBuffer.hpp"
#include	<mutex>

namespace	netPrimitives
{
	namespace global
	{
		static bool			init(false);
		static netString	cache_root;	
	}


	void globalInit(const netPath root)
	{
		global::init = true;

		/// Force-Create Cache Root
		_mkdir(root);	

		/// Set Cache-root
		global::cache_root = root;

		/// Init CURL
		curl_global_init(CURL_GLOBAL_ALL);
	}



	netMeta::netMeta() :
		download_time(0),
		download_size(0),
		download_speed(0),
		resolution_time(0),
		connection_time(0)
	{}


	netMeta::~netMeta() {}



	void netMeta::create( CURL* handle )
	{
		curl_easy_getinfo(handle, CURLINFO_TOTAL_TIME		, &download_time);
		curl_easy_getinfo(handle, CURLINFO_SIZE_DOWNLOAD	, &download_size);
		curl_easy_getinfo(handle, CURLINFO_SPEED_DOWNLOAD	, &download_speed);
		curl_easy_getinfo(handle, CURLINFO_NAMELOOKUP_TIME	, &resolution_time);
		curl_easy_getinfo(handle, CURLINFO_CONNECT_TIME		, &connection_time);
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
		return ((netBuffer*)userp)->add_data((netData)contents,(size * nmemb));
	}




	netBuffer::netBuffer( 
		const netURL&	address, 
		netSize			preemption 
	) :	
		address(address),
		cache_name(address), 
		size(0), 
		loc(0), 
		mem(NULL),
		state_flags(NULL),
		#if NET_CACHE_DISABLE_PREALLOC
		capacity(0),
		#else
		capacity(NET_DEFAULT_BUFFER_CAP),
		#endif
		preemption(preemption),
		absolute_path(global::cache_root)
	{
		#ifdef NET_DEBUG
		if(global::init)
		{
		#endif 
		cache_resolution_thread = new netThread(&netBuffer::cache_resolution_service,this);
		cache_resolution_thread->detach();
		#ifdef NET_DEBUG
		}
		else
			abort();
		#endif
	}


	netBuffer::netBuffer( netBuffer*& other ):	
		address(other->address),
		cache_name(other->cache_name), 
		size(other->size), 
		loc(0), 
		mem(other->mem),
		state_flags(NET_FLAG_MASK(adopted)|NET_FLAG_MASK(no_service)|NET_FLAG_MASK(written)),
		#if NET_CACHE_DISABLE_PREALLOC
		capacity(other->capacity),
		#else
		capacity(NET_DEFAULT_BUFFER_CAP),
		#endif
		preemption(0),
		absolute_path(global::cache_root)
	{
		other->locked = true;
	}


	netBuffer::~netBuffer()
	{
		while(NET_FLAG_IS_SET(state_flags,writing));

		cache_dump();
		strip_service();

		if(mem && !locked)
		{
			free(mem);
		}
	}


	bool netBuffer::strip_service()
	{
		if(NET_FLAG_IS_SET(state_flags,writing)||NET_FLAG_IS_SET(state_flags,no_service))
			return false;
		if(cache_resolution_thread)
		{
			delete cache_resolution_thread;
			cache_resolution_thread = NULL;
			
			NET_FLAG_SET(state_flags,no_service);
		}
		return (cache_resolution_thread==NULL);
	}



	void netBuffer::cache_resolution_service()
	{
		#ifdef NET_DEBUG
		if(global::init)
		{
		#endif
			
			cache_name_gen();

			#if !NET_CACHE_DISABLE_PREALLOC
			mem	= (netdata)malloc(capacity*sizeof(char));
			if(mem == NULL)
				NET_FLAG_SET(state_flags,netBuffer::states::heap_error);
			else
			#endif
			{
				NET_FLAG_SET(state_flags,writing);
				{
				#if !NET_CACHE_DISABLE_CACHELOAD
				if(cache_get())
					NET_FLAG_SET(state_flags,file_exists);
				else
				#endif
					curl_get();
				}
				NET_FLAG_CLEAR(state_flags,writing);
				NET_FLAG_SET(state_flags,written);
			}

		#ifdef NET_DEBUG
		}
		else
			abort();
		#endif
		
	}



	netSize netBuffer::add_data(const netData data, const netSize& ulen)
	{
		const size_t new_size(size + ulen);

		if( new_size > capacity )
		{
			capacity += ulen + preemption;
			mem	= (char*)realloc(mem,capacity);
			
			if(mem == NULL)
			{
				NET_FLAG_SET(state_flags,heap_error);
				return 0;
			}
		}
		
		memcpy(&(mem[size]), data, ulen);
		size = new_size;
		return ulen;
	}



	void netBuffer::cache_name_gen()
	{
		for( netString::iterator tag_itr = cache_name.begin(); tag_itr != cache_name.end(); ++tag_itr )
		{
			if((*tag_itr>255)||(*tag_itr<0)||!isalnum(*tag_itr))
				*tag_itr = '_';
		}
		absolute_path += "\\" + cache_name += NET_CACHE_FILE_EXT;
	}



	bool netBuffer::cache_get()
	{
		FILE*	cache_file(fopen(absolute_path.c_str(),"r"));
		char	cache_buffer[180];
		bool	cache_good(false);

		if(cache_file)
		{
			NET_FLAG_SET(state_flags,file_exists);
			cache_good = true;
			while(!feof(cache_file))
				add_data(cache_buffer,fread(cache_buffer,sizeof(char),180,cache_file));

			fclose(cache_file);
		}
		return cache_good;
	}




	void netBuffer::cache_dump()
	{
		if(size && NET_FLAG_IS_CLEAR(state_flags,file_exists) )
		{
			FILE*	cache_file(fopen(absolute_path.c_str(),"w+"));
			if(cache_file)
			{
				fwrite(mem,sizeof(char),size,cache_file);
				fclose(cache_file);
			}
		}
	}




	bool netBuffer::curl_get()
	{
		CURL*		curlh(curl_easy_init());

		if(curlh) 
		{
		
			curl_easy_setopt(curlh, CURLOPT_URL, 			address.c_str()		);
			curl_easy_setopt(curlh, CURLOPT_WRITEFUNCTION, 	curl_write			); 
			curl_easy_setopt(curlh, CURLOPT_WRITEDATA,    	this				);
			curl_easy_setopt(curlh, CURLOPT_USERAGENT, 		NET_LIBCURL_AGENT	);
			curl_easy_setopt(curlh, CURLOPT_TIMEOUT_MS,		NET_LIBCURL_TIMEOUT ); 
			curl_easy_setopt(curlh, CURLOPT_NOPROGRESS,		1L);
			curl_easy_setopt(curlh, CURLOPT_FOLLOWLOCATION, 1L);

			switch(curl_easy_perform(curlh))
			{
				case CURLE_COULDNT_CONNECT:
					NET_FLAG_SET(state_flags,connection_failure);
					break;
				case CURLE_COULDNT_RESOLVE_HOST:
					NET_FLAG_SET(state_flags,reshost_failure);
					break;
				case CURLE_COULDNT_RESOLVE_PROXY:
					NET_FLAG_SET(state_flags,resproxy_failure);
					break;
				case CURLE_OPERATION_TIMEDOUT:
					NET_FLAG_SET(state_flags,timeout_failure);
					break;
				case CURLE_OK:
					break;
				default:
					break;
			}
			
			meta.create(curlh);

			curl_easy_cleanup(curlh); 

			return true;
		}
		return false;
	}

}


