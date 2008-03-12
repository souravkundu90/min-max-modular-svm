#include <iostream>
using namespace std;
#include "m3_factory.h"

Classifier* make_libsvm(Classifier_Parameter* para)
{
	libsvm_parameter *tempLP = dynamic_cast<libsvm_parameter*>(para);
	if(tempLP == 0)
	{
		cout<<"Parameter is not fitable!"<<endl;
		return 0;
	}
	return new libsvm(*tempLP);
}
Classifier* make_svm_light(Classifier_Parameter* para)
{
	svm_light_parameter * tempLP = dynamic_cast<svm_light_parameter*>(para);
	if(tempLP ==0)
	{
		cout<<"Parameter is not fitable!"<<endl;
		return 0;
	}
	return new svm_light(*tempLP);
}


Classifier_Parameter* M3_Factory::create_parameter(const int kind)
{
	switch(kind)
	{
	case 1:
		return new libsvm_parameter();
	case 2:
		return new svm_light_parameter();
	default:
		return NULL;
	}
}
Divider* M3_Factory::create_divider(const int kind)
{
	switch(kind)
	{
	case 1:
		return new Hyper_Plane();
	case 2:
		return new Random_Divide();
	case 3:
		return new Centroid();
	case 4:
		return new Prior_Divide();
	default:
		return NULL;
	}
}
Classifier* M3_Factory::create_classifier(const int kind,
								Classifier_Parameter* para)
{
	switch(kind) 
	{
	case 1:
		return make_libsvm(para); 
	case 2:
		return make_svm_light(para);
	default:
		return NULL;
	}
}
Voter* M3_Factory::create_voter(const int kind)
{
	return new HC_Voter();
}
