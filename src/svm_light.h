#ifndef _SVM_Light_H
#define _SVM_Light_H

#include "util.h"
#include "svm_light_parameter.h"
#include "classifier.h"
class svm_light_model
{
public:

	KERNEL_PARM kernel_param;
	int subset_num1;
	int subset_num2;
	int total_sv_count;
	int* sv_count;	
	DOC* SVs;
	int** sv_index;
	double** alphas;
	double* thresholds;
	bool deap_copy_words;
	int totwords;
	double ** lin_weights;
	int memory_size;
};
class svm_light: public Classifier
{
private:
	svm_light_parameter param;
	svm_light_model model;
public:
	svm_light(svm_light_parameter param);
	~svm_light(void);
	int train(Data_Sample** sample1,Data_Sample** sample2,
         int subset_num1, int subset_num2,
         int* subset_length1,int* subset_length2,const char* model_file );
	int save_model(const char* fileURL);
	int load_model(const char* fileURL);
	double* predict(Data_Sample* sample);
	void predict(Data_Sample* sample,int& x,int& y,float& direction);
	void predict(Data_Sample* sample,float* min);
	void free_model(); 
};

#endif
