#include "prior_divide.h"
#include "iostream"
#include <fstream>
#include <time.h>
using namespace std;

//#define DEBUG
//#define DEBUG1

void Prior_Divide::divide(Data_Sample** data_sample,int sample_length,int subset_size,float label, 
		  vector<Data_Sample**> & ranked_sample,vector<vector<Divide_Info> > & all_info,char* file)

{
	double* rank_value = new double[sample_length];
	for(int i = 0; i<sample_length;i++)
	{
		rank_value[i] =0;
	}
	ifstream divide_info_file(file,ios::in);
	int infonum = 0,MOD = 0;
	bool ifrandom = 0;
	divide_info_file >> infonum >> MOD >> ifrandom;
	int* prior_info = new int[infonum];
	for(int i=0;i<infonum;++i)
	{
		divide_info_file >> prior_info[i];
	}
	divide_info_file.close();
	vector<Divide_Info> divide_info;
	for(int i=0;i<sample_length;++i)
	{
		rank_value[i] = prior_info[data_sample[i]->index];
	}
	quick_sort(rank_value,data_sample,0,sample_length-1); //modified by zhifei.ye from bubble sort to quick sort Mar.12, 2008.
	delete[] prior_info;
										
	int subset_num =(int)(sample_length/(double)subset_size+0.5);
	double average_size = sample_length/(double)subset_num;
	int count = 0;
	Divide_Info* d_info = new Divide_Info();
	for(int i = 0;i<subset_num-1;i++)
	{
		d_info->start_offset = count;++++++++
		d_info->end_offset =(int)(average_size*(i+1)-1);
		d_info->length = d_info->end_offset-d_info->start_offset+1;
		count = d_info->end_offset+1;
		divide_info.push_back(*d_info);
	}
	d_info->start_offset = count;
	d_info->end_offset = sample_length-1;
	d_info->length = d_info->end_offset - d_info->start_offset +1;
	divide_info.push_back(*d_info);

	delete d_info;
	delete[] rank_value;
	all_info.push_back(divide_info);
	Data_Sample** new_sample = new Data_Sample*[sample_length];
	for(int i =0;i<sample_length;i++)
		new_sample[i] = data_sample[i];
	ranked_sample.push_back(new_sample);
}
