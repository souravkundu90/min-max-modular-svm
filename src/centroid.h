#ifndef _Centroid_H
#define _Centroid_H
#include "util.h"
#include "divider.h"
class Centroid:public Divider
{
public :
  void divide(Data_Sample** data_sample,int sample_length,int subset_size,float label, 
		  vector<Data_Sample**>&,vector<vector<Divide_Info> >&,char* file);
};

#endif
