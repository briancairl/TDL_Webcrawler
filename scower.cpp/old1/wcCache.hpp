#ifndef		WC_CACHE_HPP
#define		WC_CACHE_HPP
	
	#include	"curl\curl.h"
	#include	"wcTypes.hpp"
	#include	"wcDef.hpp"
	#include	"wcTag.hpp"
	#include	"wcGlobal.hpp"

	class		wcCache;
	class		wcHeader; 

	#define		WC_DISABLE_CACHELOAD		0
	#define		WC_DISABLE_CACHEDUMP		0
	#define		WC_DISABLE_THREAD			1

	#define		WC_LIBCURL_AGENT			"libcurl-agent/1.0"
	#define		WC_LIBCURL_TIMEOUT			3000

	#define		WC_CACHE_FAILURE_BYTE		0x05
	#define		WC_CACHE_GROW_PREEMPTION	10
	#define		WC_DEFAULT_BUFFER_CAP		10



	typedef enum
	{
		locked	= 0,		///< Buffer content is locked
		writing,			///< Buffer is currently being written to
		written,			///< Buffer writing has completed
		heap_error,			///< Buffer memory allocation request failed
		file_error,			///< Buffer contents could not be written to a file
		file_exists,		///< Buffer contents were pulled from an exitsing cache file
		reshost_failure,	///< CURL could not resolve host
		resproxy_failure,	///< CURL could not resolve proxy
		connection_failure,	///< CURL could not form a connection
		timeout_failure,	///< CURL timedout during resolution
		service_terminated,	///< Resolution service was ended
		service_pending,	///< Resolution service is awaiting confirmation to download
		header_retrieved,	///< Resolution service obtained header data
		content_retrieved	///< Resolution service obtained page content
	}	wcCacheState;



	class wcCache
	{
	friend 
		size_t					curl_write(void* contents, size_t size, size_t nmemb, void* userp);
	protected:
				
		wcTag*					tag_ref;					
		wcFlags					state_flags;
		wcSize					capacity;
		wcSize 					size;
		wcPos					loc;
		wcBuffer				mem;

		#if !WC_DISABLE_THREAD
		wcThread*				cache_resolution_thread;
		#endif
		void					cache_resolution_service();
		bool					cache_load();
		void					cache_dump();
		bool					cache_curl_get_header();
		bool					cache_curl_get_content();

		wcSize					write(const wcBuffer data, const wcSize& ulen);
		
	public:

		wcCache( const wcTag& address );
		~wcCache();

		void					clear();
		inline void				set_beg()						{ loc = 0; }
		inline void				set_end()						{ loc = size-1; }
		inline void				set_loc( wcPos& idx )			{ loc = idx; }
		inline wcTag&			get_tag()						{ return *tag_ref;}
		inline const wcPos&		get_loc()						{ return loc; }
		inline const wcSize&	get_size()						{ return size; }
		inline const wcFlags&	get_flags()						{ return state_flags; }
		inline const wcSize&	get_capacity()					{ return capacity; }
		inline const wcBuffer&	get_buffer()					{ wcSET(state_flags,locked); return mem; }	
		inline const void 		lock_buffer()					{ wcSET(state_flags,locked); }	
		inline const void 		unlock_buffer()					{ wcCLEAR(state_flags,locked); }	

		inline const char&		operator[]( wcPos& idx )		{ return (loc<size) ? (mem[idx])    : (WC_CACHE_FAILURE_BYTE); }
		inline const char&		next()							{ return (loc<size) ? (mem[loc++])  : (WC_CACHE_FAILURE_BYTE); }
		inline const char&		last()							{ return (loc>0)	? (mem[loc--])  : (WC_CACHE_FAILURE_BYTE); }
		inline const char&		curr()							{ return mem[loc];							}
		inline void				operator++()					{ (loc<size)		? (++loc) : (NULL);		}
		inline void				operator--()					{ (loc>0)			? (--loc) : (NULL);		}

		inline bool				buffer_good()					{ return (loc<size); }
		inline wcSize			buffer_space()					{ return (capacity-size); }
		inline bool				buffer_has_content()			{ return (bool)(size); }

		inline bool				service_error()					{ return (bool)(wcIS_SET(state_flags,heap_error)||wcIS_SET(state_flags,file_error)); }
		inline bool				service_timedout()				{ return (bool)(wcIS_SET(state_flags,timeout_failure)); }
		inline bool				service_is_ready()				{ return (bool)(wcIS_SET(state_flags,written)); }
		inline bool				service_is_active()				{ return (bool)(wcIS_SET(state_flags,writing)); }
		
		#if !WC_DISABLE_GETHEADER
		inline bool				service_is_pending()			{ return (bool)(wcIS_SET(state_flags,service_pending)); }
		inline bool				service_is_terminated()			{ return (bool)(wcIS_SET(state_flags,service_terminated)); }
		inline void				terminate_service()				{ wcCLEAR(state_flags,service_pending); wcSET(state_flags,service_terminated); }
		inline void				permit_service()				{ wcCLEAR(state_flags,service_pending); }
		#endif
		
		#if !WC_DISABLE_THREAD
		bool					free_service();
		#endif
	};

#endif