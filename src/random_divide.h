#ifndef RANDOM_DIVIDE
#define RANDOM_DIVIDE
#include "util.h"
#include "divider.h"
class Random_Divide:public Divider
{
public :
  void divide(Data_Sample** data_sample,int sample_length,int subset_size,float label, 
		  vector<Data_Sample**> &,vector<vector<Divide_Info> >&,char* file);
private :
};
#endif
