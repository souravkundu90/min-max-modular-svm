#include "hyper_plane.h"
#include "iostream"
using namespace std;


void Hyper_Plane::divide(Data_Sample** data_sample,int sample_length,int subset_size,float label, 
		  vector<Data_Sample**> & ranked_sample,vector<vector<Divide_Info> > & all_info,char* file)

{
	cout<<"starting single_label_divide"<<endl;
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
	Data_Sample** new_sample = new Data_Sample*[sample_length];
	for(int i =0;i<sample_length;i++)
		new_sample[i] = data_sample[i];
	divide_one(new_sample,rank_value,sample_length,subset_size,divide_info);
	//debug
	for(int i =0;i<divide_info.size();i++)
	{
		Divide_Info d_info = divide_info[i];
		cout<<d_info.length<<":"<<d_info.start_offset<<":"<<d_info.end_offset<<"\t";
	}
	//debug
	delete[] rank_value;
	all_info.push_back(divide_info);
	ranked_sample.push_back(new_sample);
}

void filter_sample(Data_Sample** data_sample,int sample_length, float filter_label,vector<Data_Sample*> filted_sample)
{
	for(int i =0;i<sample_length;i++)
	{
		Data_Sample* sample = data_sample[i];
		bool keep = true;
		for(int j = 0;j < sample->mlabel_len;j++)
		{
			if(sample->mlabel[j] == filter_label)
				keep = false;
		}
		if(keep)
		{
			filted_sample.push_back(sample);
		}
	}
}


void Hyper_Plane::multi_label_divide(Data_Sample** data_sample,int sample_length,int subset_size,float label, 
						 vector<Data_Sample**> & ranked_sample,vector<vector<Divide_Info> >&all_info,char* file,vector<float> all_label)
{
	//debug
	cout<<"starting multi_label_divide"<<endl;
	//debug
	vector<Divide_Info> divide_info;
	vector<Data_Sample*> filted_sample;
	for(int label_index = 0; label_index < all_label.size();label_index++)
	{
		float current_label = all_label[label_index];
		if(current_label == label)
		{
			vector<Data_Sample**> temp_sample;
			vector<vector<Divide_Info> > temp_info;
			divide(data_sample,sample_length,subset_size,label,temp_sample,temp_info,file);
			ranked_sample.push_back(temp_sample[0]);
			all_info.push_back(temp_info[0]);
			temp_sample.clear();
			temp_info.clear();
			continue;
		}
		divide_info.clear();
		filted_sample.clear();
		filter_sample(data_sample,sample_length,current_label,filted_sample);
		int filted_sample_length = filted_sample.size();
		Data_Sample** filted_sample_array = new Data_Sample*[filted_sample_length];
		double * rank_value = new double[filted_sample_length];
		for(int i = 0;i<filted_sample_length;i++)
		{
			filted_sample_array[i] = filted_sample[i];
			Data_Node* vector = filted_sample_array[i]->data_vector;
			int length = filted_sample_array[i]->data_vector_length;
			rank_value[i] = 0;
			for(int j = 0; j< length;j++)
			{
				rank_value[i] += vector[j].value;
			}
		}
		divide_one(filted_sample_array,rank_value,filted_sample_length,subset_size,divide_info);
		delete[] rank_value;
		all_info.push_back(divide_info);
		ranked_sample.push_back(filted_sample_array);
	}
}