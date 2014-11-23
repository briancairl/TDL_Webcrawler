
#include	"wcMath.hpp"





wcCostEstimator::wcCostEstimator( 
	const wcFloat& learning_rate,
	const wcFloat& regularization, 
	const wcFloat& sensivity, 
	const wcFloat& bias 
) :
	learning_rate(learning_rate),
	regularization(regularization),
	bias(bias),
	iteration(0),
	input_layer(3,10,sensivity),
	hidden1_layer(10,10,sensivity),
	hidden2_layer(10,10,sensivity),
	output_layer(10,3,sensivity),
	Vt(0),Vtp(0),rt(0),dVt(0), 
	training(true),
	unseeded(true)
{
	optima.setOnes();
	optima_expref.setOnes();
}


wcCostEstimator::~wcCostEstimator()
{

}




wcFloat wcCostEstimator::cost( const wcMetaVector& meta )
{
	const Eigen::Matrix

	for( wcPos idx = 0; idx < 3; idx++ )
		if(meta(idx)<WC_PRECISION) return 1.0f;

	wcMetaVector delta0 = (meta - optima);
	wcMetaVector delta1 = (meta - wcMetaVector().Ones());
	
	return delta.cwiseProduct(delta).sum()/3.0f;
}




void wcCostEstimator::update( const wcMetaVector& meta, const wcFloat& nla )
{
	wcFloat cost0 = cost(meta);
	wcFloat step  = 0;

	iteration++;
	if(unseeded)
	{
		optima_expref = meta;
		unseeded	  = false;
	}
	
	input_layer		.activate(meta);
	hidden1_layer	.activate(input_layer);
	hidden2_layer	.activate(hidden1_layer);
	output_layer	.activate(hidden2_layer);
	optima			= output_layer.output;

	Vtp	= Vt;
	rt	= 1-cost0;
	Vt  = Vt*(1-learning_rate*cost0) + cost0*learning_rate*(rt + nla*Vtp);
	dVt = Vt-Vtp;

	if( training && (dVt < WC_ENDOFTRAINING) && (iteration > 5UL) )
	{
		training = false;
	}
	else if( training && (dVt>0) )
	{
		optima_expref	+= rt*(meta-optima_expref);
		output_layer	.reweightOutput(optima_expref,	learning_rate);
		hidden2_layer	.reweightHidden(output_layer,	learning_rate);
		hidden1_layer	.reweightHidden(hidden2_layer,	learning_rate);
		input_layer		.reweightHidden(hidden1_layer,	learning_rate);

		output_layer	.reweightSet();
		hidden2_layer	.reweightSet();
		hidden1_layer	.reweightSet();
		input_layer		.reweightSet();
	}
}



