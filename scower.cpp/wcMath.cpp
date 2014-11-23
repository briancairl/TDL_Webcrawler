
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

	Qc.setIdentity();
	Rc.setIdentity();
}


wcCostEstimator::~wcCostEstimator()
{

}




wcFloat wcCostEstimator::cost( const wcMetaVector& meta )
{
	const Eigen::Vector3f V1(Eigen::Vector3f().Ones());
/*
	for( wcPos idx = 0; idx < 3; idx++ )
		if(meta(idx)<WC_PRECISION) return 1.0f;

	wcMetaVector delta0 = (meta - optima);
	wcMetaVector delta1 = (meta - V1);
	wcFloat		 maxQR	= ((V1.transpose()*Qc*V1 + V1.transpose()*Rc*V1)(0));

	return (delta0.transpose()*Qc*delta0 + delta1.transpose()*Rc*delta1)(0)/maxQR;
*/
	for( wcPos idx = 0; idx < 3; idx++ )
		if(meta(idx)<WC_PRECISION) return 1.0f;

	wcMetaVector delta0 = (meta - optima);
	wcFloat		 maxQR	= ((V1.transpose()*Qc*V1))(0);

	return (delta0.transpose()*Qc*delta0)(0)/maxQR;
	
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


	/// Value Function Update
	Vtp	= Vt;
	rt  = 1-cost0;
	dVt = learning_rate*(rt - Vtp);
	Vt += dVt; 



	/// Predicted Optimal Document Update
	input_layer		.activate(meta);
	hidden1_layer	.activate(input_layer);
	hidden2_layer	.activate(hidden1_layer);
	output_layer	.activate(hidden2_layer);
	optima			= output_layer.output;



	if( training && (RMSE() < WC_ENDOFTRAINING) && (iteration > 5UL) )
	{
		training = false;
	}

	
	if( training /*&& (dVt>0)*/ )
	{
		optima_expref	+= dVt*(meta-optima_expref);
		output_layer	.reweightOutput(meta,			learning_rate);
		hidden2_layer	.reweightHidden(output_layer,	learning_rate);
		hidden1_layer	.reweightHidden(hidden2_layer,	learning_rate);
		input_layer		.reweightHidden(hidden1_layer,	learning_rate);

		output_layer	.reweightSet();
		hidden2_layer	.reweightSet();
		hidden1_layer	.reweightSet();
		input_layer		.reweightSet();
	}
}



