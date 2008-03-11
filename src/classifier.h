#ifndef _CLASSIFIER_H
#define _CLASSIFIER_H

class Classifier
{
////////////////////////////////////added by hoss//////////////////////////////////
public :
	virtual int train(class Data_Sample**,class Data_Sample**,
				      int, int,int*,int*,const char*) = 0; /* return the subeset size */
	virtual int save_model(const char*) = 0;
	virtual int load_model(const char*) = 0;
	virtual double* predict(class Data_Sample*) = 0; 		/*normal prediction*/
	virtual void predict(class Data_Sample*,int & x,int & y,float & dirct) = 0; /*predictino using sematric pruning*/
	virtual void predict(class Data_Sample*,float* min) = 0;                /*asematric pruning. the length of the min array is equal to subset_num1*/
	virtual void free_model() = 0;
	virtual ~Classifier(){}
///////////////////////////////////////////////////////////////////////////////////
};

#endif
