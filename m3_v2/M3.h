#ifndef _M3_H
#define _M3_H

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <map>
#include <string>

#include "mpi.h"

#include "m3_factory.h"
#include "data_split.h"
#include "parameter.h"
#include "util.h"
#include "divider.h"
#include "classifier.h"
#include "voter.h"
/* #include "hc_voter.h" */
/* #include "hyper_plane.h" */
/* #include "libsvm_parameter.h" */
/* #include "libsvm.h" */
/* #include "svm_light.h" */

using namespace std;

namespace M3{

  bool rank_master(int rank);
  bool rank_slave(int rank);
  bool rank_run(int rank);
  bool flag_train();
  bool flag_classify();
  bool flag_compare();
  bool flag_score();

  void parse();
  void initialize(int argc,
		  char * argv[]);
  void finalize();
  void load_train_data();
  void divide_train_data();
  void training_train_data();
  void classify_test_data();
  void score_test_data();
  void compare_true_label();

  class M3_Master{
  private:
    struct Train_Task_Info{
      int process_rank_1,process_rank_2;
      int subset_num_1,subset_num_2;
      int continue_size;

      int free_task;

      int task_num_1,task_num_2;
      int total_task;

      Train_Task_Info(int pr_1,
		      int pr_2,
		      int sn_1,
		      int sn_2,
		      int cs){
	process_rank_1=pr_1;
	process_rank_2=pr_2;
	subset_num_1=sn_1;
	subset_num_1=sn_1;
	subset_num_2=sn_2;
	continue_size=cs;

	free_task=0;
	task_num_1=subset_num_1/continue_size+((subset_num_1%continue_size)?1:0);
	task_num_2=subset_num_2/continue_size+((subset_num_2%continue_size)?1:0);
	total_task=task_num_1*task_num_2;
      }

      bool task_over(){
	return free_task>=total_task;
      }

      int left_1(){
	return (free_task/task_num_2)*continue_size;
      }

      int left_2(){
	return (free_task%task_num_2)*continue_size;
      }

      int right_1(){
	return min(left_1()+continue_size,subset_num_1)-1;
      }

      int right_2(){
	return min(left_2()+continue_size,subset_num_2)-1;
      }

      int length_1(){
	return right_1()-left_1()+1;
      }

      int length_2(){
	return right_2()-left_2()+1;
      }
    };

    struct Block_Index{
      int process;
      int index;

      bool operator < (const Block_Index & bi) const{
	return process<bi.process ||
	  index<bi.index;
      }
    };

    struct Pipe_Info{		/* for sematric pruning */
      float label_1,label_2;
      int start_offset,end_offset;
      int subset_num_1,subset_num_2;
      int total_level,total_subset_pair;

      vector<int> level_in_process;
      vector<vector<int> > level_index;
      vector<bool> process_use;

      Pipe_Info(){
	start_offset=0;
	end_offset=0;
	subset_num_1=0;
	subset_num_2=0;
	total_level=0;
	total_subset_pair=0;
	level_in_process.clear();
	level_index.clear();
	process_use.clear();
      }

      const bool operator < (const Pipe_Info & pi) const{
	return total_level<pi.total_level;
      }

      void info_complete_sematric_pruning(){
	subset_num_1=total_subset_pair/subset_num_2;
	total_level=subset_num_1+subset_num_2-1;
	for (int i=0;i<total_level;i++){
	  vector<int> tmp;
	  tmp.clear();
	  level_index.push_back(tmp);
	}
      }

      void info_complete_asematric_pruning(){
	subset_num_1=total_subset_pair/subset_num_2;
	total_level=subset_num_2;
	for (int i=0;i<total_level;i++){
	  vector<int> tmp;
	  tmp.clear();
	  level_index.push_back(tmp);
	}
      }

      void process_distribute(int total_process){
	for (int i=0;i<total_process;i++)
	  process_use.push_back(false);

	int avg=total_level/total_process;
	int pst=total_level%total_process;
	int pre=(pst/2)+(pst&1);
	int post=total_process-(pst-pre);
	int now=1,now_in=0;

	for (int i=0;i<total_level;i++){
	  if (now<=pre || now>post){
	    if (now_in>=avg+1){
	      now++;
	      now_in=0;
	    }
	  } else if (now_in>=avg){
	    now++;
	    now_in=0;
	  }
	  level_in_process.push_back(now);
	  process_use[now-1]=true;
	  now_in++;
	}
      }

      void process_transfor(int base){
	for (int i=0;i<process_use.size();i++)
	  process_use[i]=0;
	for (int i=0;i<level_in_process.size();i++){
	  int kk=level_in_process[i]+base-1;
	  level_in_process[i]=kk;
	  process_use[kk-1]=true;
	}
      }

      bool in_pipe(Subset_Info & si){
	return si.label_1==label_1 && si.label_2==label_2;
      }

      int x(int index){
	return index/subset_num_2;
      }

      int y(int index){
	return index%subset_num_2;
      }

      int level_sematric_pruning(int index){
	return x(index)+y(index);
      }

      int level_asematric_pruning(int index){
	return y(index);
      }

      int pre_process(int index){
	if (index==0) 
	  return -1;
	else return level_in_process[index-1];
      }

      int post_process(int index){
	if (index>=level_in_process.size()-1)
	  return -1;
	else return level_in_process[index+1];
      }

      int last_process(){
	return level_in_process[level_in_process.size()-1];
      }

      bool use_process(int id){
	if (id>0 && id<=process_use.size())
	  return process_use[id-1];
	return false;
      }
    };

    vector<Train_Task_Info> m_train_task_info;

    vector<Subset_Info> m_test_subset;

    vector<Pipe_Info> m_pipe_info;

    vector<bool> m_pipe_process_use;

    map<float,int> m_label_to_index;
    vector<float> m_index_to_label;

    map<float,vector<int> > m_label_to_process;
    map<int,double> m_process_to_label;
    map<int,int> m_process_train_data_num;
    map<int,int> m_process_train_subset_num;
    int m_free_process;
    int m_test_process_num;

    int m_il_process_delta;//the delta between this slave rank and pre slave rank
    int m_il_process_num;
    int subset_config_index;

    float string_to_float(char * str,
			  int ll,
			  int rr);

    int string_to_int(char * str,
		      int ll,
		      int rr);

    void parse_data(char * read_buf,
		    Data_Sample * data_sample);

    void data_package(Data_Sample * sample_buf,
		      Data_Node * node_buf,
		      Data_Sample * sample_buf_send,
		      Data_Node * node_buf_send,
		      int len,
		      float sample_label,
		      bool * been_sent,
		      int & sbs_len,
		      int & nsb_len);

    void check_load_data();

    void increase_learning_pre_load();//pre fill the information

    /* Not support the increase learning. */
    void load_train_data_serial(string file_name, // The file's name. 
				vector<bool> need_train_index); // The table point out which data need to be trained. 

    void load_train_data_parallel(string file_name);

    void check_divide_data();

    void handle_subset_info_write(ofstream & subset_config,
				  Subset_Info subset_info);

    int handle_subset_info_read(ifstream & config_in,
				Subset_Info & subset_info);

    void make_train_info();

    void save_for_increase_learning();

    void test_ctrl(int ctrl);

    void load_subset_config();

    bool ask_load_subset(int index);

    void classify_test_data_serial(string file_name,
				   vector<bool> test_flag);

    void classify_test_data_parallel();

    void classify_test_data_nonpruning(const string &);

    void make_pipe_info_pruning();

    void send_pipe_info_pruning(Pipe_Info & pi);

    void handle_pipe_score(vector<bool> & test_flag);	/* combine test and score */

    void classify_test_data_pruning();

    double get_min_score_full(int ll,
			      int rr,
			      ifstream * fin,
			      double sm,
			      bool flag);

    double get_min_score_min_vector(ifstream * fin,
				    double sm,
				    bool flag);

    void score_test_data_nonpruning(vector<bool> test_flag);

    void score_file_combine_sematric_pruning(); // faint for the finit file point in OS!!!!!!!!

    void score_test_data_sematric_pruning(vector<bool> test_flag);

  public:
    M3_Master();
    void load_train_data(const string &);
    void divide_train_data();
    void training_train_data();
    void classify_test_data();
    void score_test_data();
    void compare_true_label(const string &);
  };

  class M3_Slave{
  private:
    int m_train_data_num;
    bool m_memory_enough;

    // debug
    static const int DEBUG_MEMORY_CAPACITY=10;

    struct Sample_Link{
      Data_Sample * sample_head;
      Sample_Link * next;
      int length;
    };
    Sample_Link * m_sample_link_head,* m_sample_link_tail;

    Data_Sample ** m_sample_arr;
    vector<Divide_Info> m_divide_situation;

    float m_my_label;
    vector<Data_Sample **> m_sample_arr_pool;
    vector<vector<Divide_Info> > m_divide_situation_pool;

    int m_il_process_rank;
    map<int,vector<int> > m_il_index_inverse;//fill the sample arr pool

    float string_to_float(char * str,
			  int ll,
			  int rr);

    int string_to_int(char * str,
		      int ll,
		      int rr);

    void parse_data(char * read_buf,
		    Data_Sample * data_sample);


    void data_unpackage(Data_Sample * sample_buf,
			Data_Node * node_buf,
			int sp_buf_len,
			int nd_buf_len);

    void check_load_data();

    void increase_learning_pre_load();

    bool memory_test(int sp_buf_len,
		     int nd_buf_len);

    void load_train_data_serial();

    void load_train_data_parallel();

    static void local_memory_scarcity(){
      throw new exception;
    }

    void pre_divide();

    void check_divide_data();

    void save_for_increase_learning();

    void save_data_for_increase_learning(ofstream & os,
					 Data_Sample & ds);

    void subset_sample_package(int ll,
			       int rr,
			       Data_Sample * sample_buf,
			       int & nb_len);

    void subset_node_package(int sb_len,
			     Data_Sample * sample_buf,
			     Data_Node * node_buf);

  public:
    M3_Slave();
    ~M3_Slave();
    void load_train_data();
    void divide_train_data();
    void training_train_data();
    void classify_test_data();	// do nothing
  };

  class M3_Run{  
  private:

    struct Level_Info{
      int * x, * y;
      Subset_Info * si;
      int level;
      int length;
      Classifier ** classifier;
      int pre_process,post_process;

      Level_Info(int l,int len){
	level=l;
	length=len;
	x=new int[len];
	y=new int[len];
	si=new Subset_Info[len];
      }

      ~Level_Info(){
	delete [] x;
	delete [] y;
	delete [] si;

	for (int i=0;i<length;i++)
	  delete classifier[i];
	delete [] classifier;
      }

      bool in_level(int dx,int dy){
	return (dx-x[0]>=0 && dx-x[0]<length)
	  && (y[0]-dy>=0 && y[0]-dy<length);
      }

      int classifier_id(int dx,int dy){
	if (in_level(dx,dy))
	  return dx-x[0];
	else return -1;
      }
    };
    vector<Level_Info*> m_level_info;

    struct Pipe_Pool{
      vector<vector<Level_Info*> > level_info;
      vector<bool> mid_save;
      vector<float*> result;
      int len;
      int ptr;
      int r_id;

      Pipe_Pool(){
	level_info.clear();
	mid_save.clear();
      }

      ~Pipe_Pool(){
	for (int i=0;i<level_info.size();i++){
	  for (int j=0;j<level_info[i].size();j++)
	    delete level_info[i][j];
	  level_info[i].clear();
	}
	level_info.clear();


	for (int i=0;i<mid_save.size();i++)
	  if (mid_save[i])
	    delete [] result[i];
	result.clear();
	mid_save.clear();
      }

      void init(int l){
	len=l;
	r_id=0;

	result.clear();
	for (int i=0;i<mid_save.size();i++)
	  if (mid_save[i]){
	    result.push_back(new float[len]);
	  }
	  else {
	    result.push_back(NULL);
	  }
      }

      void clear(){
	ptr=0;
      }

      void clear_rid(){
	r_id=0;
      }

      void next_rid(){
	r_id++;
      }

      void push(float f){
	result[r_id][ptr++]=f;
      }

      void add(vector<Level_Info*> li,bool ms){
	level_info.push_back(li);
	mid_save.push_back(ms);
      }
    };
    Pipe_Pool m_pipe_pool;

    struct Mid_Level_Info_Sematric_Pruning{
      int * X,* Y;
      int * x,* y;
      float * dirct;
      int len;
      int nid;

      void new_mlisp(int l){
	len=l;
	nid=l;
	X=new int[len];
	Y=new int[len];
	x=new int[len];
	y=new int[len];
	dirct=new float [len];
      }

      void init(){
	nid=0;
      }

      void updata(){
	nid++;
      }

      void put(int fx,int fy,int fX,int fY,float fd){
	x[nid]=fx;
	y[nid]=fy;
	X[nid]=fX;
	Y[nid]=fY;
	dirct[nid]=fd;
      }

      void del_mlisp(){
	delete [] X;
	delete [] Y;
	delete [] x;
	delete [] y;
	delete [] dirct;
      }

      int get_X(){
	return X[nid];
      }

      int get_Y(){
	return Y[nid];
      }

      int get_x(){
	return x[nid];
      }

      int get_y(){
	return y[nid];
      }

      float get_d(){
	return dirct[nid];
      }

      float getn_d(){
	return dirct[nid];
      }

      int getn_X(){
	if (dirct[nid]<0)
	  return X[nid]+1;
	else return X[nid];
      }

      int getn_Y(){
	if (dirct[nid]<0)
	  return Y[nid];
	else return Y[nid]+1;
      }

      int getn_x(){
	if (dirct[nid]<0)
	  return 0;
	else return x[nid];
      }

      int getn_y(){
	if (dirct[nid]<0)
	  return y[nid];
	else return 0;
      }

    };
    Mid_Level_Info_Sematric_Pruning m_mlisp[2];
    int m_mlisp_now,m_mlisp_next;

    struct Mid_Level_Info_Asematric_Pruning{
      float * min_vector;
      int len;
      int test_num,line_num;
      /* the vector dim is line_num */
      /* ther are test_num datas be test once */

      void new_mliap(int tn,int ln){
	len=tn*ln;
	test_num=tn;
	line_num=ln;
	min_vector=new float[len];
      }

      void del_mliap(){
	delete [] min_vector;
      }

      int address(int id){
	return id*line_num;
      }

      void clear(){
	memset(min_vector,0,len*sizeof(float));
      }

      float max_vector(int id){
	int st=address(id);
	float mmax=0;
	bool flag=true;
	for (int i=0;i<line_num;i++)
	  if (flag){
	    mmax=min_vector[st+i];
	    flag=false;
	  }
	  else mmax=max(mmax,min_vector[st+i]);
	return mmax;
      }
    };
    Mid_Level_Info_Asematric_Pruning m_mliap;


    struct All_Test_Sample_Buf{
      Data_Sample * buf; /* the speed up for save all test data, buf */
      int len;
      int ptr;

      void init(){
	ptr=0;
      }

      void clear(){
	for (int i=0;i<len;i++){
	  delete [] buf[i].data_vector;
	}

	delete [] buf;
      }
    };
    All_Test_Sample_Buf m_atsb;

    float m_data_label_1,m_data_label_2; /* some data for train */
    int m_data_process_1,m_data_process_2;
    int m_data_subset_num_1,m_data_subset_num_2;
    Data_Sample ** m_sample_subset_1, ** m_sample_subset_2;
    int * m_sample_subset_len_1, * m_sample_subset_len_2;
    Data_Node * m_node_buf_1,* m_node_buf_2;
    Data_Sample * m_sample_buf_1,* m_sample_buf_2;
    int m_sample_len_1,m_sample_len_2;
    int m_node_len_1,m_node_len_2;

    float string_to_float(char * str,
			  int ll,
			  int rr);

    int string_to_int(char * str,
		      int ll,
		      int rr);

    void parse_data(char * read_buf,
		    Data_Sample * data_sample);

    void subset_sample_unpackage(int subset_num,
				 int * subset_len,
				 Data_Sample * sample_buf,
				 Data_Sample ** sample_subset,
				 int & node_len);

    void subset_node_unpackage(Data_Sample * sample_buf,
			       Data_Node * node_buf,
			       int sb_len);

    Classifier * make_classifier();

    void score_test_data_full(vector<Classifier*> & classifier,
			      vector<Subset_Info> & subset_info,
			      vector<ofstream*> & middle_score,
			      Data_Sample * sample_buf);

    void score_test_data_min_vector(vector<Classifier*> & classifier,
				    vector<Subset_Info> & subset_info,
				    vector<ofstream*> & middle_score,
				    Data_Sample * sample_buf);

    void only_classify_test_data_serial(MPI_Status & mpi_status,
					vector<Classifier*> & classifier,
					vector<Subset_Info> & subset_info,
					vector<ofstream*> & middle_score,
					Data_Sample * sample_buf,
					Data_Node * node_buf);

    void only_classify_test_data_parallel(string & file_name,
					  vector<bool> & test_flag,
					  vector<Classifier*> & classifier,
					  vector<Subset_Info> & subset_info,
					  vector<ofstream*> & middle_score,
					  Data_Sample * sample_buf,
					  Data_Node * node_buf);
    void make_level_info_classifier();

    void classify_test_data_nonpruning();

    int get_level_info_pruning();

    void pipe_get_mlisp(int len,
			Level_Info * li);

    void pipe_give_mlisp(int len,
			 Level_Info * li);

    void pipe_get_mliap(int len,
			Level_Info * li);

    void pipe_give_mliap(int len,
			 Level_Info * li);

    void pipe_get_sample_buf_from_file(ifstream & file_in,
				       char * read_buf,
				       Data_Sample * sample_buf,
				       Data_Node * node_buf,
				       int & sb_len,
				       int & nb_len,
				       int & index,
				       vector<bool> & test_flag);

    void make_all_test_sample_buf(string file_name,
				  vector<bool> test_flag);

    void make_all_test_sample_buf(ifstream & file_name,
				  vector<bool> test_flag,
				  int & index); /* for combine score */

    Data_Sample * pipe_get_sample_buf_from_all_data_buf(int & sb_len);

    void pipe_level_classify_sematric_pruning(int mlevel,
					      Data_Sample * sample_buf,
					      int sb_len);

    void pipe_level_classify_asematric_pruning(int mlevel,
					       Data_Sample * sample_buf,
					       int sb_len);

    void pipe_classify_test_data_sematric_pruning(string file_name,
						  vector<bool> test_flag,
						  bool mid_score_save);

    void pipe_classify_test_data_asematric_pruning(string file_name,
						   vector<bool> test_flag,
						   bool mid_score_save);

    void classify_test_data_pruning(string file_name,
				    vector<bool> test_flag);

    void classify_and_score_test_data_pruning(string file_name,
					      vector<bool> test_flag);

  public:
    M3_Run();
    ~M3_Run();
    void load_train_data();	// do nothing
    void divide_train_data();	// do nothing
    void training_train_data();
    void classify_test_data();
  };

  static M3_Parameter * m3_parameter;

  static int m3_all_process_num;
  static int m3_my_rank;

  static int m3_start_slave_process_rank; // denote the slave process num start
  static int m3_subset_size;
  static int m3_continue_subset_size;

  static int M3_TAG=0;	// point out the recive tag
  static int M3_SUBSET_INFO_TAG=9999; // special tag for free response

  static MPI_Datatype MPI_Data_Node;  // need to be commit
  static MPI_Datatype MPI_Data_Sample;
  static MPI_Datatype MPI_Subset_Info;

  static M3_Master * m3_master;
  static M3_Slave * m3_slave;
  static M3_Run * m3_run;

  static ofstream debug_out;
  static double m3_start_time;
 
  const int M3_MASTER_RANK=0; // master must be rank 0

  // There are some mistake that define the follow as const, so I define them as static
  static int CTRL_MEMORY_ENOUGH=0;
  static int CTRL_MEMORY_SCARCITY=1;
  // For load
  static int CTRL_READ_DONE=0;
  static int CTRL_ALLOC_MEMORY=1; // serial
  static int CTRL_GET_DATA=2;	
  static int CTRL_TRAIN_LOAD_FULL=3;	  // parallel
  // For train
  static int CTRL_TRAIN_DONE=0;
  static int CTRL_TRAIN_CONTINUE=1;
  // For test
  static int CTRL_TEST_DONE=0;
  static int CTRL_TEST_CLEAR=1;
  static int CTRL_LOAD_SUBSET=2;
  static int CTRL_CLASSIFY_DATA=3;
  // For test & score combine
  static int CTRL_TS_DONE=0;
  static int CTRL_TS_CONTINUE=1;

  // define some length
  static int NODE_BUF_SIZE=1024*1024*100; // May be defined by para
  static int NODE_SIMPLE_BUF_SIZE=1024*1024; // node num in one sample
  static int SAMPLE_BUF_SIZE=100;
  static const int READ_BUF_SIZE=1024*1024*10;
  static int TEST_SAMPLE_BUF_SIZE=SAMPLE_BUF_SIZE;
  static int TEST_NODE_BUF_SIZE=NODE_BUF_SIZE;
  static const int PRUNING_LEVEL_LENGTH=1000;
  static int TEST_AND_SCORE_ALL_DATA_BUF_SIZE=TEST_SAMPLE_BUF_SIZE*100; /* 100 level pipe once */

  const string LABEL_INDEX_CONFIG="labelIndex.config";
  const string SUBSET_CONFIG="subset.config";
  const string SUBSET_DIR="Subset/";
  const string SCORE_DIR="Score/";
  const string RESULT="result";
  const string RESULT_COMPARE="result_compare";
  const string RESULT_MATRIX="result.matrix";
  const string RESULT_CONFIDENT="result.confident";
  const string DIVID_DATA_IL_DIR="Divide_Data_IL/";
  const string DEFAULT_IL_SUBSET_CONFIG="subset.config_il";
  const string DEFAULT_IL_CONFIG="increase_learning.config";
};

#endif
