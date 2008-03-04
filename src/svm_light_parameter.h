#ifndef _SVM_Light_Parameter_H
#define _SVM_Light_Parameter_H
extern "C"
{
# include "svm_common.h"
# include "svm_learn.h"
}

#include "parameter.h"
class svm_light_parameter:public Classifier_Parameter
{
public:
  long verbosity;
  long kernel_cache_size;
  LEARN_PARM learn_parm;
  KERNEL_PARM kernel_parm;
public:
	svm_light_parameter(void);
	~svm_light_parameter(void);
	void Parse(int argc, char* argv[]);
	void print_help(void);
};
#endif
