#ifndef _Hyper_Plane_H
#define _Hyper_Plane_H
#include "util.h"
#include "divider.h"
class Hyper_Plane:public Divider
{
public :
  void divide(Data_Sample** data_sample,int sample_length,int subset_size,float label, 
		  vector<Data_Sample**>&,vector<vector<Divide_Info> >&,char* file);
  void multi_label_divide(Data_Sample** data_sample,int sample_length,int subset_size,float label, 
      vector<Data_Sample**> & vds,vector<vector<Divide_Info> >&vvdi,char* file,vector<float> all_label);
private :
};

#endif
