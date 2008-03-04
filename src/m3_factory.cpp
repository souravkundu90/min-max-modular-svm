#include <iostream>
using namespace std;
#include "m3_factory.h"
Classifier_Parameter* makeLibCP()
{
	return new libsvm_parameter();
}
Divider* makeHyperD()
{
	return new Hyper_Plane();
}
Divider* makeRandomD()
{
	return new Random_Divide();
}
Divider* makePriorD()
{
	return new Prior_Divide();
}
Divider* makeCentroiD(){
  return new Centroid();
}
Classifier_Parameter* makeLightCP()
{
	return new svm_light_parameter();
}
Classifier* makeSvmC(Classifier_Parameter* para)
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
Voter* makeHcV()
{
	return new HC_Voter();
}
M3_Factory::M3_Factory()
{
	register_new(1,1,1,1,makeLibCP,makeHyperD,makeSvmC,makeHcV);
	register_new(2,-1,2,-1,makeLightCP,NULL,make_svm_light,NULL);
	register_new(-1,2,-1,-1,NULL,makeRandomD,NULL,NULL);
	register_new(-1,3,-1,-1,NULL,makeCentroiD,NULL,NULL);
	register_new(-1,4,-1,-1,NULL,makePriorD,NULL,NULL);
}
M3_Factory& M3_Factory::instance()
{
	static M3_Factory ins;
	return ins;
}
void M3_Factory::register_new(	const int kind1,const int kind2,
								const int kind3,const int kind4,
										const pf_class_para a,
										const pf_divider b,
										const pf_classifier c,
										const pf_voter d)
{
	if(kind1 != -1)
	{
		pair<int,pf_class_para> p1(kind1,a);
		_decCP.insert(p1);
	}
	if(kind2 != -1)
	{
		pair<int,pf_divider> p2(kind2,b);
		_decD.insert(p2);
	}
	if(kind3 != -1)
	{
		pair<int,pf_classifier> p3(kind3,c);
		_decC.insert(p3);
	}
	if(kind4 != -1)
	{
		pair<int,pf_voter> p4(kind4,d);
		_decV.insert(p4);
	}
}
Classifier_Parameter* M3_Factory::create_parameter(const int kind)
{
	return _decCP[kind]();
}
Divider* M3_Factory::create_divider(const int kind)
{
	return _decD[kind]();
}
Classifier* M3_Factory::create_classifier(const int kind,
								Classifier_Parameter* para)
{
	return _decC[kind](para);
}
Voter* M3_Factory::create_voter(const int kind)
{
	return _decV[kind]();
}
