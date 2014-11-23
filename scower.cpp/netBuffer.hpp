/// @file	netcache.hpp
/// @brief	This library contains funtions for retrieving webcontent source using libcurl
///			services, as well as provides rudimentary source-caching services.
///
///
#ifndef		NETBUFFER_HPP
#define		NETBUFFER_HPP

#include	"netTypedefs.hpp"

namespace	netPrimitives
{
	using namespace netTypes;

	#define						NET_DEBUG
	#define						NET_LIBCURL_AGENT			"libcurl-agent/1.0"
	#define						NET_LIBCURL_TIMEOUT			5500		// ms

	#define						NET_DEFAULT_BUFFER_PREEMPT	10
	#define						NET_DEFAULT_BUFFER_CAP		100
	#define						NET_DEFAULT_CACHE_LOCATION	""

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
	void globalInit(const netPath root = NET_DEFAULT_CACHE_LOCATION);




	/// @brief	Meta-data for downloaded blocks
	class netMeta
	{
	private:
		double					download_time;
		double					download_size;
		double					download_speed;
		double					resolution_time;
		double					connection_time;
	public:
		netMeta();
		~netMeta();
		inline const double&	get_download_size()		{ return download_size; }
		inline const double&	get_download_time()		{ return download_time; }
		inline const double&	get_download_speed()	{ return download_speed; }
		inline const double&	get_resolution_time()	{ return resolution_time; }
		inline const double&	get_connection_time()	{ return connection_time; }

		void					create( CURL* handle );
	};




	///	@brief	A smart-buffer for the cache
	class netBuffer
	{
	friend 
		size_t					curl_write(void* contents, size_t size, size_t nmemb, void* userp);
	protected:
		
		typedef enum
		{
			writing	= 0,		///< Buffer is currently being written to
			written,			///< Buffer writing has completed
			heap_error,			///< Buffer memory allocation request failed
			file_error,			///< Buffer contents could not be written to a file
			file_exists,		///< Buffer contents were pulled from an exitsing cache file
			reshost_failure,	///< CURL could not resolve host
			resproxy_failure,	///< CURL could not resolve proxy
			connection_failure,	///< CURL could not form a connection
			timeout_failure,	///< CURL timedout during resolution
			no_service,			///< Associated thread was deleted to free-up memory
			adopted
		}	netBufferState;
		
		netFlags				state_flags;
		netString				address;
		netString				cache_name;
		netString				absolute_path;
		netSize					preemption;
		netSize					capacity;
		netSize 				size;
		netSize					loc;
		netData					mem;
		netMeta					meta;
		bool					locked;

		netThread*				cache_resolution_thread;
		void					cache_resolution_service();
		void					cache_name_gen();
		bool					cache_get();
		void					cache_dump();
		bool					curl_get();

		netSize					add_data(const netData data, const netSize& ulen);
		
	public:

		netBuffer(
			const netURL&		address,
			netSize				preemption	=	NET_DEFAULT_BUFFER_PREEMPT
		);

		netBuffer( netBuffer*&	other );

		~netBuffer();

		inline bool				operator< ( netBuffer& other )	{ return size < other.size; }
		inline bool				operator> ( netBuffer& other )	{ return size > other.size; }
		inline bool				operator<=( netBuffer& other )	{ return size <=other.size; }
		inline bool				operator>=( netBuffer& other )	{ return size >=other.size; }
		inline bool				operator==( netBuffer& other )	{ return address == other.address; }

		inline void				set_loc( netOffset& idx )		{ loc = idx; }
		inline const netSize&	get_loc()						{ return loc; }
		inline const netSize&	get_size()						{ return size; }
		inline const netFlags&	get_flags()						{ return state_flags; }
		inline const netSize&	get_capacity()					{ return capacity; }
		inline const netString&	get_address()					{ return address; }
		inline netMeta&			get_meta()						{ return meta; }
		inline const char&		operator[]( netOffset& idx )	{ return (loc<size) ? (mem[idx])   : (-1); }
		inline const char&		next()							{ return (loc<size) ? (mem[loc++]) : (-1); }
		inline bool				has_content()					{ return (bool)(size); }
		inline bool				error()							{ return (bool)(NET_FLAG_IS_SET(state_flags,heap_error)	||NET_FLAG_IS_SET(state_flags,file_error)); }
		inline bool				ready()							{ return (bool)NET_FLAG_IS_SET(state_flags,written); }
		inline bool				timedout()						{ return (bool)NET_FLAG_IS_SET(state_flags,timeout_failure); }
		inline bool				resolving()						{ return (bool)NET_FLAG_IS_SET(state_flags,writing); }
		inline bool				good()							{ return (loc<size); }
		inline netSize			space()							{ return (capacity-size); }
		bool					strip_service();
	};

}
#endif
