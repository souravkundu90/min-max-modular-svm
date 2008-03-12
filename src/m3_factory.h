#ifndef M3_FACTORY_H
#define M3_FACTORY_H
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
public :
	static Classifier_Parameter* create_parameter(const int kind);
	static Divider* create_divider(const int kind);
	static Classifier* create_classifier(const int kind,Classifier_Parameter*);
	static Voter* create_voter(const int kind);
};

#endif
