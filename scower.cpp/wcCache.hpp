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
	#define		WC_CACHE_BUFFER_CAP			10

	#define		WC_VALID_CACHE_PTR(cptr)	((cptr!=NULL) && (cptr)->get_size())

	typedef enum wcxCacheState
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
		void					set_beg()						{ loc = 0; }
		void					set_end()						{ loc = size-1; }
		void					set_loc( wcPos& idx )			{ loc = idx; }
		wcTag*					get_tag()						{ return tag_ref;}
		const wcPos&			get_loc()						{ return loc; }
		const wcSize&			get_size()						{ return size; }
		const wcFlags&			get_flags()						{ return state_flags; }
		const wcSize&			get_capacity()					{ return capacity; }
		const wcBuffer&			get_buffer()					{ wcSET(state_flags,locked); return mem; }	
		const void 				lock_buffer()					{ wcSET(state_flags,locked); }	
		const void 				unlock_buffer()					{ wcCLEAR(state_flags,locked); }	

		inline const char&		operator[]( wcPos& idx )		{ return (loc<size) ? (mem[idx])    : (WC_CACHE_FAILURE_BYTE); }
		inline const char&		next()							{ return (loc<size) ? (mem[loc++])  : (WC_CACHE_FAILURE_BYTE); }
		inline const char&		last()							{ return (loc>0)	? (mem[loc--])  : (WC_CACHE_FAILURE_BYTE); }
		inline const char&		curr()							{ return mem[loc];							}
		inline void				operator++()					{ (loc<size)		? (++loc) : (NULL);		}
		inline void				operator--()					{ (loc>0)			? (--loc) : (NULL);		}
		inline void				_inc()							{ (loc<size)		? (++loc) : (NULL);		}
		inline void				_dec()							{ (loc>0)			? (--loc) : (NULL);		}
				
		inline bool				buffer_valid()					{ return mem!=NULL; }
		inline bool				buffer_good()					{ return (loc<size); }
		inline wcSize			buffer_space()					{ return (capacity-size); }
		inline bool				buffer_has_content()			{ return (size>0); }

		inline wcFlags			service_error()					{ return (wcIS_SET(state_flags,heap_error)||wcIS_SET(state_flags,file_error)); }
		inline wcFlags			service_timedout()				{ return (wcIS_SET(state_flags,timeout_failure)); }
		inline wcFlags			service_is_ready()				{ return (wcIS_SET(state_flags,written)); }
		inline wcFlags			service_is_active()				{ return (wcIS_SET(state_flags,writing)); }
	
		
		#if !WC_DISABLE_THREAD
		bool					free_service();
		#endif
	};

#endif