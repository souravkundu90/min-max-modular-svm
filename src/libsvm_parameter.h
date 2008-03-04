#ifndef _LibSVM_Parameter_H
#define _LibSVM_Parameter_H
#include "svm.h"
#include "util.h"
#include "parameter.h"
class libsvm_parameter:public Classifier_Parameter
{
public:
	svm_parameter param;
	libsvm_parameter(void);
	~libsvm_parameter(void);
	void Parse(int argc, char* argv[]);
	void print_help();
};

#endif
