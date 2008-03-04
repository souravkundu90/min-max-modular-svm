#include "svm_light.h"
#include "iostream"
#include "fstream"
#include "vector"
#include "string.h"
#include "time.h"
#define min(a,b) a>b?b:a
//#define debug
extern "C"
{
	void  svm_learn_classification(DOC *, double *, long, long, LEARN_PARM *, 
				KERNEL_PARM *, KERNEL_CACHE *, MODEL *);
	void   kernel_cache_init(KERNEL_CACHE *,long, long);
	void   kernel_cache_cleanup(KERNEL_CACHE *);
	void clear_vector_n(double *vec, long int n);
	void add_vector_ns(double *vec_n, WORD *vec_s, double faktor);
	double sprod_ns(double *vec_n, WORD *vec_s);
	CFLOAT kernel(KERNEL_PARM *kernel_parm, DOC *a, DOC *b);
}
using namespace std;
svm_light::svm_light(svm_light_parameter param)
{
	this->param = param;
}

svm_light::~svm_light(void)
{
}

int svm_light::train(Data_Sample** sample1,Data_Sample** sample2,
		int subset_num1, int subset_num2,
		int* subset_length1,int* subset_length2, const char* model_file )
{
#ifdef debug	
	ofstream debug_stream;
	if(debug)
	{
		string name = "lignt/";
		name = name + model_file;
		debug_stream.open(name.c_str());
		debug_stream << model_file << " svm light walk in"<<endl;
	}
#endif
	
	int totwords = 0;
	int longest_sample =0;
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
		start2[i]= total_sample_num;
		total_sample_num += subset_length2[i];
	}

	DOC** docs1 = new DOC*[subset_num1];
    DOC** docs2 = new DOC*[subset_num2];
	DOC** all_docs = new DOC*[total_sample_num];
	int index_counter = 0;
	//scan for statistics and build DOCs
	for(int i =0;i<subset_num1;i++)
	{
		DOC* docs  = new DOC[subset_length1[i]];
		docs1[i] = docs;
		Data_Sample* subset = sample1[i];
		for(int j = 0;j<subset_length1[i];j++)
		{
			Data_Sample data_sample = subset[j];
			int max_word_id = data_sample.data_vector[data_sample.data_vector_length-1].index;
			if(max_word_id > totwords)
				totwords = max_word_id;
			if(longest_sample < data_sample.data_vector_length)
				longest_sample = data_sample.data_vector_length;
			docs[j].docnum = data_sample.index;
			docs[j].words = new WORD[data_sample.data_vector_length + 1];
			memcpy(docs[j].words,data_sample.data_vector,sizeof(WORD)*data_sample.data_vector_length);
			docs[j].words[data_sample.data_vector_length].wnum = 0;
			docs[j].twonorm_sq = sprod_ss(docs[j].words,docs[j].words);
			all_docs[index_counter++] = &docs[j];
		}
	}
	for(int i =0;i<subset_num2;i++)
	{
		DOC* docs  = new DOC[subset_length2[i]];
		docs2[i] = docs;
		Data_Sample* subset = sample2[i];
		for(int j = 0;j<subset_length2[i];j++)
		{
			Data_Sample data_sample = subset[j];
			int max_word_id = data_sample.data_vector[data_sample.data_vector_length-1].index;
			if(max_word_id > totwords)
				totwords = max_word_id;
			if(longest_sample < data_sample.data_vector_length)
				longest_sample = data_sample.data_vector_length;
			docs[j].docnum = data_sample.index;
			docs[j].words = new WORD[data_sample.data_vector_length + 1];
			memcpy(docs[j].words,data_sample.data_vector,sizeof(WORD)*data_sample.data_vector_length);
			docs[j].words[data_sample.data_vector_length].wnum = 0;
			docs[j].twonorm_sq = sprod_ss(docs[j].words,docs[j].words);
			all_docs[index_counter++] = &docs[j];
		}
	}

	KERNEL_CACHE kernel_cache;
	bool * nonezero = new bool[total_sample_num];
	for(int i = 0; i<total_sample_num;i++)
	{
		nonezero[i] = false;
	}
	int** SV_ids = new int*[subset_num1 * subset_num2];

	model.kernel_param = param.kernel_parm;
	model.subset_num1 = subset_num1;
	model.subset_num2 = subset_num2;
	model.totwords = totwords;
	model.sv_index = new int*[subset_num1*subset_num2];
	model.alphas = new double*[subset_num1*subset_num2];
	model.sv_count = new int[subset_num1 * subset_num2];
	model.thresholds = new double[subset_num1*subset_num2];
#ifdef debug	
	debug_stream << "start training"<<endl;
#endif
	for(int i = 0;i<subset_num1;i++)
	{
		for(int j =0;j<subset_num2;j++)
		{
			MODEL light_model;
			int index = i*subset_num2+j;
			DOC * docs=new DOC[subset_length1[i]+subset_length2[j]];
			double * target=new double[subset_length1[i]+subset_length2[j]];
			int ii = 0;
			for(;ii<subset_length1[i];ii++)
			{
				docs[ii] = docs1[i][ii];
				target[ii] = 1;
			}
			for(;ii<subset_length1[i]+ subset_length2[j];ii++)
			{
				docs[ii]= docs2[j][ii-subset_length1[i]];
				target[ii] = -1;
			}
			int totdoc = subset_length1[i] + subset_length2[j];
		
			if(param.kernel_parm.kernel_type == LINEAR) { /* don't need the cache */
				if(param.learn_parm.type == CLASSIFICATION) {
					svm_learn_classification(docs,target,totdoc,totwords,&(param.learn_parm),
							&(param.kernel_parm),NULL,&light_model);
				}
			}
			else {
				if(param.learn_parm.type == CLASSIFICATION) {
					/* Always get a new kernel cache. It is not possible to use the
					   same cache for two different training runs */
					kernel_cache_init(&kernel_cache,totdoc,param.kernel_cache_size);
					svm_learn_classification(docs,target,totdoc,totwords,&(param.learn_parm),
							&(param.kernel_parm),&kernel_cache,&light_model);
					/* Free the memory used for the cache. */
					kernel_cache_cleanup(&kernel_cache);
				}
			}
			//todo: get results svs before clean temp docs
			/*debug
			cout<<"nsv:"<<light_model.sv_num-1<<endl;
			cout<<light_model.b<<endl;
			for(int t =1;t < light_model.sv_num;t++)
			{
				cout<<light_model.alpha[t]<<" ";
			}
			cout<<endl;
			for(int t =1;t <light_model.sv_num;t++)
			{
				for(j =0; (light_model.supvec[t]->words[j]).wnum;j++)
					cout<<(light_model.supvec[t]->words[j]).wnum<<":"<<(light_model.supvec[t]->words[j]).weight<<" ";
				cout<<endl;
			}

			//debug*/
			model.sv_count[index] = light_model.sv_num-1;
			model.thresholds[index] = light_model.b;
			SV_ids[index] = new int[light_model.sv_num-1];
			model.alphas[index] = new double[light_model.sv_num-1];
			for(int t =1;t<light_model.sv_num;t++)
			{
				int docnum = light_model.supvec[t]->docnum;
				int real_rank =0;
				if(docnum<=subset_length1[i]-1)
				{
					real_rank = docnum + start1[i];
				}else
				{
					real_rank = docnum-subset_length1[i] + start2[j];
				}
				SV_ids[index][t-1]= real_rank;
				nonezero[real_rank] = true;
				model.alphas[index][t-1] = light_model.alpha[t];
			}
			
			free(light_model.supvec);
			free(light_model.index);
			free(light_model.alpha);
			delete[] target;
			delete[] docs;
		}
	}
#ifdef debug
	if(debug)
		debug_stream << "building model " <<endl;
#endif
	int total_sv_num =0;
	for(int i =0;i<total_sample_num;i++)
	{
		if(nonezero[i])
			total_sv_num ++;
	}
	model.total_sv_count = total_sv_num;
	model.SVs = new DOC[total_sv_num];
	int* sv_seq = new int[total_sample_num];
	int p =0;
	for(int i =0;i<total_sample_num;i++)
	{
		if(nonezero[i])
		{
			sv_seq[i]=p;
			model.SVs[p++] = *(all_docs[i]);
		}	
	}
	for(int i =0;i<subset_num1*subset_num2;i++)
	{
		model.sv_index[i] = new int[model.sv_count[i]];
		for(int j =0;j<model.sv_count[i];j++)
		{
			model.sv_index[i][j] = sv_seq[SV_ids[i][j]];
		}
	}
	/*debug
	for(int i = 0;i<model.subset_num1*model.subset_num2;i++)
	{
		cout<<model.sv_count[i]<<": ";
		for(int j =0;j<model.sv_count[i];j++)
		{
		  cout<<model.sv_index[i][j]<<" ";
		}
		cout<<endl;
	}
	//debug*/
	//calculate memory usage
	model.memory_size =sizeof(svm_light_model);
	if(model.kernel_param.kernel_type == 0)
	{
		model.memory_size += ((model.totwords+1)*sizeof(double)+sizeof(double*))*model.subset_num1* model.subset_num2;
		model.lin_weights = new double*[model.subset_num1*model.subset_num2];
		for(int i =0;i<model.subset_num1*model.subset_num2;i++)
		{
			model.lin_weights[i] = new double[model.totwords+1];
			clear_vector_n(model.lin_weights[i],model.totwords);
			for(int j =0;j<model.sv_count[i];j++)
			{
				add_vector_ns(model.lin_weights[i],model.SVs[model.sv_index[i][j]].words,model.alphas[i][j]);	
			}
		}
	}else
	{
		model.memory_size += longest_sample*sizeof(WORD)*model.total_sv_count;
		model.memory_size += (sizeof(int)+ sizeof(double)) * model.subset_num1*model.subset_num2;
		model.memory_size += model.total_sv_count * (sizeof(DOC)+sizeof(int*)+sizeof(double*));
		for(int i =0;i<model.subset_num1*model.subset_num2;i++)
			model.memory_size += model.sv_count[i] * (sizeof(double)+sizeof(int));
	}
#ifdef debug
	if (debug)
		debug_stream << "saving model"<<endl;
#endif
	save_model(model_file);
	cout<<"model successfully save to "<<model_file<<endl;
#ifdef debug	
	if(debug)
		debug_stream<<"model successfully save to "<<model_file<<endl;
#endif
	model.deap_copy_words = false;
	free_model();	
	delete[] sv_seq;
	delete[] nonezero;
	for(int i=0;i<total_sample_num;i++)
	{
		delete[] all_docs[i]->words;
	}
	for(int i =0;i<subset_num1;i++)
	{
		delete[] docs1[i];
	}
	delete[] docs1;
	for(int i =0;i<subset_num2;i++)
	{
		delete[] docs2[i];
	}
	delete[] docs2;
	for(int i =0;i< subset_num1*subset_num2;i++)
	{
		delete[] SV_ids[i];
	}
	delete[] SV_ids;
	delete[] all_docs;
	delete[] start1;
	delete[] start2;
	return 0;
#ifdef debug
	if(debug)
		debug_stream.close();
#endif
}
double* svm_light::predict(Data_Sample* sample)
{
	double* dec_values = new double[model.subset_num1*model.subset_num2];
	DOC doc;
	WORD* words = new WORD[sample->data_vector_length + 1];
	memcpy(words,sample->data_vector,sizeof(WORD)*sample->data_vector_length);
	words[sample->data_vector_length].wnum = 0;
	if(model.kernel_param.kernel_type ==2)//rbf
	{
		doc.twonorm_sq = sprod_ss(words,words);
	}
	doc.words = words;
	if(model.kernel_param.kernel_type ==0) //linear
	{
		for(int i =0;i<model.subset_num1;i++)
		{
			for(int j =0;j<model.subset_num2;j++)
			{
			  int index = i*model.subset_num2 +j;
			  register double sum=0;
			  register WORD *ai;
			  register int max_feature_id = model.totwords;
			  double* lin_weight = model.lin_weights[index];
			  ai = words;
		   	  while (ai->wnum)
			  {
				if(ai->wnum <=max_feature_id)
				{
				    sum+=(lin_weight[ai->wnum]*ai->weight);
				}
				ai++;
			  }
			  sum  -= model.thresholds[index];
			  dec_values[index] = sum;
			}
		}	
	}else
	{
		register int i;
		double *k_values = new double[model.total_sv_count];
		for(i =0;i<model.total_sv_count;i++)
		{
			k_values[i] = kernel(&model.kernel_param,&model.SVs[i],&doc);
		}
		register double dist;
		for(i =0;i<model.subset_num1;i++)
		{
			for(int t =0;t<model.subset_num2;t++)
			{
				register int index = i* model.subset_num2 + t;
				dist =0;
				int n_sv = model.sv_count[index];
				for(int j =0;j<n_sv;j++)
				{
					dist += k_values[model.sv_index[index][j]]*model.alphas[index][j];
				}
				dist -= model.thresholds[index];
				dec_values[index] = dist;
			}
		}
		delete[] k_values;
	}

	delete [] words;

	return dec_values;
}
void svm_light::predict(Data_Sample* sample,int& x,int& y,float& direction)
{
	DOC doc;
	WORD* words = new WORD[sample->data_vector_length + 1];
	memcpy(words,sample->data_vector,sizeof(WORD)*sample->data_vector_length);
	words[sample->data_vector_length].wnum = 0;
	if(model.kernel_param.kernel_type ==2)//rbf
	{
		doc.twonorm_sq = sprod_ss(words,words);
	}
	doc.words = words;
	if(model.kernel_param.kernel_type ==0) //linear
	{
		int i =x,j=y;

		while(i<model.subset_num1 && j<model.subset_num2)
		{
		  	int index = i*model.subset_num2 +j;
			register double sum=0;
			register WORD *ai;
			register int max_feature_id = model.totwords;
			double* lin_weight = model.lin_weights[index];
			ai = words;
		   	while (ai->wnum)
			{
				if(ai->wnum <=max_feature_id)
				{
				    sum+=(lin_weight[ai->wnum]*ai->weight);
				}
				ai++;
			}
			sum  -= model.thresholds[index];
			direction = sum < 0 ? -1:1;
			if(direction >0)
				j++;
			else
				i++;
		}
		x = min(i,model.subset_num1-1);
		y = min(j,model.subset_num2-1);	
	}else
	{
		int i=x,j=y;
		int total_sv_count = model.total_sv_count;
		double* k_values=new double[total_sv_count];
		for(int i =0;i<total_sv_count;i++)
			k_values[i] =-1;
		register double dist;
		while(i<model.subset_num1 && j<model.subset_num2)
		{
			register int index = i* model.subset_num2 + j;
			dist =0;
			int n_sv = model.sv_count[index];
			double* alphas = model.alphas[index];
			for(int t =0;t<n_sv;t++)
			{
				int sv_id = model.sv_index[index][t];
				if(k_values[sv_id] < 0)
				{
					k_values[sv_id] =  kernel(&model.kernel_param,&model.SVs[sv_id],&doc);
				}
				dist += k_values[sv_id]*alphas[t];
			}
			dist -= model.thresholds[index];
			direction = dist < 0 ? -1:1;
			if(direction >0)
				j++;
			else
				i++;
		}
		x = min(i,model.subset_num1-1);
		y = min(j,model.subset_num2-1);	
		delete[] k_values;
	}
	delete [] words;
 }
void svm_light::predict(Data_Sample* sample,float* min)
{
	DOC doc;
	WORD* words = new WORD[sample->data_vector_length + 1];
	memcpy(words,sample->data_vector,sizeof(WORD)*sample->data_vector_length);
	words[sample->data_vector_length].wnum = 0;
	if(model.kernel_param.kernel_type ==2)//rbf
	{
		doc.twonorm_sq = sprod_ss(words,words);
	}
	doc.words = words;
	if(model.kernel_param.kernel_type ==0) //linear
	{
		for(int i =0;i<model.subset_num1;i++)
		{
			if(min[i]<0)
				continue;
			for(int j =0;j<model.subset_num2;j++)
			{
			  int index = i*model.subset_num2 +j;
			  register double sum=0;
			  register WORD *ai;
			  register int max_feature_id = model.totwords;
			  double* lin_weight = model.lin_weights[index];
			  ai = words;
		   	  while (ai->wnum)
			  {
				if(ai->wnum <=max_feature_id)
				{
				    sum+=(lin_weight[ai->wnum]*ai->weight);
				}
				ai++;
			  }
			  sum  -= model.thresholds[index];
			  min[i] = min(min[i],sum);
			  if(sum < 0)
			  {
				  break;
			  }
			}
		}	
	}else
	{
		register int i;
		double *k_values = new double[model.total_sv_count];
		for(i =0;i<model.total_sv_count;i++)
		{
			k_values[i] = -1;
		}
		register double dist;
		for(i =0;i<model.subset_num1;i++)
		{
			if(min[i] <0)
				continue;
			for(int t =0;t<model.subset_num2;t++)
			{
				register int index = i* model.subset_num2 + t;
				dist =0;
				int n_sv = model.sv_count[index];
				register int sv_id,j;
				for(j =0;j<n_sv;j++)
				{
					sv_id = model.sv_index[index][j];
					if(k_values[sv_id]<0)
						k_values[sv_id]= kernel(&model.kernel_param,&model.SVs[sv_id],&doc);
					dist += k_values[sv_id]*model.alphas[index][j];
				}
				dist -= model.thresholds[index];
				min[i] = min(min[i],dist);
				if(dist < 0)
				{
					break;
				}			
			}
		}
		delete[] k_values;
	}
	delete [] words;
}

int svm_light::save_model(const char* model_file)
{
	  FILE *modelfl;
	  if ((modelfl = fopen (model_file, "w")) == NULL)
	  {
		  cout<<"fail to write to model file: "<<model_file<<endl;
		  return 0;
	  }
	  fprintf(modelfl,"%d # memory_size\n",model.memory_size); 
	  fprintf(modelfl,"%ld # kernel type\n",
		  model.kernel_param.kernel_type);
	  fprintf(modelfl,"%ld # kernel parameter -d \n",
	  	  model.kernel_param.poly_degree);
	  fprintf(modelfl,"%.8g # kernel parameter -g \n",
		  model.kernel_param.rbf_gamma);
	  fprintf(modelfl,"%.8g # kernel parameter -s \n",
	  	  model.kernel_param.coef_lin);
	  fprintf(modelfl,"%.8g # kernel parameter -r \n",
		  model.kernel_param.coef_const);
	  fprintf(modelfl,"%s # kernel parameter -u \n",model.kernel_param.custom);
	  fprintf(modelfl,"%d # subset num 1\n",model.subset_num1);
	  fprintf(modelfl,"%d # subset num 2\n",model.subset_num2);
	  fprintf(modelfl,"%d # hightest feature index\n", model.totwords);
	  fprintf(modelfl,"%d # total sv number\n",model.total_sv_count);
	  fprintf(modelfl,"sv number of each sub model\n");
	  int model_num = model.subset_num1*model.subset_num2;
	  for(int i =0;i<model_num;i++)
	  {
		fprintf(modelfl,"%d ",model.sv_count[i]);
	  }
	  fprintf(modelfl,"\n");
	  if(model.kernel_param.kernel_type ==0) //linear
	  {
		  fprintf(modelfl,"Linear Weights\n");
		  for(int i =0;i < model_num;i++)
		  {
			 for(int j =0;j<=model.totwords;j++)
				fprintf(modelfl,"%.8g ",model.lin_weights[i][j]);
			 fprintf(modelfl,"\n");
		  }
	  }
	  else{
		  fprintf(modelfl,"SV:\n");
		  for(int i =0;i<model.total_sv_count;i++)
		  {	
			  if(model.kernel_param.kernel_type == 2)//rbf
			  {
				  fprintf(modelfl,"%.8g ",model.SVs[i].twonorm_sq);
			  }
			  int j=0;
			  while(model.SVs[i].words[j].wnum != 0)
		 	 {
		    	fprintf(modelfl,"%d:%.8g ",
		   	    (model.SVs[i].words[j]).wnum,
		  	    (model.SVs[i].words[j]).weight);
			   	j++;
			 }
		 	 fprintf(modelfl,"\n");
		  }
		  fprintf(modelfl,"alphas:\n");
		  for(int i =0;i<model_num;i++)
	  	  {
		 	for(int j =0;j<model.sv_count[i];j++)
			 {
				fprintf(modelfl,"%d:%.8g ",
				model.sv_index[i][j],model.alphas[i][j]);
			 }
			 fprintf(modelfl,"\n");
		  }
	  }
	  fprintf(modelfl,"thresholds:\n");
 	  for(int i =0;i<model_num;i++)
	  {
		fprintf(modelfl,"%.8g ",model.thresholds[i]);
	  }	  
	  fprintf(modelfl,"\n");
	  fclose(modelfl);
	  return 1;
}
extern vector<string> split(const string& src, string delimit, 
		string null_subst="");

int svm_light::load_model(const char* model_file)
{
	int line_buf_size = 1024*1024*10;
	char* buf = new char[line_buf_size];
	ifstream model_in;
	model_in.open(model_file);
	model_in.getline(buf,line_buf_size);
	model.memory_size = atoi(buf);
	model_in.getline(buf,line_buf_size);
	model.kernel_param.kernel_type = atoi(buf);
    model_in.getline(buf,line_buf_size);
	model.kernel_param.poly_degree = atoi(buf);
	model_in.getline(buf,line_buf_size);
	model.kernel_param.rbf_gamma = atof(buf);
	model_in.getline(buf,line_buf_size);
	model.kernel_param.coef_lin = atof(buf);
	model_in.getline(buf,line_buf_size);
	model.kernel_param.coef_const = atof(buf);
	model_in.getline(buf,line_buf_size);
 	sscanf(buf,"%s%*[^\n]\n",model.kernel_param.custom);
	model_in.getline(buf,line_buf_size);
	model.subset_num1 = atoi(buf);
	model_in.getline(buf,line_buf_size);
	model.subset_num2 = atoi(buf);
	model_in.getline(buf,line_buf_size);
	model.totwords = atoi(buf);
	model_in.getline(buf,line_buf_size);
	model.total_sv_count = atoi(buf);
	model_in.getline(buf,line_buf_size);
	int model_num = model.subset_num1*model.subset_num2;
	//sv_count[i]
	model.sv_count = new int[model_num];
	model_in.getline(buf,line_buf_size);
	string line(buf);
	vector<string> splitted_line = split(line, " ");
	for(int i =0;i<model_num;i++)
	{
		model.sv_count[i] = atoi(splitted_line[i].c_str());
	}
	splitted_line.clear();
	if(model.kernel_param.kernel_type ==0) //linear
	{
		model_in.getline(buf,line_buf_size);
		model.lin_weights = new double*[model_num];
		for(int i =0;i<model_num;i++)
		{
			model.lin_weights[i] = new double[model.totwords+1];
			model_in.getline(buf,line_buf_size);
			line.assign(buf);
			splitted_line = split(line," ");
			for(int j =0;j<=model.totwords;j++)
			{
				model.lin_weights[i][j] = atof(splitted_line[j].c_str());
			}
			splitted_line.clear();	
		}
	}else
	{
		model_in.getline(buf,line_buf_size);
		model.SVs = new DOC[model.total_sv_count];
		for(int i =0;i<model.total_sv_count;i++)
		{
			model_in.getline(buf,line_buf_size);
			line.assign(buf);
			splitted_line = split(line," ");
			if(model.kernel_param.kernel_type == 2)//rbf
			{
				model.SVs[i].twonorm_sq = atof(splitted_line[0].c_str());
				splitted_line.erase(splitted_line.begin());
			}
			WORD* nodes = new WORD[splitted_line.size()];
			for(int j =0;j<splitted_line.size()-1;j++)
			{
				string one_node = splitted_line[j];
				nodes[j].wnum = atoi(one_node.c_str());
				nodes[j].weight = atof(one_node.substr(one_node.find(":")+1).c_str());
			}
			//debug
			nodes[splitted_line.size()-1].wnum =0;
			splitted_line.clear();
			model.SVs[i].words = nodes;
		}
		model_in.getline(buf,line_buf_size);
		model.sv_index = new int*[model_num];
		model.alphas = new double*[model_num];
		for(int i =0;i<model_num;i++)
		{
			model.sv_index[i] = new int[model.sv_count[i]];
			model.alphas[i] = new double[model.sv_count[i]];
			model_in.getline(buf,line_buf_size);
			line.assign(buf);
			splitted_line = split(line," ");
			for(int j =0;j<model.sv_count[i];j++)
			{
				string one_node = splitted_line[j];
				model.sv_index[i][j]= atoi(one_node.c_str());
				model.alphas[i][j] = atof(one_node.substr(one_node.find(":")+1).c_str());
			}
			splitted_line.clear();
		}
	}
	model_in.getline(buf,line_buf_size);
	model.thresholds = new double[model_num];
	model_in.getline(buf,line_buf_size);
	line.assign(buf);
	splitted_line = split(line," ");
	for(int i =0;i<model_num;i++)
	{
		model.thresholds[i]= atof(splitted_line[i].c_str());
	}	
	splitted_line.clear();
	model.deap_copy_words = true;
	model_in.close();
	delete[] buf;
	return 1;
}
void svm_light::free_model()
{
	delete[] model.sv_count;
	if(model.kernel_param.kernel_type == 0)//linear
	{
		for(int i =0;i<model.subset_num1*model.subset_num2;i++)
		{
			delete[] model.lin_weights[i];
		}
		delete[] model.lin_weights;
		if(!model.deap_copy_words)
		{
			delete[] model.SVs;
			for(int i =0;i<model.subset_num1*model.subset_num2;i++)
			{	
				delete[] model.sv_index[i];
				delete[] model.alphas[i];
			}
			delete[] model.sv_index;
			delete[] model.alphas;
		}
	}else{
			
		if(model.deap_copy_words)
		{
			for(int i =0;i<model.total_sv_count;i++)
			{
				delete[] model.SVs[i].words;
			}
		}	
		delete[] model.SVs;
		for(int i =0;i<model.subset_num1*model.subset_num2;i++)
		{
			delete[] model.sv_index[i];
			delete[] model.alphas[i];
		}
		delete[] model.sv_index;
		delete[] model.alphas;
	}
	delete[] model.thresholds;
}
