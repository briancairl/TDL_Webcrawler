#ifndef WC_GLOBAL_HPP
#define WC_GLOBAL_HPP

	#include	"wcTypes.hpp"
	#include	"wcDef.hpp"
	#include	"wcTag.hpp"
	#include	<fstream>

	#define		WC_STATE_FOLDER_NAME		"state"
	#define		WC_CACHE_FOLDER_NAME		"cache"
	#define		WC_CONTENT_FOLDER_NAME		"content"
	#define		WC_SINFO_FOLDER_NAME		"session_info"
	#define		WC_SINFO_LOG_NAME			"logger"


	namespace wcGlobal
	{	
		void					init( const wcPath& root );
		const bool				is_init();
		const wcString&			get_cache_root();
		const wcString&			get_state_root();
		const wcString&			get_content_root();
		const wcString&			get_sinfo_root();
		extern std::ofstream	log_out;
	}

#endif