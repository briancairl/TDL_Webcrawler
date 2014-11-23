#include "wcANN.hpp"


wcLayer::wcLayer( const wcSize n, const wcSize m, const wcFloat sensitivity ) :
	sensitivity(sensitivity),
	weights(m,n),
	bias(m,1),
	d_weights(m,n),
	d_bias(m,1),
	input(n,1),
	output(m,1),
	error(m,1),
	dirac(m,1)
{
	weights		= weights.setRandom().cwiseAbs()/1000;
	bias		= bias.setRandom().cwiseAbs()	/1000;
	d_weights	.setZero();
	d_bias		.setZero();
	input		.setZero();
	output		.setZero();
	error		.setZero();
}



wcLayer::~wcLayer()
{

}



void wcLayer::activate( const wcLayer& last )
{
	this->input  = last.output;
	this->output = weights*input;
	for( wcPos idx = 0; idx < output.rows(); idx++ )
		output(idx) = (1.0f/(1.0f + expf(-sensitivity*(output(idx)+bias(idx)))));
}




void wcLayer::activate( const VectorXf& input )
{
	this->input	 = input;
	this->output = weights*input;
	for( wcPos idx = 0; idx < output.rows(); idx++ )
		output(idx) = (1.0f/(1.0f + expf(-sensitivity*(output(idx)+bias(idx)))));
}




void wcLayer::reweightOutput( const VectorXf& target, const wcFloat& lr )
{
	wcFloat moment(lr/2.0f);

	error = output-target;
	for( wcPos idx = 0; idx < output.rows(); idx++ )
	{		
		dirac(idx) = output(idx)*( 1.0f - output(idx) )*error(idx);
		d_bias(idx)=-lr*dirac(idx) + moment*d_bias(idx);

		for( wcPos jdx = 0; jdx < input.rows(); jdx++ )
			d_weights(idx,jdx) = -lr*dirac(idx)*input(jdx) + moment*d_weights(idx,jdx);
	}
}



void wcLayer::reweightHidden( const wcLayer& last, const wcFloat& lr )
{
	wcFloat moment(lr/2.0f);

	error = last.weights.transpose()*last.dirac;
	for( wcPos idx = 0; idx < output.rows(); idx++ )
	{		
		dirac(idx) = output(idx)*( 1.0f - output(idx) )*error(idx);
		d_bias(idx)=-lr*dirac(idx) + moment*d_bias(idx);

		for( wcPos jdx = 0; jdx < input.rows(); jdx++ )
			d_weights(idx,jdx) = -lr*dirac(idx)*input(jdx) + moment*d_weights(idx,jdx);
	}
}



void wcLayer::reweightSet()
{
	weights += d_weights;
	bias    += d_bias;
}