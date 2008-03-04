#include "centroid.h"
#include <fstream>
#include <string>
#include <vector>
using namespace std;
extern vector<string> split(const string& src, string delimit, 
		string null_subst="");
double inner_product(Data_Sample *a, Data_Sample *b) 
	/* compute the inner product of two sparse vectors */
{
	register float sum=0;
	register int len1 = a->data_vector_length;
	register int len2 = b->data_vector_length;
	register int i=0;
	register int j=0;
	register Data_Node *ai = a->data_vector;
	register Data_Node *bj = b->data_vector;
	while (i<len1 && j<len2) {
		if(ai[i].index > bj[j].index) {
			j++;
		}
		else if (ai[i].index < bj[j].index) {
			i++;
		}
		else {
			sum+=ai[i].value * bj[j].value;
			i++;
			j++;
		}
	}
	return((double)sum);
}
Data_Sample* subtract(Data_Sample* a,Data_Sample* b)
	//compute the subtract of two vectors
{
	register int len1 = a->data_vector_length;
	register int len2 = b->data_vector_length;
	register int i=0;
	register int j=0;
	register Data_Node * temp_nodes = new Data_Node[len1+len2];   
	register int length=0;
	register Data_Node *ai = a->data_vector;
	register Data_Node *bj = b->data_vector;
	while (i<len1 && j<len2) {
		if(ai[i].index > bj[j].index) {
			j++;
			temp_nodes[length].index = bj[j].index;
			temp_nodes[length].value =- bj[j].value;
			length ++;
		}
		else if (ai[i].index < bj[j].index) {
			i++;
			temp_nodes[length].index = ai[i].index;
			temp_nodes[length].value = ai[i].value;
			length++;
		}
		else {
			i++;
			j++;
			temp_nodes[length].index = ai[i].index;
			temp_nodes[length].value = ai[i].value - bj[j].value;
			length++;
		}
	}
	Data_Sample * sample = new Data_Sample();
	sample->data_vector_length = length;
	sample->data_vector = temp_nodes;
	return sample;
}
void Centroid::divide(Data_Sample** data_sample,int sample_length,int subset_size,float label, 
		vector<Data_Sample**> & ranked_sample,vector<vector<Divide_Info> > & all_info,char* file)

{
	int my_label_rank = 0;
	int nr_class =0;
	//read centroids
	int line_buf_size = 1024*1024;
	char* buf = new char[line_buf_size];
	ifstream f_in;
	f_in.open(file);
	string line;
	vector<string> splitted_line;
	vector<Data_Sample*> centroids;
	while(f_in.getline(buf,line_buf_size))
	{
		line.assign(buf);
		splitted_line = split(buf," ");
		Data_Sample* sample = new Data_Sample();
		sample->data_vector_length = splitted_line.size()-1;
		sample->data_vector = new Data_Node[sample->data_vector_length];
		sample->label = atof(splitted_line[0].c_str());
		if(sample->label == label)
			my_label_rank = nr_class;
		for(int i =1;i<splitted_line.size();i++)
		{
			string one_node = splitted_line[i];
			sample->data_vector[i-1].index = atoi(one_node.c_str());
			sample->data_vector[i-1].value = atof(one_node.substr(one_node.find(":")+1).c_str());	
		}
		centroids.push_back(sample);
		splitted_line.clear();
		nr_class++;
	}
	delete[] buf;
	f_in.close();
	//read done

	Data_Sample* my_centroid = centroids[my_label_rank];
	Data_Sample** w = new Data_Sample*[nr_class]; //line between two centroids
	for(int i =0;i<nr_class;i++)
	{
		w[i] =subtract(centroids[i],my_centroid);
	}
	//centroids are no longer useful
	for(int i =0;i<nr_class;i++)
	{
		Data_Sample* sample = centroids[i];
		delete[] sample->data_vector;
		delete sample;
	}
	centroids.clear();
	//compute two norm of all samples and w
	double* x_norm = new double[sample_length];
	for(int i =0;i<sample_length;i++)
		x_norm[i] = inner_product(data_sample[i],data_sample[i]);
	double* w_norm = new double[nr_class];
	for(int i =0;i<nr_class;i++)
		w_norm[i] = inner_product(w[i],w[i]);

	double* rank_value = new double[sample_length];
	Divide_Info* d_info = new Divide_Info();
	vector<Divide_Info> divide_info;
	for(int class_num=0;class_num<nr_class;class_num++)
	{
		for(int i = 0; i<sample_length;i++)
		{
			rank_value[i] =0;
		}
		for(int i = 0;i<sample_length;i++)
		{
			Data_Sample * sample = data_sample[i];
			double inner_p = inner_product(sample,w[class_num]);
			rank_value[i] = x_norm[i] - inner_p*inner_p/w_norm[class_num];
		}	
		Data_Sample** new_sample = new Data_Sample*[sample_length];
		for(int i =0;i<sample_length;i++)
			new_sample[i] = data_sample[i];

		for(int i = 0;i<sample_length;i++)
		{
			for(int j =i+1;j<sample_length;j++)
			{
				if(rank_value[i]>rank_value[j])
				{
					double temp = rank_value[i];
					rank_value[i] = rank_value[j];
					rank_value[j] = temp;
					Data_Sample* p_temp = new_sample[i];
					new_sample[i] = new_sample[j];
					new_sample[j] = p_temp;
				}
			}
		}
		int subset_num =(int)(sample_length/(double)subset_size+0.5);
		double average_size = sample_length/(double)subset_num;
		int count = 0;
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
		all_info.push_back(divide_info);
		divide_info.clear();
		ranked_sample.push_back(new_sample);
	}
	delete[] x_norm;
	delete[] w_norm;
	for(int i =0;i<nr_class;i++)
	{
		delete[] w[i]->data_vector;
		delete w[i];
	}
	delete[] w;
	delete d_info;
	delete[] rank_value;
}

