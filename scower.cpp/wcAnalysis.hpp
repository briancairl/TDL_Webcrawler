#ifndef		WC_ANALYSIS_HPP
#define		WC_ANALYSIS_HPP
	
	#include	<fstream>

	#include	"wcTypes.hpp"
	#include	"wcDef.hpp"
	#include	"wcTag.hpp"
	#include	"wcGlobal.hpp"
	#include	"wcCache.hpp"

	#define		WC_NO_TEXT_DUMP		0
	#define		WC_PA_LINKDEP		500.0f

	class wcPageAnalyzer
	{
	friend class wcSession;
	private:
		wcRangeVector		text_ranges;
		wcString			base;
		void				make_base( wcCache* cache );

		void				perf_link_get( wcCache* cache, wcStringCell& url_o, wcMetaVector& meta_o );
		void				perf_body_get( wcCache* cache );
		void				perf_quality_analysis( wcCache* cache, wcMetaVector& meta_o );
		void				perf_relevance_analysis( wcCache* cache, const wcStringCell& query_vector, wcMetaVector& meta_o);
		void				perf_text_dump( wcCache* cache, wcMetaVector& meta_o );

	public:
		wcPageAnalyzer( wcSession& session, wcCache* cache, const wcStringCell& query_vector,
			wcMetaVector& meta_o,	
			wcStringCell& url_o
		);
		~wcPageAnalyzer();
	};


#endif