#include "wcTypes.hpp"



wcFloat wcMetaVector::operator*=( const wcMetaVector& other )
{ 
	return
	p_density	*other.p_density	+
	p_quality	*other.p_quality	+
	p_relevence	*other.p_relevence; 
}


void wcMetaVector::operator*=( const wcFloat& scale )			
{ 
	p_density	*=scale; 
	p_quality	*=scale; 
	p_relevence	*=scale; 
}


void wcMetaVector::operator/=( const wcFloat& scale )			
{ 
	p_density	/=scale; 
	p_quality	/=scale; 
	p_relevence	/=scale; 
}


void wcMetaVector::operator+=( const wcMetaVector& other )	
{ 
	p_density	+=other.p_density; 
	p_quality	+=other.p_quality; 
	p_relevence	+=other.p_relevence; 
}


void wcMetaVector::operator-=( const wcMetaVector& other )	
{ 
	p_density	-=other.p_density; 
	p_quality	-=other.p_quality; 
	p_relevence	-=other.p_relevence; 
}


wcMetaVector::wcMetaVector() : 
	p_density(0),
	p_quality(0),
	p_relevence(0)
{}


wcMetaVector::~wcMetaVector() 
{}