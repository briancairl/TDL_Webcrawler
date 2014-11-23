/// @file	netcache.hpp
/// @brief	This library contains funtions for retrieving webcontent source using libcurl
///			services, as well as provides rudimentary source-caching services.
///
///
#ifndef		NETCACHE_HPP
#define		NETCACHE_HPP

#include	"nettypedefs.hpp"

namespace	netcache
{
	using namespace	nettypes;

	#define						NET_DEBUG
	#define						NET_LIBCURL_AGENT			"libcurl-agent/1.0"
	#define						NET_LIBCURL_TIMEOUT			1500		// ms

	#define						NET_DEFAULT_BUFFER_PREEMPT	10
	#define						NET_DEFAULT_BUFFER_CAP		100
	#define						NET_DEFAULT_CACHE_LOCATION	""
	#define						NET_DEFAULT_MULTILIMIT		10

	#define						NET_CACHE_FILE_EXT			".ccf"
	#define						NET_CACHE_DISABLE_CACHELOAD	0 
	#define						NET_CACHE_DISABLE_PREALLOC	1
	
	#define						NET_FLAG_MASK(n)			(1<<n)
	#define						NET_FLAG_SET(flags,n)		flags|=(1<<n)
	#define						NET_FLAG_CLEAR(flags,n)		flags&=~(1<<n)
	#define						NET_FLAG_IS_SET(flags,n)	(flags&(1<<n))
	#define						NET_FLAG_IS_CLEAR(flags,n)	(!NET_FLAG_IS_SET(flags,n))




	/// @brief	Initializes cache global members
	///			This function must be called at the program start
	void globalinit(const netPATH root = NET_DEFAULT_CACHE_LOCATION);




	/// @brief	Meta-data for downloaded blocks
	class netMeta
	{
	private:
		double			download_time;
		double			download_size;
		double			download_speed;
		double			resolution_time;
		double			connection_time;
	public:
		netMeta();
		~netMeta();
		void			create( CURL* handle );
	};




	///	@brief	A smart-buffer for the cache
	class netNode
	{
	friend 
		size_t					curl_write_buffer(void* contents, size_t size, size_t nmemb, void* userp);
	private:
		
		typedef enum
		{
			writing	= 0,		///< Flags that buffer is currently being written to
			written,			///< Flags that buffer writing has ceased
			heap_error,			///< Flags that buffer memory request failed
			file_error,			///< Flags that buffer contents could not be written to a file
			file_exists,		///< Flags that buffer contents were pulled from an exitsing file
			reshost_failure,
			resproxy_failure,
			connection_failure,
			timeout_failure,
			tagged
		} states;
		
		netflags				feedback_flags;
		netflags				state_flags;

		nettag					address;
		nettag					cache_name;
		nettag					absolute_path;
		netsize					preemption;
		netsize					capacity;
		netsize 				size;
		netsize					loc;
		netdata					mem;
		netMeta					meta;

		netThread*				cache_resolution_thread;
		void					cache_resolution_service();
		void					cache_name_gen();
		bool					cache_get();
		void					cache_dump();
		bool					curl_get();

		netsize					add_data(const netdata data, const netsize& ulen);
		
		void					get_base_address();
		void					get_edge_addresses();
		
	public:

		typedef enum
		{
			hrefs	= 0
		} feedback_types;

		struct
		{
			netURL_multi		edge_addresses;
			netURL				base_address;
		} feedback;

		netNode(
			const netURL&		address,
			const netflags		fb_args,
			netsize				preemption	=	NET_DEFAULT_BUFFER_PREEMPT
		);
		
		~netNode();

		inline void				set_as_tagged()					{ NET_FLAG_SET(state_flags,tagged); }
		inline bool				clear_tagged_state()			{ NET_FLAG_CLEAR(state_flags,tagged); }
		inline bool				get_tagged_state()				{ return (bool)NET_FLAG_IS_SET(state_flags,tagged); }

		inline void				set_loc( netoffset& idx )		{ loc = idx; }
		inline const netsize&	get_loc()						{ return loc; }
		inline const netsize&	get_size()						{ return size; }
		inline const netflags&	get_flags()						{ return state_flags; }
		inline const netsize&	get_capacity()					{ return capacity; }
		inline const nettag&	get_address()					{ return address; }
		inline const char&		operator[]( netoffset& idx )	{ return (loc<size) ? (mem[idx])   : (-1); }
		inline const char&		next()							{ return (loc<size) ? (mem[loc++]) : (-1); }
		inline bool				has_content()					{ return (bool)(size); }
		inline bool				error()							{ return (bool)(NET_FLAG_IS_SET(state_flags,heap_error)||NET_FLAG_IS_SET(state_flags,file_error)); }
		inline bool				ready()							{ return (bool)NET_FLAG_IS_SET(state_flags,written); }
		inline bool				resolving()						{ return (bool)NET_FLAG_IS_SET(state_flags,writing); }
		inline bool				good()							{ return (loc<size); }
		inline netsize			space()							{ return (capacity-size); }

	};




	class netEdgeAnalyzer
	{
	typedef std::list<netNode*>		netmultiprim;
	typedef netmultiprim::iterator	netmulti_itr;
	private:
		netsize					limit;
		netURL_multi			pending;
		
		netmultiprim			multi_buffer;
		netmulti_itr			multi_buffer_itr;
		netThread*				multi_service_thread;
		void					multi_service();
		bool					multi_service_running;
		bool					multi_service_paused;

	public:
		netEdgeAnalyzer( 
			const netsize&		limit		=	NET_DEFAULT_MULTILIMIT 
		);
		netEdgeAnalyzer( 
			const netURL_multi& urls, 
			const netsize&		limit		=	NET_DEFAULT_MULTILIMIT 
		);
		~netEdgeAnalyzer();
		
		bool					prime_iterator();
		void					erase_iterator();
		const netNode*			get_iterator();
		const netNode*			get_front()						{ return multi_buffer.front(); }
		const netNode*			get_back()						{ return multi_buffer.back(); }
		void					pop_front()						{ delete multi_buffer.front();	multi_buffer.pop_front(); }
		void					pop_back()						{ delete multi_buffer.back();	multi_buffer.back(); }
		inline void				operator+=( const netURL& url )	{ pending.emplace_back(url); }
		inline void				set_limit( const netsize& lim )	{ limit = lim; }
		inline void				clear_pending()					{ pending.clear(); }
		inline void				clear_active()					{ multi_buffer.clear(); }
		inline netsize			size()							{ return multi_buffer.size(); }
		inline netsize			pending_size()					{ return pending.size(); }
	};

};


#endif
