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

  // This function is only for the multi-label that you must need the information of all label that is stored in vec<>all_label
  // For one-versus-rest:if the left label is _i means that _i versus rest, where _i is the major label, 
  // then the place of _j(!=_i) is the divide information of label _i when _j is major label where _i is the rest, 
  // and the place of _i is the divide information of label _i.
  void divide(Data_Sample** data_sample,int sample_length,int subset_size,float label, 
      vector<Data_Sample**> & vds,vector<vector<Divide_Info> >&vvdi,char* file,vector<float> all_label){
          // This code is only for test
          int num_label=all_label.size();
          vds.clear();
          for (int i=0;i<num_label;i++) vds.push_back(data_sample);

          vector<Divide_Info> vdi;
          vdi.clear();
          int ll=0,rr=0;
          rr=subset_size;
          for (;rr<sample_length;rr+=subset_size){
              Divide_Info di;
              di.start_offset=ll;
              di.end_offset=(sample_length<=rr)?sample_length-1:rr-1;
              di.length=di.end_offset-di.start_offset+1;
              vdi.push_back(di);
              ll=rr;
          }
          for (int i=0;i<num_label;i++) vvdi.push_back(vdi);

  };

  void parse(char*);
  virtual ~Divider(){}
};
#endif
