#ifndef		WC_ANN_HPP
#define		WC_ANN_HPP

	#include	"wcGlobal.hpp"
	#include	"wcTypes.hpp"
	#include	"wcDef.hpp"
	
	using namespace Eigen;


	class wcLayer
	{
	private:
		wcFloat	sensitivity;
		Eigen::Matrix<float,-1,-1> d_weights;
		Eigen::Matrix<float,-1,-1> d_bias;
		Eigen::Matrix<float,-1,-1> weights;
		Eigen::Matrix<float,-1,-1> bias;
		Eigen::Matrix<float,-1,-1> input;
		Eigen::Matrix<float,-1,-1> error;
		Eigen::Matrix<float,-1,-1> dirac;
	public:
		Eigen::Matrix<float,-1,-1> output;

		wcLayer( const wcSize n, const wcSize m, const wcFloat sensitivity=1.0f );
		~wcLayer();

		void	reweightOutput( const VectorXf& target, const wcFloat& lr);
		void	reweightHidden( const wcLayer& last,   const wcFloat& lr );
		void	reweightSet();
		void	activate( const wcLayer& last );
		void	activate( const VectorXf& input );
		wcFloat	RMSE() { return error.norm(); }
	};


#endif