#include "libsvm.h"
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#define min(a,b) a>b?b:a

using namespace std;
class Kernel
{
public :
	static double k_function(const svm_node* x,const svm_node* y,const svm_parameter& param);
};
libsvm::libsvm(libsvm_parameter param)
{
	this->param = param;
}

libsvm::~libsvm(void)
{
}

int libsvm::train(Data_Sample** sample1,Data_Sample** sample2,
		int subset_num1, int subset_num2,
		int* subset_length1,int* subset_length2,const char* model_file )
{
	decision_function * d_functions = new decision_function[subset_num1*subset_num2];
	int p = 0;
	int total_sample_num = 0;
	int* start1 = new int[subset_num1];
	int* start2 = new int[subset_num2];
	for(int i =0;i<subset_num1;i++)
	{
		start1[i] = total_sample_num;
		total_sample_num+=subset_length1[i];
	}
	for(int i = 0;i<subset_num2;i++)
	{
		start2[i] = total_sample_num;
		total_sample_num += subset_length2[i];
	}
	bool* nonezero = new bool[total_sample_num];
	for(int i =0;i<total_sample_num;i++)
	{
		nonezero[i] = false;
	}
	for(int i = 0;i<subset_num1;i++)
	{
		for(int j =0;j<subset_num2;j++)
		{
			svm_problem sub_prob;
			sub_prob.l = subset_length1[i] + subset_length2[j];
			sub_prob.y = (double*) malloc(sizeof(double)*sub_prob.l);
			sub_prob.x = (svm_node**) malloc(sizeof(svm_node*)*sub_prob.l);
			for(int ii =0;ii<subset_length1[i];ii++)
			{
				Data_Sample sample = sample1[i][ii];
				sub_prob.y[ii] = 1;
				svm_node* nodes =(svm_node*) malloc(sizeof(svm_node)*(sample.data_vector_length +1));
				memcpy(nodes,sample.data_vector,sizeof(svm_node)*sample.data_vector_length);
				nodes[sample.data_vector_length].index = -1;
				nodes[sample.data_vector_length].value = 0;	
				sub_prob.x[ii] = nodes;
			}
			for(int jj =0;jj<subset_length2[j];jj++)
			{
				Data_Sample sample = sample2[j][jj];
				sub_prob.y[jj+subset_length1[i]] = -1;
				svm_node* nodes =(svm_node*) malloc(sizeof(svm_node)*(sample.data_vector_length +1));
				memcpy(nodes,sample.data_vector,sizeof(svm_node)*sample.data_vector_length);
				nodes[sample.data_vector_length].index = -1;
				nodes[sample.data_vector_length].value = 0;	
				sub_prob.x[jj+subset_length1[i]] = nodes;
			}
			/*debug
			cout<<"subpoblem info: l = " <<sub_prob.l<<endl;
			for(int dd =0;dd <sub_prob.l;dd++)
			{
				cout<<sub_prob.y[dd]<<" ";
			}
			cout<<endl;
			for(int dd =0;dd< sub_prob.l;dd++)
			{
				svm_node* node = sub_prob.x[dd];
				int pp =0;
				while(node[pp].index != -1)
				{
					cout<<node[pp].index<<":"<<node[pp].value<<" ";
					pp++;
				}
				cout<<endl;
				cin.get();
			}
			debug*/
			
			decision_function f = svm_train_one(&sub_prob,&param.param,param.param.C,param.param.C);
			d_functions[p++] = f;
			for(int ii =0;ii<subset_length1[i];ii++)
			{
				if(f.alpha[ii] >0||f.alpha[ii]<0)
					nonezero[ii+start1[i]] =true;
			}
			for(int jj =0;jj<subset_length2[j];jj++)
			{
				if(f.alpha[jj+subset_length1[i]]>0 || f.alpha[jj+subset_length1[i]]<0)
					nonezero[jj+start2[j]] = true;
			}
			free(sub_prob.y);
			for(int ii =0;ii<sub_prob.l;ii++)
			{
				free(sub_prob.x[ii]);
			}
			free(sub_prob.x);
		} 
	}
	
	//build a model
	//initialize
	model.memory_size = sizeof(m3_libsvm_submodel);
	model.param = param.param;
	model.subset_num1 =subset_num1;
	model.subset_num2 = subset_num2;
	model.n_sv = new int[subset_num1+subset_num2];
	model.sv_start = new int[subset_num1+subset_num2];
	model.memory_size += 2*sizeof(int)*(subset_num1+subset_num2);
	model.svm_coef = new double*[subset_num1*subset_num2];
	model.rho = new double[subset_num1*subset_num2];
	model.memory_size += (sizeof(double)+sizeof(double*))*subset_num1*subset_num2;
	
	//count sv of each subset
	int total_sv =0;
	for(int i =0;i<subset_num1;i++)
	{
		int sv_count =0;
		for(int j =0;j<subset_length1[i];j++)
		{
			if(nonezero[j+start1[i]])
			{
				sv_count ++;
			}
		}
		model.sv_start[i] = total_sv;
		total_sv +=sv_count;
		model.n_sv[i] = sv_count;
	}
	for(int i =0;i<subset_num2;i++)
	{
		int sv_count =0;
		for(int j =0;j<subset_length2[i];j++)
		{
			if(nonezero[j+start2[i]])
			{
				sv_count ++;
			}
		}
		model.sv_start[i+subset_num1] = total_sv;
		total_sv +=sv_count;
		model.n_sv[i+subset_num1] = sv_count;
	}
	//copy svs

	model.l = total_sv;
	model.SVs = new svm_node*[model.l];
	model.memory_size +=sizeof(svm_node*)*model.l;
	p =0;
	for(int i =0;i<subset_num1;i++)
	{
		for(int j =0;j<subset_length1[i];j++)
		{
			if(nonezero[j+start1[i]])
			{
				Data_Sample sample = sample1[i][j];
				svm_node* sv = new svm_node[sample.data_vector_length+1];
				model.memory_size += sizeof(svm_node)*(sample.data_vector_length+1);
				model.SVs[p++] = sv;
				memcpy(sv,sample.data_vector,sizeof(svm_node)*sample.data_vector_length);
				sv[sample.data_vector_length].index = -1;
				sv[sample.data_vector_length].value = 0;
			}
		}
	}
	for(int i =0;i<subset_num2;i++)
	{
		for(int j =0;j<subset_length2[i];j++)
		{
			if(nonezero[j+start2[i]])
			{
				Data_Sample sample = sample2[i][j];
				svm_node* sv = new svm_node[sample.data_vector_length+1];
				model.memory_size += sizeof(svm_node)*(sample.data_vector_length+1);
				model.SVs[p++] = sv;
				memcpy(sv,sample.data_vector,sizeof(svm_node)*sample.data_vector_length);
				sv[sample.data_vector_length].index = -1;
				sv[sample.data_vector_length].value = 0;
			}
		}
	}
	//store sv_coef and rho
	for(int i =0;i<subset_num1;i++)
	{
		for(int j =0;j<subset_num2;j++)
		{
			int index = subset_num2*i +j;
			model.rho[index] = d_functions[index].rho;
			model.svm_coef[index] = new double[model.n_sv[i] + model.n_sv[j+subset_num1]];
			model.memory_size += sizeof(double)*(model.n_sv[i] + model.n_sv[j+subset_num1]);
			int temp_index =0;
			for(int ii = 0; ii<subset_length1[i];ii++)
			{
				if(nonezero[ii+start1[i]])
				{
					model.svm_coef[index][temp_index++] = d_functions[index].alpha[ii];
				}
			}
			for(int ii = 0; ii<subset_length2[j];ii++)
			{
				if(nonezero[ii+start2[j]])
				{
					model.svm_coef[index][temp_index++] = d_functions[index].alpha[ii+subset_length1[i]];
				}
			}

		}
	}
	save_model(model_file);
	int memory_size = model.memory_size;
	delete [] start1;
	delete [] start2;
	delete [] d_functions;
	delete [] nonezero;
	free_model();
	return memory_size;
}
extern const char* kernel_type_table[];
int libsvm::save_model(const char* fileURL)
{
	ofstream model_out;
	model_out.open(fileURL);
	cout<<"saving model to file: "<<fileURL<<endl;
	model_out<<"Memory_Size "<<model.memory_size<<endl;
	model_out<<"Kernel_Type "<<kernel_type_table[param.param.kernel_type]<<endl;
	if(param.param.kernel_type == POLY)
			model_out<<"degree "<<param.param.degree<<endl;
	
	if(param.param.kernel_type == POLY || param.param.kernel_type == RBF || param.param.kernel_type == SIGMOID)
			model_out<<"gamma "<<param.param.gamma<<endl;
	
	if(param.param.kernel_type == POLY || param.param.kernel_type == SIGMOID)
			model_out<<"coef0 "<<param.param.coef0<<endl;
	model_out<<"Total_SV "<<model.l<<endl;
	model_out<<"Possitive_Subset_Number "<<model.subset_num1<<endl;
	model_out<<"Negative_Subset_Number "<<model.subset_num2<<endl;
	model_out<<"Number of SV for Each Set:"<<endl;

	for(int i =0;i<model.subset_num1+model.subset_num2-1;i++)
	{
		model_out<<model.n_sv[i]<<" ";
	}
	model_out<<model.n_sv[model.subset_num1+model.subset_num2-1]<<endl;
	
	model_out<<"Start index of SV for Each Set:"<<endl;

	for(int i =0;i<model.subset_num1+model.subset_num2-1;i++)
	{
		model_out<<model.sv_start[i]<<" ";
	}
	model_out<<model.sv_start[model.subset_num1+model.subset_num2-1]<<endl;
	
	model_out<<"SVs: "<<endl;
	for(int i =0;i<model.l;i++)
	{
		svm_node * sv = model.SVs[i];
		int p =0;
		while(sv[p+1].index != -1)
		{
			model_out<<sv[p].index<<":"<<sv[p].value<<" ";
			p++;
		}
		model_out<<sv[p].index<<":"<<sv[p].value<<endl;
	}
	model_out<<"Alphas:"<<endl;
	for(int i =0;i<model.subset_num1;i++)
	{
		for(int j =0;j<model.subset_num2;j++)
		{
			double * alphas = model.svm_coef[i*model.subset_num2 +j];
			int nsv = model.n_sv[i] + model.n_sv[j+model.subset_num1];
			for(int ii = 0;ii<nsv-1;ii++)
			{
				model_out<<alphas[ii]<<" ";
			}
			model_out<<alphas[nsv-1]<<endl;
		}
	}
	model_out<<"rho:"<<endl;
	for(int i =0;i<model.subset_num1*model.subset_num2-1;i++)
	{
		model_out<<model.rho[i]<<" ";
	}
	model_out<<model.rho[model.subset_num1*model.subset_num2-1]<<endl;
	model_out.close();
	return 0;
}
void libsvm::free_model()
{
	for(int i =0;i<model.subset_num1*model.subset_num2;i++)
	{
		delete[] model.svm_coef[i];
	}
	delete[] model.svm_coef;
	delete[] model.rho;
	for(int i = 0;i<model.l;i++)
	{
		delete[] model.SVs[i];
	}
	delete[] model.SVs;
	delete[] model.n_sv;
	delete[] model.sv_start;
}
int seek(char* buf,int bufsize,char objective)
{
	int i =0;
	while(buf[i] != objective && i<bufsize)
	{
		i++;
	}
	if(i == bufsize-1 && buf[i] != objective)
	{
		return -1;
	}
	else
	{
		return i;
	}
}
typedef basic_string<char>::size_type S_T;
static const S_T npos = -1;
vector<string> split(const string& src, string delimit, 

		string null_subst="")
{
	if( src.empty() || delimit.empty() ) throw "split:empty string\0";

	vector<string> v;
	S_T deli_len = delimit.size();
	long index = npos, last_search_position = 0;
	while( (index=src.find(delimit,last_search_position))!=npos )
	{
		 if(index==last_search_position)
			   v.push_back(null_subst);
		  else
			    v.push_back( src.substr(last_search_position, 
							index-last_search_position) );
		   last_search_position = index + deli_len;
	}
	string last_one = src.substr(last_search_position);
	v.push_back( last_one.empty()? null_subst:last_one );
	return v;
}

int libsvm::load_model(const char* fileURL)
{
	int line_buf_size = 1024*1024*10;
	char* buf = new char[line_buf_size];
	ifstream model_in;
	model_in.open(fileURL);
	//memory size
	model_in.getline(buf,line_buf_size);
	int offset = seek(buf,line_buf_size,' ');
	model.memory_size = atoi(&buf[offset+1]);
	//kernel type
	model_in.getline(buf,line_buf_size);
	offset = seek(buf,line_buf_size,' ');
	int kernel_type =0;
	while(kernel_type_table[kernel_type] != NULL)
	{
		if(strcmp(kernel_type_table[kernel_type],&buf[offset+1])==0)
		{
			model.param.kernel_type = kernel_type;
			break;
		}
		kernel_type ++;
	}
	switch(model.param.kernel_type)
	{
		case POLY:
			model_in.getline(buf,line_buf_size);
			offset = seek(buf,line_buf_size,' ');
			model.param.degree = atoi(&buf[offset+1]);
			
			model_in.getline(buf,line_buf_size);
			offset = seek(buf,line_buf_size,' ');
			model.param.gamma = atof(&buf[offset+1]);
			
			model_in.getline(buf,line_buf_size);
			offset = seek(buf,line_buf_size,' ');
			model.param.coef0 = atof(&buf[offset+1]);
			break;
		case RBF:
			model_in.getline(buf,line_buf_size);
			offset = seek(buf,line_buf_size,' ');
			model.param.gamma = atof(&buf[offset+1]);
			break;
		case SIGMOID:
			model_in.getline(buf,line_buf_size);
			offset = seek(buf,line_buf_size,' ');
			model.param.gamma = atof(&buf[offset+1]);
			
			model_in.getline(buf,line_buf_size);
			offset = seek(buf,line_buf_size,' ');
			model.param.coef0 = atof(&buf[offset+1]);
			break;
		default:
			break;
	}
	model_in.getline(buf,line_buf_size);
	offset = seek(buf,line_buf_size,' ');
	model.l = atoi(&buf[offset+1]);
	model_in.getline(buf,line_buf_size);
	offset = seek(buf,line_buf_size,' ');
	model.subset_num1= atoi(&buf[offset+1]);
	model_in.getline(buf,line_buf_size);
	offset = seek(buf,line_buf_size,' ');
	model.subset_num2 = atoi(&buf[offset+1]);
	model.n_sv = new int[model.subset_num1+model.subset_num2];
	model.sv_start = new int[model.subset_num1+model.subset_num2];
	model_in.getline(buf,line_buf_size);
	model_in.getline(buf,line_buf_size);
	
	string line(buf);
	vector<string> sub_line = split(line," ");
	for(int i =0;i<sub_line.size();i++)
	{
		model.n_sv[i] = atoi(sub_line[i].c_str());
	}
	sub_line.clear();

		
	model_in.getline(buf,line_buf_size);
	model_in.getline(buf,line_buf_size);
	line.assign(buf);
	sub_line = split(line," ");
	for(int i =0;i<sub_line.size();i++)
	{
		model.sv_start[i] = atoi(sub_line[i].c_str());
	}
	sub_line.clear();

	//load sv
	model_in.getline(buf,line_buf_size);
	model.SVs = new svm_node*[model.l];
	for(int i =0;i<model.l;i++)
	{
		model_in.getline(buf,line_buf_size);
		string line(buf);
		vector<string> splitted_line = split(line," ");
		svm_node* nodes = new svm_node[splitted_line.size()+1];
		for(int j =0;j<splitted_line.size();j++)
		{
			string one_node = splitted_line[j];
			nodes[j].index = atoi(one_node.c_str());
			nodes[j].value = atof(one_node.substr(one_node.find(":")+1).c_str());
		}
		//corrected my hoss : swap splitted_line.clear() and nodes...,nodes...///
		nodes[splitted_line.size()].index = -1;
		nodes[splitted_line.size()].value = 0;
		splitted_line.clear();
		/////////////////////////////////////////////////////////////////////////
		model.SVs[i] = nodes;
	}
	//load sv_coef and rho
	model.svm_coef = new double*[model.subset_num1*model.subset_num2];
	model.rho = new double[model.subset_num1*model.subset_num2];
	model_in.getline(buf,line_buf_size);
	for(int i =0;i<model.subset_num1;i++)
	{
		for(int j =0;j<model.subset_num2;j++)
		{
			double* alphas = new double[model.n_sv[i]+model.n_sv[j+model.subset_num1]];
			model_in.getline(buf,line_buf_size);
			string line(buf);
			vector<string> splitted_line = split(line," ");
			for(int ii =0;ii<splitted_line.size();ii++)
			{
				alphas[ii] = atof(splitted_line[ii].c_str());
			}
			splitted_line.clear();
			model.svm_coef[i*model.subset_num2+j] = alphas;
		}
	}
	model_in.getline(buf,line_buf_size);
	model_in.getline(buf,line_buf_size);
	line.assign(buf);
	vector<string> splitted_line = split(line," ");
	for(int i =0;i<splitted_line.size();i++)
	{
		model.rho[i] = atof(splitted_line[i].c_str());
	}
	splitted_line.clear();	
	model_in.close();
	delete[] buf;
	return 1;
}

double* libsvm::predict(Data_Sample* sample)
{
	svm_node* x = new svm_node[sample->data_vector_length +1];
	memcpy(x,sample->data_vector,sizeof(svm_node)*sample->data_vector_length);
	x[sample->data_vector_length].index = -1;
	double * k_values = new double[model.l];
	double* dec_values = new double[model.subset_num1*model.subset_num2];
	for(int i =0;i<model.l;i++)
	{
		k_values[i] = Kernel::k_function(x,model.SVs[i],model.param);
	}
	for(int i =0;i<model.subset_num1;i++)
	{
		for(int j =0;j<model.subset_num2;j++)
		{
			double sum =0;
			double* alphas = model.svm_coef[i*model.subset_num2+j];
			int s1 = model.sv_start[i];
			int s2 = model.sv_start[j+model.subset_num1];
			int c1 = model.n_sv[i];
			int c2 = model.n_sv[j+model.subset_num1];
			for(int ii = 0;ii<c1;ii++)
			{
				sum += alphas[ii]*k_values[s1+ii];
			}
			for(int ii =0;ii<c2;ii++)
			{
				sum+= alphas[ii+c1] *k_values[s2+ii];
			}
			sum -= model.rho[i*model.subset_num2+j];
			dec_values[i*model.subset_num2 +j] = sum;
		}
	}
	delete[] k_values;
	delete[] x;
	return dec_values;
}
void libsvm::predict(Data_Sample* sample,int& x,int& y,float& direction)
{
	svm_node* x_node = new svm_node[sample->data_vector_length +1];
	memcpy(x_node,sample->data_vector,sizeof(svm_node)*sample->data_vector_length);
	x_node[sample->data_vector_length].index = -1;
	double* k_values= new double[model.l] ;
	for(int i =0;i<model.l;i++)
		k_values[i] =-1;
	int i =x,j = y;
	while(i<model.subset_num1 && j<model.subset_num2)
	{
		double sum =0;
		int index = i*model.subset_num2+j;
		double* alphas = model.svm_coef[index];
		int s1 = model.sv_start[i];
		int s2 = model.sv_start[j+model.subset_num1];
		int c1 = model.n_sv[i];
		int c2 = model.n_sv[j+model.subset_num1];
		for(int ii = 0;ii<c1;ii++)
		{
			if(k_values[s1+ii] < 0)
			{
				k_values[s1+ii] = Kernel::k_function(x_node,model.SVs[s1+ii],model.param);
			}
			sum += alphas[ii]*k_values[s1+ii];
		}
		for(int ii =0;ii<c2;ii++)
		{
			if(k_values[s2+ii] < 0)
			{
				k_values[s2+ii] = Kernel::k_function(x_node,model.SVs[s2+ii],model.param);
			}
			sum+= alphas[ii+c1] *k_values[s2+ii];
		}
		sum -= model.rho[index];
		direction = sum > 0?1:-1;
		if(direction >0)
		{
			j++;
		}else
		{
			i++;
		}
	}
	delete[] k_values;
	delete[] x_node;
    x = min(i,model.subset_num1-1);
	y = min(j,model.subset_num2-1);
}
void libsvm::predict(Data_Sample* sample,float* min_value)
{
	svm_node* x = new svm_node[sample->data_vector_length +1];
	memcpy(x,sample->data_vector,sizeof(svm_node)*sample->data_vector_length);
	x[sample->data_vector_length].index = -1;
	double * k_values = new double[model.l];
	for(int i =0;i<model.l;i++)
	{
		k_values[i] = -1; 
	}
	for(int i =0;i<model.subset_num1;i++)
	{
		if(min_value[i]<0)
			continue;
		for(int j =0;j<model.subset_num2;j++)
		{
			double sum =0;
			double* alphas = model.svm_coef[i*model.subset_num2+j];
			int s1 = model.sv_start[i];
			int s2 = model.sv_start[j+model.subset_num1];
			int c1 = model.n_sv[i];
			int c2 = model.n_sv[j+model.subset_num1];
			int sv_id;
			for(int ii = 0;ii<c1;ii++)
			{
				sv_id = s1+ii;
				if(k_values[sv_id]<0)
					k_values[sv_id] = Kernel::k_function(x,model.SVs[sv_id],model.param);
				sum += alphas[ii]*k_values[sv_id];
			}
			for(int ii =0;ii<c2;ii++)
			{
				sv_id = s2+ii;
				if(k_values[sv_id]<0)
					k_values[sv_id] = Kernel::k_function(x,model.SVs[sv_id],model.param);
				sum+= alphas[ii+c1] *k_values[sv_id];
			}
			sum -= model.rho[i*model.subset_num2+j];
			min_value[i] = min(sum,min_value[i]);
			if(sum < 0)
				break;
		}
	}
	delete[] k_values;
	delete[] x;
}
