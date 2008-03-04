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
	delete[] prior_info;

#ifdef DEBUG1
	for(int i=0;i<sample_length;++i)
	{
		cout<<data_sample[i]->index<<" ";
		cout<<static_cast<int>(rank_value[i])/MOD<<" ";
		cout<<static_cast<int>(rank_value[i])%MOD<<endl;
	}
	cout<<endl;
#endif
	vector<Divide_Info> temp_divide_info;
	Divide_Info* d_info = new Divide_Info();
	d_info->start_offset = 0;
	for(int i=0;i<sample_length-1;++i)
	{
		int up = static_cast<int>(rank_value[i])%MOD;
		int down = static_cast<int>(rank_value[i+1])%MOD;
		if(up!=down)
		{
			d_info->end_offset = i;
			d_info->length = d_info->end_offset-d_info->start_offset+1;
			temp_divide_info.push_back(*d_info);
			d_info->start_offset = i+1;
		}
	}
	d_info->end_offset = sample_length-1;
	d_info->length = d_info->end_offset-d_info->start_offset+1;
	temp_divide_info.push_back(*d_info);

#ifdef DEBUG1
	cout<<"Print divide info: "<<endl;	
	vector<Divide_Info>::iterator iter = temp_divide_info.begin();
	vector<Divide_Info>::iterator enditer = temp_divide_info.end();
	while(iter != enditer)
	{
		cout<<iter->start_offset<<" ";
		cout<<iter->end_offset<<" ";
		cout<<iter->length<<endl;
		++iter;
	}
#endif
	vector<Divide_Info>::iterator it = temp_divide_info.begin();
	vector<Divide_Info>::iterator end = temp_divide_info.end();
	if(ifrandom==1)
	{
		srand(time(0));
		for(;it != end;++it)
		{
			int mod_len = it->length;
			if(mod_len<=subset_size)
			{
				divide_info.push_back(*it);
				continue;
			}
			int b = it->start_offset,e = it->end_offset;
			int change_pos = 0;
			int modmod = mod_len;
			for(int i=b;i<e;++i)
			{
				change_pos = rand()%modmod+i;
				--modmod;
				double temp = rank_value[i];
				rank_value[i] = rank_value[change_pos];
				rank_value[change_pos] = temp;
				Data_Sample* p_temp = data_sample[i];
				data_sample[i] = data_sample[change_pos];
				data_sample[change_pos] = p_temp;
			}
			int subset_num = static_cast<int>(mod_len/static_cast<double>(subset_size)+0.5);
			double average_size = mod_len/static_cast<double>(subset_num);
			int count = b;
			for(int i = 0;i<subset_num-1;i++)
			{
				d_info->start_offset = count;
				d_info->end_offset =(int)(average_size*(i+1)-1) + b;
				d_info->length = d_info->end_offset-d_info->start_offset+1;
				count = d_info->end_offset+1;
				divide_info.push_back(*d_info);
			}
			d_info->start_offset = count;
			d_info->end_offset = e;
			d_info->length = d_info->end_offset - d_info->start_offset +1;
			divide_info.push_back(*d_info);
		}
	}
	else
	{
		for(;it != end;++it)
		{
			divide_info.push_back(*it);
		}
	}
#ifdef DEBUG
	for(int i=0;i<sample_length;++i)
	{
		cout<<data_sample[i]->index<<" ";
		cout<<static_cast<int>(rank_value[i])/MOD<<" ";
		cout<<static_cast<int>(rank_value[i])%MOD<<endl;
	}
	cout<<endl;
	cout<<"Print divide info: "<<endl;	
	vector<Divide_Info>::iterator i = divide_info.begin();
	vector<Divide_Info>::iterator ei = divide_info.end();
	while(i != ei)
	{
		cout<<i->start_offset<<" ";
		cout<<i->end_offset<<" ";
		cout<<i->length<<endl;
		++i;
	}
#endif

/*
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


*/


/*
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
*/
	delete d_info;
	delete[] rank_value;
	all_info.push_back(divide_info);
	Data_Sample** new_sample = new Data_Sample*[sample_length];
	for(int i =0;i<sample_length;i++)
		new_sample[i] = data_sample[i];
	ranked_sample.push_back(new_sample);
}
