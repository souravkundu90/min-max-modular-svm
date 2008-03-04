#ifndef _Divider_H
#define _Divider_H
#include "util.h"
#include <vector>
using namespace std;

class Divider{
protected:
  double precent;
public:
  virtual void divide(Data_Sample** data_sample,int sample_length,int subset_size,float label, 
		  vector<Data_Sample**> &,vector<vector<Divide_Info> >&,char* file)=0;
  void parse(char*);
  virtual ~Divider(){}
};
#endif
