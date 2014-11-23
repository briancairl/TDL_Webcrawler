#ifndef		WC_SESSION_HPP
#define		WC_SESSION_HPP

#include	"wcGlobal.hpp"
#include	"wcTypes.hpp"
#include	"wcTag.hpp"
#include	"wcGraph.hpp"
#include	"wcMath.hpp"
#include	<fstream>



	class wcPathPoint
	{
	friend 
		std::ostream& operator<<( std::ostream& os, wcPathPoint& path_pt );
	public:
		wcString	url;
		wcFloat		time;
		wcFloat		leading_cost;
		wcFloat		relevance;
		wcFloat		RMSE;
		wcFloat		Vt;
		wcPathPoint( 
			const wcString& url, 
			const wcFloat& time, 
			const wcFloat& leading_cost,
			const wcFloat& relevance,
			const wcFloat& RMSE,
			const wcFloat& Vt
		) :
			url(url), time(time), leading_cost(leading_cost), relevance(relevance), RMSE(RMSE), Vt(Vt)
		{}

		~wcPathPoint()
		{}
	};

	typedef	std::list<wcPathPoint>	wcPathLog;





	class wcSession
	{

	#define	QV_T(n)				const wcString& qv##n
	#define	QV_EVAL(n)			if(!qv##n.empty()) query_vector.emplace_back(qv##n);

	friend class wcAgent;
	friend class wcPageAnalyzer;

	private:
		wcString				session_name;
		wcSize					max_steps;

		wcStringCell			query_vector;
		wcCostEstimator			cost_estimator;
	
		wcTime					epoch;
		wcPathLog				explored_path;
		wcStringCell			explored_urls;

		wcTeam 					agents;

		void					log_point();

	public:
		wcSession( 
			const wcString&		name,
			const wcURL&		url, 
			const wcFloat&		lr,
			const wcSize&		max_steps,
			QV_T(1)="", 
			QV_T(2)="", 
			QV_T(3)="", 
			QV_T(4)="",
			QV_T(5)="", 
			QV_T(6)="", 
			QV_T(7)="", 
			QV_T(8)=""
		);

		wcSession( 
			const wcString&		name,
			const wcURL&		url, 
			const wcFloat&		lr,
			const wcSize&		max_steps,
			const wcStringCell& queries 
		);

		~wcSession();
	
	
		bool 					step( wcSize end_at = 0 );
		bool					unexplored( const wcString& url );
	};



#endif