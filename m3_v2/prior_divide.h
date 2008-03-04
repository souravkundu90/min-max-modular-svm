#ifndef Prior_Divide_H
#define Prior_Divide_H
#include "util.h"
#include "divider.h"
class Prior_Divide:public Divider
{
public :
 // vector<Divide_Info> divide(Data_Sample ** data_sample, int sample_length,int subset_size);
  void divide(Data_Sample** data_sample,int sample_length,int subset_size,float label, 
		  vector<Data_Sample**>&,vector<vector<Divide_Info> >&,char* file);
};

#endif
