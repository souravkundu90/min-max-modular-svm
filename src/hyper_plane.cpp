#include "hyper_plane.h"
#include "iostream"
using namespace std;


void Hyper_Plane::divide(Data_Sample** data_sample,int sample_length,int subset_size,float label, 
		  vector<Data_Sample**> & ranked_sample,vector<vector<Divide_Info> > & all_info,char* file)

{
	vector<Divide_Info> divide_info;
	double* rank_value = new double[sample_length];
	for(int i = 0; i<sample_length;i++)
	{
		rank_value[i] =0;
	}

	for(int i = 0;i<sample_length;i++)
	{
		Data_Sample * sample = data_sample[i];
		Data_Node* vector = sample->data_vector;
		int length = sample->data_vector_length;
		for(int j = 0; j< sample->data_vector_length;j++)
		{
			rank_value[i] += vector[j].value;
		}
	}
	for(int i = 0;i<sample_length;++i)
	{
		for(int j =i+1;j<sample_length;++j)
		{
			if(rank_value[i]>rank_value[j])
			{
				double temp = rank_value[i];
				rank_value[i] = rank_value[j];
				rank_value[j] = temp;
				Data_Sample* p_temp = data_sample[i];
				data_sample[i] = data_sample[j];
				data_sample[j] = p_temp;
			}
		}
	}

	//quick_sort(rank_value,data_sample,0,sample_length); //modified by zhifei.ye from bubble sort to quick sort Mar.12, 2008.

	int subset_num =(int)(sample_length/(double)subset_size+0.5);
	double average_size = sample_length/(double)subset_num;
	int count = 0;
	Divide_Info* d_info = new Divide_Info();
	for(int i = 0;i<subset_num-1;i++)
	{
		d_info->start_offset = count;
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
	size_t up_end_index = 0;
	for(int i=0; i<vecSize; ++i)
	{
		rangeUP = (i == 0)? 0 : rangeCURRENT;
		rangeCURRENT = (i == 0)? (rank_value[divide_info[i].end_offset] - rank_value[divide_info[i].start_offset]): rangeDOWN;
		rangeDOWN = (i == vecSize - 1)? 0: (rank_value[divide_info[i+1].end_offset] - rank_value[divide_info[i+1].start_offset]);
		Divide_Info &tempCUR = divide_info[i];
		if(rangeUP>0.0001)
		{
			int j = up_end_index;
			double bound = rank_value[j] - precent * rangeUP;
			while(rank_value[j--]>bound)
			{
				--(tempCUR.start_offset);
			}
		}
		up_end_index = tempCUR.end_offset;
		if(rangeDOWN>0.0001)
		{
			Divide_Info &tempDOWN = divide_info[i+1];
			int j = tempDOWN.start_offset;
			double bound = rank_value[j] + precent * rangeDOWN;
			while(rank_value[j++]<bound)
			{
				++(tempCUR.end_offset);
			}
		}
		tempCUR.length = tempCUR.end_offset - tempCUR.start_offset + 1;
	}
	delete d_info;
	delete[] rank_value;
	all_info.push_back(divide_info);
	Data_Sample** new_sample = new Data_Sample*[sample_length];
	for(int i =0;i<sample_length;i++)
		new_sample[i] = data_sample[i];
	ranked_sample.push_back(new_sample);
}

