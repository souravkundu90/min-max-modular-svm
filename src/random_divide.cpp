#include "random_divide.h"
#include "iostream"
#include "time.h"
using namespace std;
void Random_Divide::divide(Data_Sample** data_sample,int sample_length,int subset_size,float label, 
		  vector<Data_Sample**> &ranked_sample,vector<vector<Divide_Info> >&all_info,char* file)
{
	vector<Divide_Info> divide_info;
	srand(time(0));
	double* rank_value = new double[sample_length];
	for(int i = 0; i<sample_length;i++)
	{
		rank_value[i] = rand();
	}
	quick_sort(rank_value,data_sample,0,sample_length); //modified by zhifei.ye from bubble sort to quick sort Mar.12, 2008.
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

	double rangeUP = 0;
	double rangeDOWN = 0;
	double rangeCURRENT = 0;
	size_t vecSize = divide_info.size();
	for(int i=0; i<vecSize; ++i)
	{
		rangeCURRENT = (i == 0)? (divide_info[i].end_offset - divide_info[i].start_offset+1): rangeDOWN;
		rangeDOWN = (i == vecSize - 1)? 0: (divide_info[i+1].end_offset - divide_info[i+1].start_offset+1);
		Divide_Info &tempCUR = divide_info[i];
		if(rangeUP>0.0001)
		{
			tempCUR.start_offset -= size_t(precent * rangeUP);
		}
		if(rangeDOWN>0.0001)
		{
			tempCUR.end_offset += size_t(precent * rangeDOWN);;
		}
		tempCUR.length = tempCUR.end_offset - tempCUR.start_offset + 1;
		rangeUP = rangeCURRENT;
	}

	delete d_info;
	delete[] rank_value;
	all_info.push_back(divide_info);
	Data_Sample** new_sample = new Data_Sample*[sample_length];
	for(int i =0;i<sample_length;i++)
		new_sample[i] = data_sample[i];
	ranked_sample.push_back(new_sample);
}

