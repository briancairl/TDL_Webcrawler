#ifndef		WC_MATH_HPP
#define		WC_MATH_HPP

#include	"wcANN.hpp"
#include	"wcGlobal.hpp"
#include	"wcTypes.hpp"
#include	"wcDef.hpp"
	
	using namespace Eigen;

	#define WC_PRECISION		1e-3
	#define WC_ENDOFTRAINING	1e-5

	class wcCostEstimator
	{
	private:
		bool				training;
		bool				unseeded;

		wcPos				iteration;
		wcFloat				regularization;
		wcFloat				bias;
		wcFloat				learning_rate;

		wcLayer				input_layer;
		wcLayer				hidden1_layer;
		wcLayer				hidden2_layer;
		wcLayer				output_layer;

		Eigen::Matrix3f		Qc;
		Eigen::Matrix3f		Rc;
		wcMetaVector		optima;
		wcMetaVector		optima_expref;
		wcFloat				Vt;
		wcFloat				Vtp;
		wcFloat				rt;
		wcFloat				dVt;
	public:
		wcCostEstimator( 
			const wcFloat& learning_rate	= 0.5f,
			const wcFloat& regularization	= 1.0f, 
			const wcFloat& sensivity		= 5.0f, 
			const wcFloat& bias				= 1.0f
		);
		~wcCostEstimator();

		void				update( const wcMetaVector& meta, const wcFloat& nla = 2.0f );
		wcFloat				cost( const wcMetaVector& meta );
		wcFloat				value( const wcMetaVector& meta );
		wcFloat				D_VT()			{ return dVt; }
		wcFloat				VT()			{ return Vt; }
		wcFloat				RMSE()			{ return output_layer.RMSE(); }
	};



#endif