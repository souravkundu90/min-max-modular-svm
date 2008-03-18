#ifndef _DIVIDER_CPP
#define _DIVIDER_CPP

#include "divider.h"
#include <fstream>
#include <iostream>
using namespace std;

void Divider::quick_sort(double* rank_value,Data_Sample **data_sample,int start,int end)
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
void Divider::parse(char* filename)
{
	try
	{
		ifstream inf(filename);
		char in,iin;
		while(inf>>in)
		{
			switch(in)
			{
				case '-':
				{
					inf>>iin;
					switch(iin)
					{
						case 'o':
						{
							inf>>precent;
							break;
						}
					}
					break;
				}
	
			}
		}
		inf.close();
	}
	catch(...)
	{
		precent = 0;
		return;
	}
}

void Divider::divide_one(Data_Sample **data_sample, double *rank_value, int sample_length, int subset_size, std::vector<Divide_Info>& divide_info)
{
	quick_sort(rank_value,data_sample,0,sample_length-1); //modified by zhifei.ye from bubble sort to quick sort Mar.12, 2008.

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
}
#endif
