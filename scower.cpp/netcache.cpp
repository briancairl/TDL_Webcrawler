
#include	"netcache.hpp"
#include	<mutex>

namespace	netcache
{
	namespace global
	{
		static bool		init(false);
		static netstr	cache_root;	
	}


	void globalinit(const netPATH root)
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
	size_t curl_write_buffer(void* contents, size_t size, size_t nmemb, void* userp) 
	{
		return ((netNode*)userp)->add_data((netdata)contents,(size * nmemb));
	}




	netNode::netNode( 
		const netURL&	address, 
		const netflags	fb_args, 
		netsize preemption 
	) :	
		address(address),
		cache_name(address), 
		size(0), 
		loc(0), 
		mem(NULL),
		state_flags(NULL),
		feedback_flags(fb_args),
		#if NET_CACHE_DISABLE_PREALLOC
		capacity(0),
		#else
		capacity(NET_DEFAULT_BUFFER_CAP),
		#endif
		preemption(preemption),
		absolute_path(global::cache_root)
	{
		cache_resolution_thread = new netThread(&netNode::cache_resolution_service,this);
		cache_resolution_thread->detach();
	}



	netNode::~netNode()
	{
		while(NET_FLAG_IS_SET(state_flags,netNode::states::writing));

		cache_dump();
		if(cache_resolution_thread)
		{

			delete cache_resolution_thread;
		}
		if(mem)
		{
			free(mem);
		}
	}



	void netNode::get_base_address()
	{
		bool	got_dot(false);
		
		feedback.base_address.clear();
		for( netstr::const_iterator itr(address.begin()); itr!=address.end(); ++itr )
		{
				
			switch(*itr)
			{
			case '/':
				if(got_dot)
					return;
				else
					feedback.base_address += *itr;
				break;
			case '.':
				got_dot = true;
			default:
				feedback.base_address += *itr;
				break;
			}
		}
	}




	void netNode::get_edge_addresses()
	{
		const char		href_tok[6] = "href=";
		const size_t	href_size   = 5;
		static netstr	bass_addr;
		bool			href_found(false);
		char			temp(-1);


		get_base_address();
			
		set_loc(0);
		while(good())
		{
			if(next()==href_tok[0])
			{
				href_found = true;
				for( size_t idx(1); idx < href_size; idx++ )
				{
					if(next()!=href_tok[idx])
					{
						href_found = false;
						break;
					}
				}
				if(href_found)
				{
					feedback.edge_addresses.emplace_back();
					next();	// dummy-read {",'}
					while(href_found)
					{
						temp = next();
						switch(temp)
						{
						case -1:
						case '\"':
						case '\'':
							href_found = false;

							if(!feedback.edge_addresses.back().empty())
							{
								switch(feedback.edge_addresses.back().front())
								{
									case '/':
										feedback.edge_addresses.back() = bass_addr + feedback.edge_addresses.back();
										break;
									case '#':
									case '{':
										feedback.edge_addresses.pop_back();
								}
							} else {
								feedback.edge_addresses.pop_back();
							}


							break;
						default:
							feedback.edge_addresses.back().push_back(temp);
						}
					}
				}
			}
		}
	}



	void netNode::cache_resolution_service()
	{
		#ifdef NET_DEBUG
		if(global::init)
		{
		#endif
			
			cache_name_gen();

			#if !NET_CACHE_DISABLE_PREALLOC
			mem	= (netdata)malloc(capacity*sizeof(char));
			if(mem == NULL)
				NET_FLAG_SET(state_flags,netNode::states::heap_error);
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

			if(NET_FLAG_IS_SET(feedback_flags,hrefs))
				get_edge_addresses();

		#ifdef NET_DEBUG
		}
		else
			abort();
		#endif
		
	}



	netsize netNode::add_data(const netdata data, const netsize& ulen)
	{
		const size_t new_size(size + ulen);

		if( new_size > capacity )
		{
			capacity += ulen + preemption;
			mem	= (char*)realloc(mem,capacity);
			
			if(mem == NULL)
			{
				NET_FLAG_SET(state_flags,netNode::states::heap_error);
				return 0;
			}
		}
		
		memcpy(&(mem[size]), data, ulen);
		size = new_size;
		return ulen;
	}



	void netNode::cache_name_gen()
	{
		for( nettag::iterator tag_itr = cache_name.begin(); tag_itr != cache_name.end(); ++tag_itr )
		{
			if((*tag_itr<255)&&!isalnum(*tag_itr))
				*tag_itr = '_';
		}
		absolute_path += "\\" + cache_name += NET_CACHE_FILE_EXT;
	}



	bool netNode::cache_get()
	{
		FILE*	cache_file(fopen(absolute_path.c_str(),"r"));
		char	cache_buffer[180];
		bool	cache_good(false);

		if(cache_file)
		{
			NET_FLAG_SET(state_flags,netNode::states::file_exists);
			cache_good = true;
			while(!feof(cache_file))
				add_data(cache_buffer,fread(cache_buffer,sizeof(char),180,cache_file));

			fclose(cache_file);
		}
		return cache_good;
	}




	void netNode::cache_dump()
	{
		if(size && NET_FLAG_IS_CLEAR(state_flags,netNode::states::file_exists) )
		{
			FILE*	cache_file(fopen(absolute_path.c_str(),"w+"));
			if(cache_file)
			{
				fwrite(mem,sizeof(char),size,cache_file);
				fclose(cache_file);
			}
		}
	}




	bool netNode::curl_get()
	{
		CURL*		curlh(curl_easy_init());

		if(curlh) 
		{
		
			curl_easy_setopt(curlh, CURLOPT_URL, 			address.c_str()		);
			curl_easy_setopt(curlh, CURLOPT_WRITEFUNCTION, 	curl_write_buffer	); 
			curl_easy_setopt(curlh, CURLOPT_FILE,    		this				);
			curl_easy_setopt(curlh, CURLOPT_USERAGENT, 		NET_LIBCURL_AGENT	);
			curl_easy_setopt(curlh, CURLOPT_TIMEOUT_MS,		NET_LIBCURL_TIMEOUT ); 
			curl_easy_setopt(curlh, CURLOPT_FOLLOWLOCATION, 1L);

			switch(curl_easy_perform(curlh))
			{
				case CURLE_COULDNT_CONNECT:
					NET_FLAG_SET(state_flags,netNode::states::connection_failure);
					break;
				case CURLE_COULDNT_RESOLVE_HOST:
					NET_FLAG_SET(state_flags,netNode::states::reshost_failure);
					break;
				case CURLE_COULDNT_RESOLVE_PROXY:
					NET_FLAG_SET(state_flags,netNode::states::resproxy_failure);
					break;
				case CURLE_OPERATION_TIMEDOUT:
					NET_FLAG_SET(state_flags,netNode::states::timeout_failure);
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





	netEdgeAnalyzer::netEdgeAnalyzer( const netsize& limit ) :
		multi_service_thread(NULL),
		multi_service_running(false),
		multi_service_paused(false),
		limit(limit)
	{
		multi_service_thread = new std::thread(&netEdgeAnalyzer::multi_service,this); 
	}




	netEdgeAnalyzer::netEdgeAnalyzer( const netURL_multi& urls, const netsize& limit ) :
		multi_service_thread(NULL),
		multi_service_running(false),
		multi_service_paused(false),
		limit(limit),
		pending(urls)
	{
		multi_service_thread = new std::thread(&netEdgeAnalyzer::multi_service,this);
	}



	netEdgeAnalyzer::~netEdgeAnalyzer()
	{
		multi_service_running = 
		multi_service_paused  = false;
		
		// Remove the management service routine 
		while(!multi_service_thread->joinable());	
		multi_service_thread->join();
		multi_buffer.clear();
		delete multi_service_thread;

		// Free all nodes
		while(multi_buffer.size())
		{
			delete multi_buffer.front();
			multi_buffer.pop_front();
		}
	}



	void netEdgeAnalyzer::multi_service()
	{
		multi_service_running = true;
		while(multi_service_running)
		{
			if( (!multi_service_paused) && (multi_buffer.size() < limit) && !pending.empty() )
			{
				multi_buffer.emplace_back(new netNode(pending.front(),NET_FLAG_MASK(netNode::feedback_types::hrefs)));
				pending.pop_front();
			}
		}
	}




	bool netEdgeAnalyzer::prime_iterator() 
	{
		for( netmulti_itr temp_itr = multi_buffer.begin(); temp_itr!=multi_buffer.end(); ++temp_itr )
		{
			if( !(*temp_itr)->get_tagged_state() && (*temp_itr)->ready() )
			{
				(*temp_itr)->set_as_tagged();
				multi_buffer_itr = temp_itr;
				return true;
			}
		}
		return false;
	}


	const netNode* netEdgeAnalyzer::get_iterator() 
	{
		return (*multi_buffer_itr);
	}


	void netEdgeAnalyzer::erase_iterator()
	{
		if((*multi_buffer_itr)->get_tagged_state())
		{
			delete *multi_buffer_itr;
			multi_buffer.erase(multi_buffer_itr);
		}
	}

}


