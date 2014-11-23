#include "wcGlobal.hpp"
#include <direct.h>



namespace wcGlobal
{
	bool			cache_init = false;

	wcString		state_root;
	wcString		cache_root;
	wcString		content_root;
	wcString		sinfo_root;
	std::ofstream	log_out;

	void init( const wcPath& root )
	{
		wcGlobal::cache_init	= true;

		wcGlobal::sinfo_root	=
		wcGlobal::state_root	= 
		wcGlobal::cache_root	= 
		wcGlobal::content_root	= root;

		wcGlobal::sinfo_root	+= '\\';
		wcGlobal::state_root	+= '\\';
		wcGlobal::cache_root	+= '\\';
		wcGlobal::content_root	+= '\\';

		wcGlobal::sinfo_root	+= WC_SINFO_FOLDER_NAME;
		wcGlobal::state_root	+= WC_STATE_FOLDER_NAME;
		wcGlobal::cache_root	+= WC_CACHE_FOLDER_NAME;
		wcGlobal::content_root	+= WC_CONTENT_FOLDER_NAME;

		wcGlobal::sinfo_root	+= '\\';
		wcGlobal::state_root	+= '\\';
		wcGlobal::cache_root	+= '\\';
		wcGlobal::content_root	+= '\\';

		_mkdir(root);
		_mkdir(wcGlobal::sinfo_root.c_str());
		_mkdir(wcGlobal::state_root.c_str());
		_mkdir(wcGlobal::cache_root.c_str());
		_mkdir(wcGlobal::content_root.c_str());

		log_out.open((wcGlobal::sinfo_root+WC_SINFO_LOG_NAME+".log").c_str());
	}


	const bool is_init()				{ return cache_init;	}
	const wcString&	get_cache_root()	{ return cache_root;	}
	const wcString&	get_state_root()	{ return state_root;	}
	const wcString&	get_content_root()	{ return content_root;	}
	const wcString&	get_sinfo_root()	{ return sinfo_root;	}
}