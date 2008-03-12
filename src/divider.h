#ifndef _Divider_H
#define _Divider_H
#include "util.h"
#include <vector>
using namespace std;


class Divider{
protected:
  double precent;
public:
	
void quick_sort(double* rank_value,Data_Sample** data_sample,int start,int end)
{
	if(start == end)
		return;
	int i = start -1;
	int j = end +1;
	double mid = rank_value[start];
	do{
		do{
			i++;
		}while(rank_value[i]<mid);
		do{
			j--;
		}while(rank_value[j]>mid);
		if(i<j)
		{
			double temp = rank_value[i];
			rank_value[i] = rank_value[j];
			rank_value[j] = temp;
			Data_Sample* p_temp = data_sample[i];
			data_sample[i] = data_sample[j];
			data_sample[j] = p_temp;
		}
	}while(i<j);
	quick_sort(rank_value,data_sample,start,j);
	quick_sort(rank_value,data_sample,j +1,end);
}
  virtual void divide(Data_Sample** data_sample,int sample_length,int subset_size,float label, 
		  vector<Data_Sample**> &,vector<vector<Divide_Info> >&,char* file)=0;
  void parse(char*);
  virtual ~Divider(){}
};
#endif
