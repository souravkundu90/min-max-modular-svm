#ifndef M3_FACTORY_H
#define M3_FACTORY_H
#include <map>
using namespace std;
#include "libsvm.h"
#include "svm_light.h"
#include "hc_voter.h"
#include "hyper_plane.h"
#include "random_divide.h"
#include "prior_divide.h"
#include "centroid.h"
class M3_Factory
{
private :
	typedef Classifier_Parameter* (*pf_class_para)();
	typedef Divider* (*pf_divider)();
	typedef Classifier* (*pf_classifier)(Classifier_Parameter*);
	typedef Voter* (*pf_voter)();
public :
	static M3_Factory& instance();
	Classifier_Parameter* create_parameter(const int kind);
	Divider* create_divider(const int kind);
	Classifier* create_classifier(const int kind,
										Classifier_Parameter*);
	Voter* create_voter(const int kind);
	void register_new(	const int,const int,const int,const int ,
								const pf_class_para,
								const pf_divider,
								const pf_classifier,
								const pf_voter);
private :
	M3_Factory();
	map<int,pf_class_para> _decCP;
	map<int,pf_divider> _decD;
	map<int,pf_classifier> _decC;
	map<int,pf_voter> _decV;
};

#endif
