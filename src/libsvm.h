#ifndef _LibSVM_H
#define _LibSVM_H
#include "libsvm_parameter.h"
#include "classifier.h"
struct m3_libsvm_submodel{
	svm_parameter param;
	int l;             //total number of svs in subset_num1*subset_num2 sub problems.
	int subset_num1; 
	int subset_num2;
	svm_node** SVs;    //length = l;
	int* n_sv;			//length = subset_num1 + subset_num2
	int* sv_start; 	//length = subset_num1 + subset_num2;
	double** svm_coef; //subset_num1*subset_num2 sub models, length = n_sv[i] + n_sv[j]
	double* rho; 	//subset_num1*subset_num2 sub models
	int memory_size; //memory requirment to store all these stuff.
};
class libsvm : public Classifier
{
private:
	libsvm_parameter param;
	m3_libsvm_submodel model;
public:
	libsvm(libsvm_parameter param);
	~libsvm(void);
	
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
