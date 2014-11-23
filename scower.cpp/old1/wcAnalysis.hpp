#ifndef		WC_ANALYSIS_HPP
#define		WC_ANALYSIS_HPP
	
	#include	<fstream>

	#include	"wcTypes.hpp"
	#include	"wcDef.hpp"
	#include	"wcTag.hpp"
	#include	"wcGlobal.hpp"
	#include	"wcCache.hpp"

	#define		WC_NO_TEXT_DUMP		0


	namespace wcAlgorithm
	{
		void	wcOptimaRefShow();
		void	wcOptimaRefUpdate( const wcMetaVector& node_meta );
		wcFloat wcOptimaDistance	( const wcMetaVector& node_meta );
	}




	class wcPageAnalyzer
	{
	private:
		wcRangeVector		text_ranges;
		wcString			base;
		void				make_base( wcCache& cache );

		void				perf_link_get( wcCache& cache );
		void				perf_body_get( wcCache& cache );
		void				perf_quality_analysis( wcCache& cache );
		void				perf_relevance_analysis( wcCache& cache, const wcStringCell& query_vector );
		void				perf_text_dump( wcCache& cache );

	public:
		wcPageAnalyzer( wcCache& cache, const wcStringCell& query_vector );
		~wcPageAnalyzer();
		
		struct
		{
			wcFloat			quality;
			wcFloat			relevance;
			wcFloat			density;
			wcStringCell	urls;
		} results;
	};


#endif