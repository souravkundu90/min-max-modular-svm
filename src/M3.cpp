// !!!WARNING:There are something unfinished:
// !!!WARNING:Initialize and finalize must be finished.
// !!!WARNING:All block that I mark "// do something to handle" must be filled.

// !!!WARNING:All block that I mark "// debug" and follow until blank line which changed something must be delete.
// !!!WARNING:All block that I mark "// debug" and follow until blank line which not changed anything could be delete.
// !!!WARNING:   and others.


#ifndef _M3_CPP
#define _M3_CPP

#include "M3.h"

#include "m3_master.h"
#include "m3_slave.h"
#include "m3_run.h"
using namespace std;






// Test whether this process is master.
bool M3::rank_master(int rank){
    return rank==M3_MASTER_RANK;
}

// Test whether this process is slave.
bool M3::rank_slave(int rank){
    return rank>=m3_start_slave_process_rank;
}

// Test whether this process is run.
bool M3::rank_run(int rank){
    return rank && rank<m3_start_slave_process_rank;
}

bool M3::flag_train(){
    return m3_parameter->flag_train;
}

bool M3::flag_classify(){
    return m3_parameter->flag_classify;
}

bool M3::flag_compare(){
    return m3_parameter->flag_compare;
}

bool M3::flag_score(){
    return m3_parameter->flag_score;
}

void M3::parse(){
    user_config * uc=new user_config;
    m3_parameter=new M3_Parameter("m3.config");

    //m3_parameter=new M3_Parameter();
    delete uc;
}

void M3::initialize(int argc,char * argv[]){

    // There are some parameter need to be determined.
    // For easy to test, I only to let them be a small const
    m3_start_slave_process_rank=m3_parameter->running_process_num+1;
    m3_continue_subset_size=m3_parameter->modular_size;
    m3_subset_size=m3_parameter->subset_size;

    int init_flag;
    MPI_Initialized(&init_flag);
    if (init_flag){
        // do something to handle this error
    }

    // Initialize and get some process informaiton

    m3_start_time=MPI_Wtime();

    MPI_Comm_size(MPI_COMM_WORLD,
        &m3_all_process_num);
    MPI_Comm_rank(MPI_COMM_WORLD,
        &m3_my_rank);

    if (rank_master(m3_my_rank)){
        system("mkdir Subset");
        system("mkdir Score");
        system("mkdir Divide_Data_IL");
    }


    // Middle infomation file name
    string debug_name="out.debug_";
    char name_tmp[20];
    sprintf(name_tmp,
        "%d",
        m3_my_rank);
    debug_name+=name_tmp;
    debug_out.open(debug_name.c_str());

    //Commit Data_Sample & Data_Node
    int data_node_len[2];
    data_node_len[0]=1;
    data_node_len[1]=1;
    MPI_Datatype data_node_type[2];
    data_node_type[0]=MPI_INT;
    data_node_type[1]=MPI_FLOAT;
    MPI_Aint data_node_offset[2];
    data_node_offset[0]=0;
    data_node_offset[1]=sizeof(MPI_INT);
    MPI_Type_struct(2,
        data_node_len,
        data_node_offset,
        data_node_type,
        &MPI_Data_Node);
    MPI_Type_commit(&MPI_Data_Node);

    int data_sample_len[6];
    data_sample_len[0]=1;
    data_sample_len[1]=1;
    data_sample_len[2]=1;
    data_sample_len[3]=1;
    data_sample_len[4]=1;
    data_sample_len[5]=1;
    MPI_Datatype data_sample_type[6];
    data_sample_type[0]=MPI_INT;
    data_sample_type[1]=MPI_FLOAT;
    data_sample_type[2]=MPI_INT;
    data_sample_type[3]=MPI_POINTER;	// pointer as int
    data_sample_type[4]=MPI_INT;
    data_sample_type[5]=MPI_POINTER;	// pointer as int
    MPI_Aint data_sample_offset[6];
    data_sample_offset[0]=0;
    data_sample_offset[1]=data_sample_offset[0]+sizeof(MPI_INT);
    data_sample_offset[2]=data_sample_offset[1]+sizeof(MPI_FLOAT);
    data_sample_offset[3]=data_sample_offset[2]+sizeof(MPI_INT);
    data_sample_offset[4]=data_sample_offset[3]+sizeof(MPI_POINTER);
    data_sample_offset[5]=data_sample_offset[4]+sizeof(MPI_INT);
    MPI_Type_struct(6,
        data_sample_len,
        data_sample_offset,
        data_sample_type,
        &MPI_Data_Sample);
    MPI_Type_commit(&MPI_Data_Sample);

    int subset_info_len[13];
    for (int i=0;i<13;i++)
        subset_info_len[i]=1;
    MPI_Datatype subset_info_type[13];
    subset_info_type[0]=MPI_FLOAT;
    subset_info_type[1]=MPI_FLOAT;
    for (int i=2;i<13;i++)
        subset_info_type[i]=MPI_INT;
    MPI_Aint subset_info_offset[13];
    subset_info_offset[0]=0;
    subset_info_offset[1]=subset_info_offset[0]+sizeof(MPI_FLOAT);
    subset_info_offset[2]=subset_info_offset[1]+sizeof(MPI_FLOAT);
    for (int i=3;i<13;i++)
        subset_info_offset[i]=subset_info_offset[i-1]+sizeof(MPI_INT);
    MPI_Type_struct(13,
        subset_info_len,
        subset_info_offset,
        subset_info_type,
        &MPI_Subset_Info);
    MPI_Type_commit(&MPI_Subset_Info);

    if (rank_master(m3_my_rank))
        m3_master=new M3_Master;
    else if (rank_slave(m3_my_rank))
        m3_slave=new M3_Slave;
    else if (rank_run(m3_my_rank))
        m3_run=new M3_Run;
}

void M3::finalize(){

  MPI_Type_free(&MPI_Data_Node);
  MPI_Type_free(&MPI_Data_Sample);
  MPI_Type_free(&MPI_Subset_Info);

  // debug
  TIME_DEBUG_OUT << "Total train time: " << link_train_time << endl;
  TIME_DEBUG_OUT << "Total predict time: " << link_predict_time << endl;

  delete m3_parameter;

  if (rank_master(m3_my_rank))
    delete m3_master;
  else if (rank_slave(m3_my_rank))
    delete m3_slave;
  else if (rank_run(m3_my_rank))
    delete m3_run;

  debug_out.close();

  MPI_Finalize();
}

void M3::load_train_data(){
  if (rank_master(m3_my_rank)){

    // debug
    // running flag
    cout << "The master is prepare reading & sent the job to all slave" << endl;

    m3_master->load_train_data(m3_parameter->train_data);
  }
  else if (rank_slave(m3_my_rank)){
    m3_slave->load_train_data();

    // debug
    // running flag
    cout << "The slave process " << m3_my_rank << " has load all the data " << endl;

  }
  else if (rank_run(m3_my_rank)){
    // do something to handle this error
  }

  MPI_Barrier(MPI_COMM_WORLD);
}

void M3::divide_train_data(){

  if (rank_master(m3_my_rank)){
    m3_master->divide_train_data();
  }
  else if (rank_slave(m3_my_rank)){
    m3_slave->divide_train_data();
  }
  else if (rank_run(m3_my_rank)){
    // do something to handle this error
  }
  
  // debug
  // running flag
  cout << "The process " << m3_my_rank << " has divided the data " << endl;


  MPI_Barrier(MPI_COMM_WORLD);
}

void M3::training_train_data(){
  if (rank_master(m3_my_rank))
    m3_master->training_train_data();
  else if (rank_slave(m3_my_rank)){
    m3_slave->training_train_data();
  }
  else if (rank_run(m3_my_rank))
    m3_run->training_train_data();

  // debug
  // running flag
  cout << "The process " << m3_my_rank << " has train done " << endl;

  MPI_Barrier(MPI_COMM_WORLD);
}

void M3::classify_test_data(){
  if (rank_master(m3_my_rank))
    m3_master->classify_test_data();
  else if (rank_run(m3_my_rank))
    m3_run->classify_test_data();

  // debug
  // running flag
  cout << "The process " << m3_my_rank << " has classify done " << endl;

  MPI_Barrier(MPI_COMM_WORLD);
}

void M3::score_test_data(){
  if (rank_master(m3_my_rank))
    m3_master->score_test_data();

  MPI_Barrier(MPI_COMM_WORLD);
}

void M3::compare_true_label(){
  if (rank_master(m3_my_rank))
    m3_master->compare_true_label(m3_parameter->compare_label_data);

  MPI_Barrier(MPI_COMM_WORLD);
}



























//M3_Master


// Master initialize.
M3::M3_Master::M3_Master(){
  m_label_to_process.clear();
  m_process_to_label.clear();
  m_process_train_data_num.clear();
  m_process_train_subset_num.clear();
  m_label_to_index.clear();
  m_index_to_label.clear();

  m_free_process=m3_start_slave_process_rank;
  subset_config_index=0;
}

// As name
float M3::M3_Master::string_to_float(char * str,int ll,int rr){
  char temp = str[rr];
  str[rr] = 0;
  float res = atof(&(str[ll]));
  str[rr] = temp;
  return res;
}

// As name
int M3::M3_Master::string_to_int(char * str,int ll,int rr){
  char temp = str[rr];
  str[rr] = 0;
  int res = atoi(&(str[ll]));
  str[rr] = temp;
  return res;
}

// Parse input data to our format.
void M3::M3_Master::parse_data(char * rbuf,Data_Sample * dsp){
    int i=0,pri=0;
  int len=0;

  dsp->mlabel_len=1;
  while (rbuf[i]!=' ') {
     i++; 
    dsp->mlabel_len+=(rbuf[i]==',');
  }

  dsp->mlabel=new float[dsp->mlabel_len];
  i=-1;
  for (int k=0;k<dsp->mlabel_len;k++){
      pri=++i;
      while (rbuf[i]!=' ' && rbuf[i]!=',') i++;
      dsp->mlabel[k]=string_to_float(rbuf,pri,i);
  }
  dsp->label=dsp->mlabel[0];

  while (rbuf[i]){
        pri=++i;
        while (rbuf[i] && rbuf[i]!=':') i++;
        if (!rbuf[i]) break;
        dsp->data_vector[len].index=string_to_int(rbuf,pri,i);
        pri=++i;
        while (rbuf[i] && rbuf[i]!=' ') i++;
        dsp->data_vector[len].value=string_to_float(rbuf,pri,i);
        len++;
    }
    dsp->data_vector_length=len;
}

// Package sample_buf & node_buf to be continue memory space.
// For MPI_Send.
void M3::M3_Master::data_package(Data_Sample * sample_buf,
                                 Data_Node * node_buf,
                                 Data_Sample * sample_buf_send,
                                 Data_Node * node_buf_send,
                                 int len,
                                 float sp_l,
                                 bool * been_sent,
                                 int & sbs_len,
                                 int & nbs_len){

    int sbs_offset=0,nbs_offset=0;

    sbs_len=0;
    nbs_len=0;
    int i;
    for (i=0;i<len;i++)
        if (sp_l==sample_buf[i].label){
            been_sent[i]=true;
            sample_buf_send[sbs_len]=sample_buf[i];

            sbs_len++;
            for (int j=0;j<sample_buf[i].data_vector_length;j++)
                node_buf_send[nbs_len+j]=sample_buf[i].data_vector[j];
            nbs_len+=sample_buf[i].data_vector_length;
        }
}

// Print middle information
void M3::M3_Master::check_load_data(){
  TIME_DEBUG_OUT << "Master use " 
		 << m_free_process-m3_start_slave_process_rank 
		 << " to store data!\n"
		 << endl;
  
  int i;
  for (i=m3_start_slave_process_rank;i<m_free_process;i++){
    TIME_DEBUG_OUT << "Process " 
		   << i 
		   << " store the data with label: " 
		   << m_process_to_label[i]
		   << " and num: "
		   << m_process_train_data_num[i]
		   << endl;
  }

  map<float,vector<int> >::iterator it;
  for (it=m_label_to_process.begin();it!=m_label_to_process.end();it++){
    vector<int> pr=(*it).second;
    TIME_DEBUG_OUT << "Label " 
		   << (*it).first
		   << " has store in "
		   << pr.size()
		   << " process(s) "
		   << endl;
    TIME_DEBUG_OUT << "They are: ";
    int i;
    for (i=0;i<pr.size();i++)
      debug_out << pr[i] 
		<< " ";
    debug_out << endl;
  }
}

// Main load block.
// file_name is the name of data file.
// need_train_index is bool arr that determine which data need to be think as train data.
// MODE:master read & sent, slave recieve
void M3::M3_Master::load_train_data_serial(string file_name,
					   vector<bool> need_train_index){

	
	
  // debug
  TIME_DEBUG_OUT << "master being to init its ele" << endl;

  char * read_buf=new char[READ_BUF_SIZE];

  Data_Node * node_buf=new Data_Node[NODE_BUF_SIZE];
  Data_Node * node_buf_send=new Data_Node[NODE_BUF_SIZE];
  Data_Sample * sample_buf=new Data_Sample[SAMPLE_BUF_SIZE];
  Data_Sample * sample_buf_send=new Data_Sample[SAMPLE_BUF_SIZE];
  bool * been_sent=new bool[SAMPLE_BUF_SIZE];

  bool read_done_flag=false;
  int total_index=0;

  ifstream file_in(file_name.c_str());

  // debug
  TIME_DEBUG_OUT << "master all ele has bee init & file:" 
		 << file_name 
		 << " has been opend" 
		 << endl;

  while (!read_done_flag){

    // debug
    TIME_DEBUG_OUT << "master loop beging now!" << endl;

    int index=0,nb_offset=0;
    // Read data from data file.
    // This loop get out only when read over or the buf is full.
    while (index<SAMPLE_BUF_SIZE){

      // Read a line buf.
      memset(read_buf,
	     0,
	     sizeof(char)*READ_BUF_SIZE);
      if (!file_in.getline(read_buf,
			   READ_BUF_SIZE)){
	read_done_flag=true;	// the input is over
	file_in.close();

	// debug
	TIME_DEBUG_OUT << "master has close the file:" << file_name << endl;

	break;
      }      
      
      if (need_train_index[total_index]){
	sample_buf[index].data_vector=&node_buf[nb_offset];
	parse_data(read_buf,
		   &(sample_buf[index])); // parser data to our struct
	nb_offset+=sample_buf[index].data_vector_length;
	sample_buf[index++].index=total_index;
      }
      total_index++;
    }

    // running flag
    cout << "Load " << total_index << "/" << need_train_index.size() 
	 << " :  at " << (100.0*total_index)/(1.0*(need_train_index.size()))
	 << "%" << endl;

    // debug
    TIME_DEBUG_OUT << "master has read_buf done!index && total_index: " 
		   << index 
		   << " " 
		   << total_index 
		   << endl;

    memset(been_sent,0,sizeof(bool)*SAMPLE_BUF_SIZE);
    while (true){
      int i;
      // Determine whether there is data which has not be send.
      for (i=0;i<index && been_sent[i];i++);
      if (i>=index) {

	// debug
	TIME_DEBUG_OUT << "this loop, no data need to be sent now !" << endl;

	break;	// all data has been send to salve;
      }

      // The i is the data's index that has not be send.
      // sp_l is the label of index i.
      float sp_l=sample_buf[i].label;
      int sbs_len,nbs_len;

      // debug
      TIME_DEBUG_OUT << "now, master sent lable:@" 
		     << i 
		     << " " 
		     << sample_buf[i].label 
		     << endl;

      // Package the all data in buf that label is sp_l as continue memory.
      data_package(sample_buf,
		   node_buf,
		   sample_buf_send,
		   node_buf_send,
		   index,
		   sp_l,
		   been_sent,
		   sbs_len,
		   nbs_len);

      // debug
      TIME_DEBUG_OUT << "Master has package down!" << endl;

      // Find the first slave_process to be ask to store data that lable sp_l
      int slave_rank=0;
      if (m_label_to_process.find(sp_l)!=m_label_to_process.end()){
	// If the sp_l is an old label that there is some process store.
	// Let the asked slave_process to be the last one that store data which is label sp_l.
	int stack_l=m_label_to_process[sp_l].size();
	slave_rank=m_label_to_process[sp_l][stack_l-1];
      }
      else {
	if (m_free_process==m3_all_process_num){
	  // all memory are not enough
	  // do something to handle this error
	}
	// Unless let asked slave_process be a new free process(if there exits one).
	slave_rank=m_free_process++;
      }

      int ask_len[2];
      ask_len[0]=sbs_len;
      ask_len[1]=nbs_len;
      int slave_alloc_flag=0;
      while (true){		// ask slave to alloc

	// debug
	TIME_DEBUG_OUT << "Master ask slave_process: " << slave_rank << endl;

	// Send the ask space control.
	MPI_Send(&CTRL_ALLOC_MEMORY,
		 1,
		 MPI_INT,
		 slave_rank,
		 M3_TAG,
		 MPI_COMM_WORLD);	

	// debug
	TIME_DEBUG_OUT << "Master ask memory: " 
		       << ask_len[0] 
		       << " " 
		       << ask_len[1] 
		       << endl;

	// Send the space information.
	MPI_Send(ask_len,
		 2,
		 MPI_INT,
		 slave_rank,
		 M3_TAG+1,
		 MPI_COMM_WORLD);

	// Get the answer from asked slave.
	MPI_Status mpi_status;
	MPI_Recv(&slave_alloc_flag,
		 1,
		 MPI_INT,
		 slave_rank,
		 M3_TAG,
		 MPI_COMM_WORLD,
		 &mpi_status);

	// If the asked slave haa no space to locate data.
	// Let the asked slave be a new free process(if exits one).
	if (slave_alloc_flag==CTRL_MEMORY_SCARCITY){
	  if (m_free_process==m3_all_process_num){
	    // all memory are not enough
	    // do something to handle this error
	  }
	  slave_rank=m_free_process++;
	}
	else break;		// now slave_rank has enough memory
      }

      // debug
      TIME_DEBUG_OUT << "master decide to sent " 
		     << sp_l 
		     << " to slave_process: " 
		     <<slave_rank 
		     << endl;

      // Save some information table.
      m_process_to_label[slave_rank]=sp_l;
      if (m_label_to_index.find(sp_l)==m_label_to_index.end()){
	m_label_to_index[sp_l]=m_index_to_label.size();
	m_index_to_label.push_back(sp_l);
      }
      if (m_process_train_data_num.find(slave_rank)==m_process_train_data_num.end())
	m_process_train_data_num[slave_rank]=sbs_len;
      else m_process_train_data_num[slave_rank]+=sbs_len;
      if (m_label_to_process.find(sp_l)==m_label_to_process.end()){
	vector<int> tmp;
	tmp.clear();
	tmp.push_back(slave_rank);
	m_label_to_process[sp_l]=tmp;
      }
      else {
	int tmp_l=m_label_to_process[sp_l].size();
	if (m_label_to_process[sp_l][tmp_l-1]!=slave_rank)
	  m_label_to_process[sp_l].push_back(slave_rank);
      }

      // debug
      TIME_DEBUG_OUT << "master begin to sent data && the CTRL:" 
		     << CTRL_GET_DATA 
		     << endl;

      // debug
      // check_send_buf
      //       TIME_DEBUG_OUT << "master check sent buf" << endl;
      //       for (int i=0;i<sbs_len;i++)
      //       	TIME_DEBUG_OUT << sample_buf_send[i].index << " "
      // 		       << sample_buf_send[i].label << " "
      // 		       << sample_buf_send[i].data_vector_length << endl;
      //       for (int i=0;i<nbs_len;i++){
      //       	if (i%8==0) debug_out << endl;
      //       	debug_out << "(" << node_buf_send[i].index << "," << node_buf_send[i].value << ")";
      //       }
      //       debug_out << endl;

      // Send data.
      MPI_Send(&CTRL_GET_DATA,
	       1,
	       MPI_INT,
	       slave_rank,
	       M3_TAG,
	       MPI_COMM_WORLD);
      MPI_Send(sample_buf_send,
	       sbs_len,
	       MPI_Data_Sample,
	       slave_rank,
	       M3_TAG+1,
	       MPI_COMM_WORLD);
      MPI_Send(node_buf_send,
	       nbs_len,
	       MPI_Data_Node,
	       slave_rank,
	       M3_TAG+2,
	       MPI_COMM_WORLD);
    }

    // debug
    TIME_DEBUG_OUT << "master loop over now ! " << endl;

  }

  // debug
  TIME_DEBUG_OUT << "master has read done! " << endl;


  for (int i=m3_start_slave_process_rank;i!=m3_all_process_num;i++){
    
    // debug
    TIME_DEBUG_OUT << "master sent to slave_process: " 
		   << i 
		   << " to load over" 
		   << endl;

    MPI_Send(&CTRL_READ_DONE,
	     1,
	     MPI_INT,
	     i,
	     M3_TAG,
	     MPI_COMM_WORLD); //sent load ok!
  }

  // debug
  TIME_DEBUG_OUT << "master sent loadover to all slave done" << endl;
  
  // Delete buf.
  delete [] read_buf;
  delete [] sample_buf;
  delete [] sample_buf_send;
  delete [] node_buf;
  delete [] node_buf_send;
  delete [] been_sent;

  // debug
  //  check_load_data();

  sort(m_index_to_label.begin(),m_index_to_label.end());
  ofstream li_fout(LABEL_INDEX_CONFIG.c_str());
  for (int i=0;i<m_index_to_label.size();i++){
    m_label_to_index[m_index_to_label[i]]=i;
    li_fout << m_index_to_label[i] << endl;
  }
  li_fout.close();

};

void M3::M3_Master::increase_learning_pre_load(){
  m_il_process_num=m3_parameter->pre_data_process;
  m_il_process_delta=m3_parameter->pre_data_process_rank[0]-m3_start_slave_process_rank;//if there are more or less run process this time
  m_free_process=m3_start_slave_process_rank+m_il_process_num;
  for (int i=m3_start_slave_process_rank;i<m3_all_process_num;i++){
    int ss=-1;
    if (i<m_free_process)
      ss=m3_parameter->pre_data_process_rank[i-m3_start_slave_process_rank];
    MPI_Send(&ss,
	     1,
	     MPI_INT,
	     i,
	     M3_TAG,
	     MPI_COMM_WORLD);
    // The ith process load the pre data in the ssth process
  }

  for (int i=0;i<m_il_process_num;i++){

    // debug
    TIME_DEBUG_OUT << "Master handle pre load in increase learning" << endl;

    int pid=m3_parameter->pre_data_process_rank[i];
    float ll=m3_parameter->pre_data_label[i];
    int num=m3_parameter->pre_data_num[i];
    int dnum=m3_parameter->pre_data_divide_num[i];

    int newpid=m3_start_slave_process_rank+i;

    if (m_label_to_process.find(ll)==m_label_to_process.end()){
      vector<int> vi;
      vi.clear();
      m_label_to_process[ll]=vi;
    }
    m_label_to_process[ll].push_back(newpid);
    m_process_to_label[newpid]=ll;
    m_process_train_data_num[newpid]=num;
    m_process_train_subset_num[newpid]=dnum;
  }

  ifstream config_in(m3_parameter->pre_subset_config.c_str());
  ofstream config_out(SUBSET_CONFIG.c_str());
  Subset_Info ts;
  while (handle_subset_info_read(config_in,ts)){
    ts.process_1+=m_il_process_delta;
    ts.process_2+=m_il_process_delta;
    handle_subset_info_write(config_out,ts);
    subset_config_index++;
  }
  config_out.close();
  config_in.close();

}

void M3::M3_Master::load_train_data_parallel(string file_name){
  // information define: int info[4];char label_file[1000];
  // info[0]=situation tag
  // info[1]=process rank
  // info[2]=now file offset
  // info[3]=sample num
  int info[4];
  char label_file[1000];

  MPI_Status mpi_status;

  // debug
  TIME_DEBUG_OUT << "master begin to split train data as label" << endl;

  Data_Split * data_split=new Data_Split(file_name,READ_BUF_SIZE);
  if (!m3_parameter->m3_multilabel)
    data_split->split();
  else data_split->multi_split();
  delete data_split;

  // debug
  TIME_DEBUG_OUT << "master split train data done" << endl;

  ifstream data_config("Split_Data/data.config");
  map<float,string> label_file_name;
  map<float,string>::iterator it;
  label_file_name.clear();
  string tmp_file_name;
  double f_label;
  while (data_config >> f_label >> tmp_file_name){
    label_file_name[f_label]=tmp_file_name;
    m_label_to_index[f_label]=m_index_to_label.size();
    m_index_to_label.push_back(f_label);
  }
  data_config.close();

  if (m3_parameter->m3_increase_learning)
    increase_learning_pre_load();

  // debug
  TIME_DEBUG_OUT << "master has load the split data config" << endl;
  for (it=label_file_name.begin();it!=label_file_name.end();it++)
    TIME_DEBUG_OUT << "label " << (*it).first << " file is " << (*it).second << endl;

  // debug
  TIME_DEBUG_OUT << "master begin to send job at first time" << endl;

  it=label_file_name.begin();
  while (1){
    if (m_free_process>=m3_all_process_num){
      // do something to handle this error
      break;
    }
    if (it==label_file_name.end())
      break;

    info[0]=CTRL_GET_DATA;
    info[1]=m_free_process;
    info[2]=0;
    info[3]=0;

    string tmps=(*it).second;
    memset(label_file,0,sizeof(char)*1000);
    for (int i=0;i<tmps.length();i++)
      label_file[i]=tmps[i];

    // debug
    TIME_DEBUG_OUT << "master send the label " << (*it).first
		   << " at file " << label_file
		   << " with offset " << info[2]
		   << " to slave_process " << m_free_process << endl;

    MPI_Send(info,
	     4,
	     MPI_INT,
	     m_free_process,
	     M3_TAG,
	     MPI_COMM_WORLD);
    MPI_Send(label_file,
	     1000,
	     MPI_CHAR,
	     m_free_process,
	     M3_TAG,
	     MPI_COMM_WORLD);

    m_process_to_label[m_free_process]=(*it).first;
    vector<int> pro_tmp;
    pro_tmp.clear();
    pro_tmp.push_back(m_free_process);
    m_label_to_process[(*it).first]=pro_tmp;

    m_free_process++;
    it++;
  }

  // debug
  TIME_DEBUG_OUT << "master has finished the first send job" << endl;

  int task_rest=m_index_to_label.size();

  while (task_rest){
    MPI_Recv(info,
	     4,
	     MPI_INT,
	     MPI_ANY_SOURCE,
	     M3_SUBSET_INFO_TAG,
	     MPI_COMM_WORLD,
	     &mpi_status);

    int situation=info[0];
    int free_process=info[1];
    int file_offset=info[2];
    int sample_num=info[3];

    // debug
    TIME_DEBUG_OUT << "master get response from " << free_process 
		   << " that load is " << situation 
		   << " & file at " << file_offset
		   << " with has load " << sample_num 
		   << " samples " << endl;

    if (m_process_train_data_num.find(free_process)==m_process_train_data_num.end())
      m_process_train_data_num[free_process]=sample_num;
    else m_process_train_data_num[free_process]+=sample_num;

    if (situation==CTRL_READ_DONE){

      // debug
      TIME_DEBUG_OUT << "label " << m_process_to_label[free_process]
		     << " is load over " << endl;

      task_rest--;
      continue;
    }

    // debug 
    TIME_DEBUG_OUT << "process " << free_process 
		   << " is too full !" << endl;

    if (m_free_process==m3_all_process_num){
      // do something to handle this error
    }

    info[0]=CTRL_GET_DATA;
    info[1]=m_free_process;
    info[2]=file_offset;
    info[3]=0;

    float f_label=m_process_to_label[free_process];
    string tmps=label_file_name[f_label];
    memset(label_file,0,sizeof(char)*1000);
    for (int i=0;i<tmps.length();i++)
      label_file[i]=tmps[i];

    // debug
    TIME_DEBUG_OUT << "master send the label " << f_label
		   << " at file " << label_file
		   << " with offset " << info[2]
		   << " to slave_process " << m_free_process << endl;

    MPI_Send(info,
	     4,
	     MPI_INT,
	     m_free_process,
	     M3_TAG,
	     MPI_COMM_WORLD);
    MPI_Send(label_file,
	     1000,
	     MPI_CHAR,
	     m_free_process,
	     M3_TAG,
	     MPI_COMM_WORLD);

    m_process_to_label[m_free_process]=f_label;
    m_label_to_process[f_label].push_back(m_free_process);

    m_free_process++;
  }

  // debug
  TIME_DEBUG_OUT << "master has parallel load over & send to all slave done" << endl;

  for (int i=m_free_process;i<m3_all_process_num;i++){
    info[0]=CTRL_READ_DONE;
    MPI_Send(info,
	     4,
	     MPI_INT,
	     i,
	     M3_TAG,
	     MPI_COMM_WORLD);
  }

  // debug
  TIME_DEBUG_OUT << "master has parallel load done!" << endl;

  sort(m_index_to_label.begin(),m_index_to_label.end());
  ofstream li_fout(LABEL_INDEX_CONFIG.c_str());
  for (int i=0;i<m_index_to_label.size();i++){
    m_label_to_index[m_index_to_label[i]]=i;
    li_fout << m_index_to_label[i] << endl;
  }
  li_fout.close();

}

void M3::M3_Master::load_train_data(const string & filename){

  // debug
  TIME_DEBUG_OUT << "come in the master load_train_data" << endl;

  if (m3_parameter->m3_load_mode==0){
    load_train_data_parallel(filename);
  }
  else if (m3_parameter->m3_load_mode==1){
    vector<bool> tmp;
    tmp.clear();
    int i;
    size_t len = M3::trainLen = M3::wc_l(filename);
    for (i=0;i<len;i++) tmp.push_back(true);
    load_train_data_serial(filename.c_str(),tmp);
  }

  // debug
  TIME_DEBUG_OUT << "go out the master load_train_data" << endl;

}

// Prin middle information.
void M3::M3_Master::check_divide_data(){
  TIME_DEBUG_OUT << "Master has get all block infomation" << endl;
  int i;
  for (i=m3_start_slave_process_rank;i<m_free_process;i++)
    TIME_DEBUG_OUT << "Process " 
		   << i
		   << " has "
		   << m_process_train_subset_num[i]
		   << " block(s) " 
		   << endl;
}

void M3::M3_Master::multilabel_make_train_info(){
  int i,j;
  m_train_task_info.clear();
  for (i=m3_start_slave_process_rank;i<m_free_process;i++)
      for (j=m3_start_slave_process_rank;j<m_free_process;j++)
      if (m_process_to_label[i]!=m_process_to_label[j]){

	//no retrain on the pre data
          /*
	if (m3_parameter->m3_increase_learning
	    && (i-m3_start_slave_process_rank<m_il_process_num)
	    && (j-m3_start_slave_process_rank<m_il_process_num))
	  continue;
        */

	// Make a new task struct that handle train task information.
    // the 3rd and 4th parameter is denote the divide infor be i versus j
	Train_Task_Info tsi(i,
			    j,
			    m_process_train_subset_multinum[i][m_label_to_index[m_process_to_label[i]]],
			    m_process_train_subset_multinum[j][m_label_to_index[m_process_to_label[i]]],
			    m3_continue_subset_size);
	m_train_task_info.push_back(tsi);

	// debug
	TIME_DEBUG_OUT << "The task pair " << m_train_task_info.size()
		       << " is LABEL " << m_process_to_label[i]
		       << " VS " << m_process_to_label[j]
		       << " in " << i << " & " << j << endl;
	TIME_DEBUG_OUT << " and the subset num pair is "
		       << tsi.subset_num_1
		       << " VS " 
		       << tsi.subset_num_2 << endl;
	TIME_DEBUG_OUT << " and the task_num pair is "
		       << tsi.task_num_1
		       << " VS " 
		       << tsi.task_num_2 << endl;

      }
}

void M3::M3_Master::make_train_info(){
  int i,j;
  m_train_task_info.clear();
  for (i=m3_start_slave_process_rank;i<m_free_process;i++)
    for (j=m3_start_slave_process_rank;j<m_free_process;j++)
      if (m_process_to_label[i]<m_process_to_label[j]){

	//no retrain on the pre data
	if (m3_parameter->m3_increase_learning
	    && (i-m3_start_slave_process_rank<m_il_process_num)
	    && (j-m3_start_slave_process_rank<m_il_process_num))
	  continue;

	// Make a new task struct that handle train task information.
	Train_Task_Info tsi(i,
			    j,
			    m_process_train_subset_num[i],
			    m_process_train_subset_num[j],
			    m3_continue_subset_size);
	m_train_task_info.push_back(tsi);

	// debug
	TIME_DEBUG_OUT << "The task pair " << m_train_task_info.size()
		       << " is LABEL " << m_process_to_label[i]
		       << " VS " << m_process_to_label[j]
		       << " in " << i << " & " << j << endl;
	TIME_DEBUG_OUT << " and the subset num pair is "
		       << tsi.subset_num_1
		       << " VS " 
		       << tsi.subset_num_2 << endl;
	TIME_DEBUG_OUT << " and the task_num pair is "
		       << tsi.task_num_1
		       << " VS " 
		       << tsi.task_num_2 << endl;

      }
}

void M3::M3_Master::multilabel_sent_all_label(){
    // Send the all-label info to the slave
  if (m3_parameter->m3_multilabel){
      int num_label=m_index_to_label.size();
      float * mlabel=new float[num_label];
      for (int i=0;i<m_index_to_label.size();i++)
          mlabel[i]=m_index_to_label[i];
      for (int i=m3_start_slave_process_rank;i<m_free_process;i++){
          MPI_Send(&num_label,
               1,
               MPI_INT,
               i,
               M3_TAG,
               MPI_COMM_WORLD);
          MPI_Send(mlabel,
              num_label,
              MPI_FLOAT,
              i,
              M3_TAG,
              MPI_COMM_WORLD);
      }
      delete [] mlabel;
  }
}


void M3::M3_Master::multilabel_get_deivide_info(){
    if (!m3_parameter->m3_multilabel)
        return;
    int len=m_index_to_label.size();
    MPI_Status mpi_status;
    int * mdivide=new int[len];
    for (int i=m3_start_slave_process_rank;i<m_free_process;i++){
        vector<int> subset_num;
        subset_num.clear();
        MPI_Recv(mdivide,
            len,
            MPI_INT,
            i,
            M3_TAG,
            MPI_COMM_WORLD,
            &mpi_status);

        for (int j=0;j<len;j++){
            subset_num.push_back(mdivide[j]);
        }

        // debug
       TIME_DEBUG_OUT << " the slave process " << i << " with label " << m_process_to_label[i] << " multilabel divide info: ";
       for (int j=0;j<len;j++)
           debug_out << subset_num[j] << " ";
       debug_out << endl;
     
       m_process_train_subset_multinum[i]=subset_num;

    }
    delete [] mdivide;
}

void M3::M3_Master::divide_train_data(){

    multilabel_sent_all_label();  

    int i;
    int block_info[2];
    MPI_Status mpi_status;
    for (i=m3_start_slave_process_rank;i<m_free_process;i++){
        // Get divide information from slave.
        MPI_Recv(&block_info,
            2,
            MPI_INT,
            i,
            M3_TAG,
            MPI_COMM_WORLD,
            &mpi_status);
        m_process_train_subset_num[i]=block_info[1];
    }

    // debug
    //  check_divide_data();

    multilabel_get_deivide_info();

    if (m3_parameter->m3_multilabel)
        multilabel_make_train_info();
    else make_train_info();
}

void M3::M3_Master::handle_subset_info_write(ofstream & subset_config,
					     Subset_Info subset_info){
      // Save subset_config
      // Format: filename_index label_1 label_2 subset_num_1 subset_num_2 memory(Byte)
      // Format: (last line) process_1 process_2 start_1 start_2 end_1 end_2
     subset_config << subset_info.save_index
		    << " "
		    << subset_info.label_1
		    << " " 
		    << subset_info.label_2
		    << " " 
		    << subset_info.subset_num_1
		    << " "
		    << subset_info.subset_num_2
		    << " "
		    << subset_info.subset_memory
		    << " "
		    << subset_info.process_1
		    << " "
		    << subset_info.process_2
		    << " "
		    << subset_info.start_1
		    << " "
		    << subset_info.start_2
		    << " "
		    << subset_info.end_1
		    << " "
		    << subset_info.end_2
		    << endl;
}

int M3::M3_Master::handle_subset_info_read(ifstream & config_in,
					   Subset_Info & ts){
  return (bool)(config_in >> ts.save_index 
		>> ts.label_1 >> ts.label_2
		>> ts.subset_num_1 >> ts.subset_num_2
		>> ts.subset_memory
		>> ts.process_1 >> ts.process_2
		>> ts.start_1 >> ts.start_2
		>> ts.end_1 >> ts.end_2);
}

void M3::M3_Master::training_train_data(){

  int index=0;

  ofstream subset_config;
  if (m3_parameter->m3_increase_learning)
    subset_config.open(SUBSET_CONFIG.c_str(),ios::app);
  else 
    subset_config.open(SUBSET_CONFIG.c_str());
  Subset_Info subset_info;

  while (1){
    // Task pair label changed.
    if (m_train_task_info[index].task_over())
      index++;

    // Task over.
    if (index>=m_train_task_info.size())
      break;

    int free_process_rank;
    MPI_Status mpi_status;

    // debug
    TIME_DEBUG_OUT << "Master recv free process" << endl;

    // Get the free process rank.
    // !!!!WARNING: M3_SUBSET_INFO_TAG=9999 is must be only used here.
    MPI_Recv(&subset_info,
	     1,
	     MPI_Subset_Info,
	     MPI_ANY_SOURCE,
	     M3_SUBSET_INFO_TAG,
	     MPI_COMM_WORLD,
	     &mpi_status);
    free_process_rank=subset_info.process_rank;

    if (subset_info.save_index>=0){
      handle_subset_info_write(subset_config,subset_info); 
    }


    // debug
    TIME_DEBUG_OUT << "Master find the process " 
		   << free_process_rank 
		   << " is free" 
		   << endl;

    Train_Task_Info tsi=m_train_task_info[index];

    // debug
    TIME_DEBUG_OUT << "Master send CTRL" << endl;

    // debug
    TIME_DEBUG_OUT << "Master send data_process infomation:" << endl;
    TIME_DEBUG_OUT << "process " << tsi.process_rank_1
		   << " must sent from subset " << tsi.left_1() 
		   << " to " << tsi.right_1() 
		   << " to process: " << free_process_rank << endl;
    TIME_DEBUG_OUT << "process " << tsi.process_rank_2
		   << " must sent from subset " << tsi.left_2() 
		   << " to " << tsi.right_2() 
		   << " to process: " << free_process_rank << endl;

    // Send to the relate data_slave and train_slave there is task.
    MPI_Send(&CTRL_TRAIN_CONTINUE,
	     1,
	     MPI_INT,
	     free_process_rank,
	     M3_TAG,
	     MPI_COMM_WORLD);
    MPI_Send(&CTRL_TRAIN_CONTINUE,
	     1,
	     MPI_INT,
	     tsi.process_rank_1,
	     M3_TAG,
	     MPI_COMM_WORLD);
    MPI_Send(&CTRL_TRAIN_CONTINUE,
	     1,
	     MPI_INT,
	     tsi.process_rank_2,
	     M3_TAG,
	     MPI_COMM_WORLD);

    // debug
    TIME_DEBUG_OUT << "master CTRL send over " << endl;
    TIME_DEBUG_OUT << "master send relate process information" << endl;

    // Send to the data_slave which data(or subset) it will be sent.
    // And which train_slave will need them.
    // i vs j then send the jth divide's info of label i and vice versa
    int data_process_1[4],data_process_2[4];
    data_process_1[0]=tsi.left_1();
    data_process_1[1]=tsi.right_1();
    data_process_1[2]=free_process_rank;
    data_process_1[3]=m_label_to_index[m_process_to_label[tsi.process_rank_2]];
    data_process_2[0]=tsi.left_2();
    data_process_2[1]=tsi.right_2();
    data_process_2[2]=free_process_rank;
    data_process_2[3]=m_label_to_index[m_process_to_label[tsi.process_rank_1]];


    // if multilabel the rank1 is the major label(index i) that divide's info is saved in the place of index i
    if (m3_parameter->m3_multilabel){
        data_process_1[3]=m_label_to_index[m_process_to_label[tsi.process_rank_1]];
    }

    MPI_Send(data_process_1,
	     4,
	     MPI_INT,
	     tsi.process_rank_1,
	     M3_TAG,
	     MPI_COMM_WORLD);
    MPI_Send(data_process_2,
	     4,
	     MPI_INT,
	     tsi.process_rank_2,
	     M3_TAG,
	     MPI_COMM_WORLD);

    // debug
    TIME_DEBUG_OUT << "master send infomation to free process " << free_process_rank << endl;

    // Send to the train_slave which data_slave it will get data from.
    // And how many data.
    Subset_Info si;
    si.label_1=m_process_to_label[tsi.process_rank_1];
    si.label_2=m_process_to_label[tsi.process_rank_2];
    si.subset_num_1=tsi.right_1()-tsi.left_1()+1;
    si.subset_num_2=tsi.right_2()-tsi.left_2()+1;
    si.save_index=subset_config_index++;
    si.process_rank=free_process_rank;
    si.process_1=tsi.process_rank_1;
    si.process_2=tsi.process_rank_2;
    si.start_1=tsi.left_1();
    si.start_2=tsi.left_2();
    si.end_1=tsi.right_1();
    si.end_2=tsi.right_2();
    MPI_Send(&si,
	     1,
	     MPI_Subset_Info,
	     free_process_rank,
	     M3_TAG,
	     MPI_COMM_WORLD);

    // New task
    m_train_task_info[index].free_task++;
  }

  // debug
  TIME_DEBUG_OUT << "Master train done " << endl;

  // Wait for all train_slave free.
  // ****************Mark:选择任意接收，填hash表。
  for (int i=M3_MASTER_RANK+1;i<m3_start_slave_process_rank;i++){
    MPI_Status mpi_status;
    MPI_Recv(&subset_info,
	     1,
	     MPI_Subset_Info,
	     i,
	     M3_SUBSET_INFO_TAG,
	     MPI_COMM_WORLD,
	     &mpi_status);

    if (subset_info.save_index>=0){
      handle_subset_info_write(subset_config,subset_info); 
    }
  }

  // Close the subset config file
  subset_config.close();

  // Send to all slave that there is no task.
  for (int i=M3_MASTER_RANK+1;i<m3_all_process_num;i++)
    MPI_Send(&CTRL_TRAIN_DONE,
	     1,
	     MPI_INT,
	     i,
	     M3_TAG,MPI_COMM_WORLD);

  save_for_increase_learning();

  // debug
  TIME_DEBUG_OUT << "Master train over " << endl;

}

void M3::M3_Master::save_for_increase_learning(){
 if (m3_parameter->m3_save_for_increase_learning){
    // debug
    TIME_DEBUG_OUT << "Master to save for increase learning" << endl;
    system("cp subset.config subset.config_il");

    ofstream ilconfig(DEFAULT_IL_CONFIG.c_str());
    ilconfig << DEFAULT_IL_SUBSET_CONFIG << endl;
    ilconfig << m_process_to_label.size() << endl;
    map<int,int>::iterator it;
    for (it=m_process_train_subset_num.begin();it!=m_process_train_subset_num.end();it++){
      int pid=(*it).first;
      ilconfig << pid << " " 
	       << m_process_to_label[pid] << " "
	       << m_process_train_data_num[pid] << " "
	       << (*it).second << " " << endl;
    }

    ilconfig.close();
  }
}

// For easy to send the test command.
void M3::M3_Master::test_ctrl(int ctrl){
  int i;
  for (i=1;i<m3_start_slave_process_rank;i++)
    MPI_Send(&ctrl,
	     1,
	     MPI_INT,
	     i,
	     M3_TAG,
	     MPI_COMM_WORLD);
}

// Master load subset config for test
void M3::M3_Master::load_subset_config(){
  m_test_subset.clear();
  ifstream config_in(SUBSET_CONFIG.c_str());
  Subset_Info ts;
  while (handle_subset_info_read(config_in,ts))
    m_test_subset.push_back(ts);
  config_in.close();

  ifstream liconfig_in(LABEL_INDEX_CONFIG.c_str());
  float l;
  int i=0;
  m_index_to_label.clear();
  m_label_to_index.clear();
  while (liconfig_in >> l){
    m_index_to_label.push_back(l);
    m_label_to_index[l]=i++;
  }
  liconfig_in.close();
}

bool M3::M3_Master::ask_load_subset(int index){
  MPI_Status mpi_status;
  Subset_Info si;
  int free_process_rank;
  int memory_res=CTRL_MEMORY_ENOUGH;
  while (true){

    if (m_test_process_num<=0)
      break;

    MPI_Recv(&si,
	     1,
	     MPI_Subset_Info,
	     MPI_ANY_SOURCE,
	     M3_SUBSET_INFO_TAG,
	     MPI_COMM_WORLD,
	     &mpi_status);
    free_process_rank=si.process_rank;

    // debug
    TIME_DEBUG_OUT << "master find process " << free_process_rank
		   << " is free to load subset " << index << endl;

    si=m_test_subset[index];
    si.process_rank=free_process_rank;
    MPI_Send(&si,
	     1,
	     MPI_Subset_Info,
	     free_process_rank,
	     M3_TAG,
	     MPI_COMM_WORLD);

    // debug
    TIME_DEBUG_OUT << "master has sent subset information to process " << free_process_rank << endl;

    MPI_Recv(&memory_res,
	     1,
	     MPI_INT,
	     free_process_rank,
	     M3_TAG,
	     MPI_COMM_WORLD,
	     &mpi_status);

    // debug
    TIME_DEBUG_OUT << "master has get memory information from process " << free_process_rank << endl;

    if (memory_res==CTRL_MEMORY_ENOUGH){

      // debug
      TIME_DEBUG_OUT << "master find process " << free_process_rank 
		     << "'s memory is enough" << endl;

      m_test_subset[index].process_rank=free_process_rank;
      return true;
    }
    else {

      // debug
      TIME_DEBUG_OUT << "master find process " << free_process_rank 
		     << "'s memory is not enough" << endl;

      m_test_process_num--;
    }

  }
  return false;
}

void M3::M3_Master::classify_test_data_serial(string file_name,
					      vector<bool> test_flag){

  char * read_buf=new char[READ_BUF_SIZE];

  int subset_index=0;
  int index=0;

  Data_Sample * sample_buf=new Data_Sample[TEST_SAMPLE_BUF_SIZE];
  Data_Node * node_buf=new Data_Node[TEST_NODE_BUF_SIZE];
  int sp_index,nd_index;
  int test_tms=0;

  while (true){
    if (subset_index>=m_test_subset.size()){

      // debug
      TIME_DEBUG_OUT << "master test over" << endl;

      break;
    }

    // debug
    // runing flag
    cout << "At the " << ++test_tms << " times test(load subset & classify) " << endl;
    
    m_test_process_num=min(m3_start_slave_process_rank,
			   m3_all_process_num)-1;

    // debug
    TIME_DEBUG_OUT << "master let slave load subset " << endl;
    
    test_ctrl(CTRL_LOAD_SUBSET);
    while (subset_index<m_test_subset.size() && ask_load_subset(subset_index))
      subset_index++;

    for (int i=0;i<m_test_process_num;i++){
      // Wait for all test process free
      Subset_Info si;
      MPI_Status mpi_status;
      MPI_Recv(&si,
	       1,
	       MPI_Subset_Info,
	       MPI_ANY_SOURCE,
	       M3_SUBSET_INFO_TAG,
	       MPI_COMM_WORLD,
	       &mpi_status);
      si.save_index=-1;
      MPI_Send(&si,
	       1,
	       MPI_Subset_Info,
	       mpi_status.MPI_SOURCE,
	       M3_TAG,
	       MPI_COMM_WORLD);
    }

    // debug
    TIME_DEBUG_OUT << "now begin to load test & classify" << endl;

    ifstream file_in(file_name.c_str());
    index=0;
    while (true){

      // debug
      TIME_DEBUG_OUT << "master begin to load test data" << endl;

      sp_index=0;
      nd_index=0;

      while (sp_index<TEST_SAMPLE_BUF_SIZE){
      
	memset(read_buf,0,sizeof(char)*READ_BUF_SIZE);
	if (!file_in.getline(read_buf,
			     READ_BUF_SIZE)){
	  // debug
	  TIME_DEBUG_OUT << "test file is over!" << endl;

	  file_in.close();
	  break;
	}

	if (index>=test_flag.size() || (!test_flag[index++]))
	  continue;

	// debug
	//	TIME_DEBUG_OUT << "begint parse data: " << read_buf << endl;

	sample_buf[sp_index].data_vector=&node_buf[nd_index];
	parse_data(read_buf,
		   &sample_buf[sp_index]);
	sample_buf[sp_index].index=index-1;

	nd_index+=sample_buf[sp_index].data_vector_length;
	sp_index++;
      }

      if (sp_index<=0) 
	break;

      test_ctrl(CTRL_CLASSIFY_DATA);

      // debug
      TIME_DEBUG_OUT << "master bcast data to all slave to classify" << endl;

      //       // debug
      //       for (int j=0;j<sp_index;j++){
      // 	TIME_DEBUG_OUT << "master bcast information:" 
      // 		       << sample_buf[j].index << " "
      // 		       << sample_buf[j].label << " "
      // 		       << sample_buf[j].data_vector_length << endl;
      //  	TIME_DEBUG_OUT;
      //  	for (int i=0;i<sample_buf[j].data_vector_length;i++)
      //  	  debug_out << "(" << sample_buf[j].data_vector[i].index
      //  		    << "," << sample_buf[j].data_vector[i].value << ")";
      //  	debug_out << endl;
      //       }


      // debug
      // runnig flag
      cout << sample_buf[0].index << " ---- " 
	   << sample_buf[sp_index-1].index << " of " 
	   << test_flag.size() << " : " 
	   << " running at " 
	   << (100.0*(sample_buf[sp_index-1].index+1))/(1.0*test_flag.size())
	   << "%" << endl;

      for (int i=1;i<m3_start_slave_process_rank;i++){
	MPI_Send(&sp_index,
		 1,
		 MPI_INT,
		 i,
		 M3_TAG,
		 MPI_COMM_WORLD);
	MPI_Send(sample_buf,
		 sp_index,
		 MPI_Data_Sample,
		 i,
		 M3_TAG,
		 MPI_COMM_WORLD);
	MPI_Send(node_buf,
		 nd_index,
		 MPI_Data_Node,
		 i,
		 M3_TAG,
		 MPI_COMM_WORLD);
      }

      // debug
      TIME_DEBUG_OUT << "master bcast data finished" << endl;

    }

    // debug
    TIME_DEBUG_OUT << "master has over one loop test " << endl;

    // debug
    TIME_DEBUG_OUT << "master send all slave clear memory" << endl;

    test_ctrl(CTRL_TEST_CLEAR);
  }

  test_ctrl(CTRL_TEST_DONE);  

  delete [] read_buf;
  delete [] node_buf;
  delete [] sample_buf;

  // debug
  TIME_DEBUG_OUT << "master test done" << endl;

}

void M3::M3_Master::classify_test_data_parallel(){

  int subset_index=0;
  int index=0;

  int sp_index,nd_index;
  int test_tms=0;

  while (true){
    if (subset_index>=m_test_subset.size()){

      // debug
      TIME_DEBUG_OUT << "master test over" << endl;

      break;
    }

    // debug
    // runing flag
    cout << "At the " << ++test_tms << " times test(load subset & classify) " << endl;
    
    m_test_process_num=min(m3_start_slave_process_rank,
			   m3_all_process_num)-1;

    // debug
    TIME_DEBUG_OUT << "master let slave load subset " << endl;
    
    test_ctrl(CTRL_LOAD_SUBSET);
    while (subset_index<m_test_subset.size() && ask_load_subset(subset_index))
      subset_index++;

    for (int i=0;i<m_test_process_num;i++){
      // Wait for all test process free
      Subset_Info si;
      MPI_Status mpi_status;
      MPI_Recv(&si,
	       1,
	       MPI_Subset_Info,
	       MPI_ANY_SOURCE,
	       M3_SUBSET_INFO_TAG,
	       MPI_COMM_WORLD,
	       &mpi_status);
      si.save_index=-1;
      MPI_Send(&si,
	       1,
	       MPI_Subset_Info,
	       mpi_status.MPI_SOURCE,
	       M3_TAG,
	       MPI_COMM_WORLD);
    }

    test_ctrl(CTRL_CLASSIFY_DATA);

    // debug
    TIME_DEBUG_OUT << "master has over one loop test " << endl;

    // debug
    TIME_DEBUG_OUT << "master send all slave clear memory" << endl;

    test_ctrl(CTRL_TEST_CLEAR);
  }

  test_ctrl(CTRL_TEST_DONE);  

  // debug
  TIME_DEBUG_OUT << "master test done" << endl;

}

void M3::M3_Master::classify_test_data_nonpruning(const string &filename){

  load_subset_config();

  size_t len = M3::testLen;

  if (m3_parameter->m3_classify_mode==0){
    classify_test_data_parallel();
  }
  else if (m3_parameter->m3_classify_mode==1){
    string file_name=filename;
    vector<bool> test_flag;
    test_flag.clear();
    for (int i=0;i<len;i++)
      test_flag.push_back(true);
    classify_test_data_serial(file_name,
			      test_flag);
  }
}

bool cmp_pruning(const Subset_Info & si_1,
		 const Subset_Info & si_2){ // collum first
  if (si_1.label_1<si_2.label_1)
    return true;
  else if (si_1.label_1>si_2.label_1)
    return false;
  
  if(si_1.label_2<si_2.label_2)
    return true;
  else if (si_1.label_2>si_2.label_2)
    return false;
  
  if (si_1.process_1<si_2.process_1)
    return true;
  else if (si_1.process_1>si_2.process_1)
    return false;
  
  if (si_1.process_2<si_2.process_2)
    return true;
  else if (si_1.process_2>si_2.process_2)
    return false;
  
  if (si_1.start_1<si_2.start_1)
    return true;
  else if (si_1.start_1>si_2.start_1)
    return false;
  
  if (si_1.start_2<si_2.start_2)
    return true;
  else if (si_1.start_2>si_2.start_2)
    return false;
  
  return false;
}

void M3::M3_Master::make_pipe_info_pruning(){

  // debug
  TIME_DEBUG_OUT << "Master make pipe infomation" << endl;
  
  int now_id=-1;
  
  int first_pr=-1;
  int first_so=-1;
  for (int i=0;i<m_test_subset.size();i++){
    Subset_Info si=m_test_subset[i];
    if (now_id==-1 || !m_pipe_info[now_id].in_pipe(si)){
      if (now_id!=-1){
	m_pipe_info[now_id].end_offset=i-1;
	if (m3_parameter->m3_pruning_mode==1) // select pruning mode
	  m_pipe_info[now_id].info_complete_sematric_pruning();
	else if (m3_parameter->m3_pruning_mode==2)
	  m_pipe_info[now_id].info_complete_asematric_pruning();

      }
      
      now_id++;
      Pipe_Info pi;
      m_pipe_info.push_back(pi);
      first_pr=si.process_1;
      first_so=si.start_1;
      m_pipe_info[now_id].start_offset=i;
      m_pipe_info[now_id].label_1=si.label_1;
      m_pipe_info[now_id].label_2=si.label_2;
    }
    if (si.process_1==first_pr && si.start_1==first_so)
      m_pipe_info[now_id].subset_num_2++;
    m_pipe_info[now_id].total_subset_pair++;
  }
  m_pipe_info[now_id].end_offset=m_test_subset.size();
  if (m3_parameter->m3_pruning_mode==1) // select pruning mode
    m_pipe_info[now_id].info_complete_sematric_pruning();
  else if (m3_parameter->m3_pruning_mode==2)
    m_pipe_info[now_id].info_complete_asematric_pruning();
  

  if (m3_parameter->m3_pruning_mode==1){ // select pruning mode
    now_id=0;
    for (int i=0;i<m_test_subset.size();i++){
      if (m_pipe_info[now_id].end_offset<i)
	now_id++;
      int sto=m_pipe_info[now_id].start_offset;
      m_pipe_info[now_id].
	level_index[m_pipe_info[now_id].level_sematric_pruning(i-sto)].push_back(i);
    }
  }
  else if (m3_parameter->m3_pruning_mode==2){
    now_id=0;
    for (int i=0;i<m_test_subset.size();i++){
      if (m_pipe_info[now_id].end_offset<i)
	now_id++;
      int sto=m_pipe_info[now_id].start_offset;
      m_pipe_info[now_id].
	level_index[m_pipe_info[now_id].level_asematric_pruning(i-sto)].push_back(i);
    }
  }

  for (int i=0;i<m_pipe_info.size();i++)
    m_pipe_info[i].process_distribute(m3_parameter->running_process_num);

  // debug
  for (int i=0;i<m_pipe_info.size();i++){
    Pipe_Info pi=m_pipe_info[i];
    TIME_DEBUG_OUT << "Pipe " << i << " is " << pi.label_1 << " vs " << pi.label_2 << endl;
    TIME_DEBUG_OUT << "has all " << pi.subset_num_1 << " * " << pi.subset_num_2 
		   << "=" << pi.total_subset_pair << " subsets "
		   << " & " << pi.total_level << " level(s)" << endl;
    for (int j=0;j<pi.level_index.size();j++){
      TIME_DEBUG_OUT << "  Level " << j << " in process " << pi.level_in_process[j] << " : ";
      for (int k=0;k<pi.level_index[j].size();k++)
	debug_out << pi.level_index[j][k] << " ";
      debug_out << endl;
    }
  }

}


void M3::M3_Master::multilabel_make_pipe_info_pruning(){

    // debug
    TIME_DEBUG_OUT << "Master make pipe infomation" << endl;

    int now_id=-1;

    int first_pr=-1;
    int first_so=-1;
    int second_pr=-1;
    for (int i=0;i<m_test_subset.size();i++){
        Subset_Info si=m_test_subset[i];
        if (now_id==-1 || !m_pipe_info[now_id].in_multilabel_pipe(si)){
            if (now_id!=-1){
                m_pipe_info[now_id].end_offset=i-1;
                if (m3_parameter->m3_pruning_mode==1) // select pruning mode
                    ;
                else if (m3_parameter->m3_pruning_mode==2)
                    m_pipe_info[now_id].info_complete_asematric_pruning();
            }

            now_id++;
            Pipe_Info pi;
            m_pipe_info.push_back(pi);
            first_pr=si.process_1;
            first_so=si.start_1;
            m_pipe_info[now_id].start_offset=i;
            m_pipe_info[now_id].label_1=si.label_1;
            m_pipe_info[now_id].label_2=si.label_2;
        }
        if (si.process_1==first_pr && si.start_1==first_so)
            m_pipe_info[now_id].subset_num_2++;
        m_pipe_info[now_id].total_subset_pair++;
        if (si.process_2==second_pr){
            m_pipe_info[now_id].put_multi_versus(1);
        }
        else {
            m_pipe_info[now_id].put_multi_versus(0);
            second_pr=si.process_2;
        }
    }
    m_pipe_info[now_id].end_offset=m_test_subset.size();
    if (m3_parameter->m3_pruning_mode==1) // select pruning mode
        ;
    else if (m3_parameter->m3_pruning_mode==2)
        m_pipe_info[now_id].info_complete_asematric_pruning();

    if (m3_parameter->m3_pruning_mode==1){ // select pruning mode
        /*
        now_id=0;
        for (int i=0;i<m_test_subset.size();i++){
            if (m_pipe_info[now_id].end_offset<i)
                now_id++;
            int sto=m_pipe_info[now_id].start_offset;
            m_pipe_info[now_id].
                level_index[m_pipe_info[now_id].level_sematric_pruning(i-sto)].push_back(i);
        }
        */
    }
    else if (m3_parameter->m3_pruning_mode==2){
        now_id=0;
        for (int i=0;i<m_test_subset.size();i++){
            if (m_pipe_info[now_id].end_offset<i)
                now_id++;
            int sto=m_pipe_info[now_id].start_offset;
            m_pipe_info[now_id].
                level_index[m_pipe_info[now_id].multilabel_level_asematric_pruning(i-sto)].push_back(i);
        }
    }

    for (int i=0;i<m_pipe_info.size();i++)
        m_pipe_info[i].process_distribute(m3_parameter->running_process_num);

    // debug
    for (int i=0;i<m_pipe_info.size();i++){
        Pipe_Info pi=m_pipe_info[i];
        TIME_DEBUG_OUT << "Pipe " << i << " is " << pi.label_1 << " vs " << pi.label_2 << endl;
        TIME_DEBUG_OUT << "has all " << pi.subset_num_1 << " * " << pi.subset_num_2 
            << "=" << pi.total_subset_pair << " subsets "
            << " & " << pi.total_level << " level(s)" << endl;
        for (int j=0;j<pi.level_index.size();j++){
            TIME_DEBUG_OUT << "  Level " << j << " in process " << pi.level_in_process[j] << " : ";
            for (int k=0;k<pi.level_index[j].size();k++)
                debug_out << pi.level_index[j][k] << " ";
            debug_out << endl;
        }
    }

}


void M3::M3_Master::send_pipe_info_pruning(Pipe_Info & pi){

  // debug
  TIME_DEBUG_OUT << "Master send pipe_info " << pi.label_1 << " vs " << pi.label_2 << endl;

  int x[PRUNING_LEVEL_LENGTH],y[PRUNING_LEVEL_LENGTH];
  Subset_Info subset_info[PRUNING_LEVEL_LENGTH];
  int quest[2];
  int send_p;
  int pre_post[2];
  for (int i=0;i<pi.total_level;i++){
    quest[0]=i;			       // level
    quest[1]=pi.level_index[i].size(); // length
    for (int j=0;j<quest[1];j++){
      x[j]=pi.x(pi.level_index[i][j]-pi.start_offset);
      y[j]=pi.y(pi.level_index[i][j]-pi.start_offset);
      subset_info[j]=m_test_subset[pi.level_index[i][j]];
    }
    send_p=pi.level_in_process[i];
    pre_post[0]=pi.pre_process(i);
    pre_post[1]=pi.post_process(i);

    m_pipe_process_use[send_p-1]=true;
    
    // debug
    TIME_DEBUG_OUT << "master send the level " << i 
		   << " to process " << send_p 
		   << " with pre: " << pre_post[0] 
		   << " & post: " << pre_post[1] 
		   << endl;
    
    MPI_Send(quest,
	     2,
	     MPI_INT,
	     send_p,
	     M3_TAG,
	     MPI_COMM_WORLD);

    // debug
    BEGIN_DEBUG;
    TIME_DEBUG_OUT << "quest done" << endl;
    END_DEBUG;

    MPI_Send(pre_post,
	     2,
	     MPI_INT,
	     send_p,
	     M3_TAG,
	     MPI_COMM_WORLD);

    // debug
    BEGIN_DEBUG;
    TIME_DEBUG_OUT << "pre_post done" << endl;
    END_DEBUG;

    MPI_Send(x,
	     quest[1],
	     MPI_INT,
	     send_p,
	     M3_TAG,
	     MPI_COMM_WORLD);

    // debug
    BEGIN_DEBUG;
    TIME_DEBUG_OUT << "x done" << endl;
    END_DEBUG;

    MPI_Send(y,
	     quest[1],
	     MPI_INT,
	     send_p,
	     M3_TAG,
	     MPI_COMM_WORLD);

    // debug
    BEGIN_DEBUG;
    TIME_DEBUG_OUT << "y done" << endl;
    END_DEBUG;

    MPI_Send(subset_info,
	     quest[1],
	     MPI_Subset_Info,
	     send_p,
	     M3_TAG,
	     MPI_COMM_WORLD);

    // debug
    BEGIN_DEBUG;
    TIME_DEBUG_OUT << "subset_info done" << endl;
    END_DEBUG;

  }


  for (int i=1;i<=m3_parameter->running_process_num;i++)
    if (pi.use_process(i)){
      quest[0]=quest[1]=-1;	// no level to be send
      if (i==pi.last_process()){
	quest[1]=1;		// save mid score

	// debug
	TIME_DEBUG_OUT << "run_process " << i << " must save the mid score of this pipe " << endl;

      }
      MPI_Send(quest,
	       2,
	       MPI_INT,
	       i,
	       M3_TAG,
	       MPI_COMM_WORLD);
    }
  
  
  // debug
  TIME_DEBUG_OUT << "master send this pipe done " << endl;

}

void M3::M3_Master::handle_pipe_score(vector<bool> & test_flag){

  MPI_Status mpi_status;

  ofstream result(RESULT.c_str());
  ofstream result_matrix(RESULT_MATRIX.c_str());
  ofstream result_confident(RESULT_CONFIDENT.c_str());
  double confi;

  int label_len=m_index_to_label.size();

  Voter * voter=M3_Factory::create_voter(m3_parameter->voter_rank);//new HC_Voter;

  double *** score_matrix=new double ** [TEST_AND_SCORE_ALL_DATA_BUF_SIZE];
  for (int i=0;i<TEST_AND_SCORE_ALL_DATA_BUF_SIZE;i++){
    score_matrix[i]=new double * [label_len];
    for (int j=0;j<label_len;j++)
      score_matrix[i][j]=new double [label_len];
  }

  float * result_buf=new float[TEST_AND_SCORE_ALL_DATA_BUF_SIZE];

  int index=0;
  int tms=0;
  int len=0;

  // debug
  TIME_DEBUG_OUT << "master begin to handle the mid score " << endl;

  while (true){

    for (len=0;index<test_flag.size();index++){
      len+=test_flag[index];
      if (len>=TEST_AND_SCORE_ALL_DATA_BUF_SIZE){
	index++;
	break;
      }
    }

    if (len==0){

      // debug
      TIME_DEBUG_OUT << "master score & test over" << endl;

      break;
    }

    for (int i=1;i<=m3_parameter->running_process_num;i++){
      MPI_Send(&CTRL_TS_CONTINUE,
	       1,
	       MPI_INT,
	       i,
	       M3_TAG,
	       MPI_COMM_WORLD);
    }

    int get_num=0;
    int max_num=label_len*(label_len-1)/2;

    while (true){
      if (get_num==max_num)
	break;

      int proc;
      float label[2];

      MPI_Recv(&proc,
	       1,
	       MPI_INT,
	       MPI_ANY_SOURCE,
	       M3_TAG,
	       MPI_COMM_WORLD,
	       &mpi_status);

      // debug
      TIME_DEBUG_OUT << "get the buf from process " << proc << endl;

      MPI_Recv(label,
	       2,
	       MPI_FLOAT,
	       proc,
	       M3_TAG,
	       MPI_COMM_WORLD,
	       &mpi_status);

      // debug
      TIME_DEBUG_OUT << "get the label pair: " 
		     << label[0] << " vs " 
		     << label[1] << endl;

      MPI_Recv(result_buf,
	       len,
	       MPI_FLOAT,
	       proc,
	       M3_TAG,
	       MPI_COMM_WORLD,
	       &mpi_status);

      int l1=m_label_to_index[label[0]];
      int l2=m_label_to_index[label[1]];

      if (l1>l2)
	swap(l1,l2);

      for (int i=0;i<len;i++)
	score_matrix[i][l1][l2]=result_buf[i];

      get_num++;
    }

    for (int i=0;i<len;i++){    
      tms++;
      voter->matrix_inverse(m_index_to_label,
			    score_matrix[i]);

      result_matrix << tms << ": " << endl;
      for (int ii=0;ii<label_len;ii++){
	for (int jj=0;jj<label_len;jj++)
	  result_matrix << score_matrix[i][ii][jj] << " ";
	result_matrix << endl;
      }
      result_matrix << endl;

      result << tms << ": " 
	     << voter->vote(m_index_to_label,
			    score_matrix[i],
			    confi) << endl;

      result_confident << tms << ": "
		       << confi << endl;
    }
  }

  result_confident.close();
  result_matrix.close();
  result.close();
  
  delete [] result_buf;
  
  for (int i=0;i<TEST_AND_SCORE_ALL_DATA_BUF_SIZE;i++){
    for (int j=0;j<label_len;j++)
      delete [] score_matrix[i][j];
    delete [] score_matrix[i];
  }
  delete [] score_matrix;

  for (int i=1;i<=m3_parameter->running_process_num;i++){
    MPI_Send(&CTRL_TS_DONE,
	     1,
	     MPI_INT,
	     i,
	     M3_TAG,
	     MPI_COMM_WORLD);
  }

}

void M3::M3_Master::classify_test_data_pruning(){

  // debug
  TIME_DEBUG_OUT << "begint to load subset " << endl;

  load_subset_config();

  // debug
  TIME_DEBUG_OUT << " load subset config over !" << endl;

  size_t len = M3::testLen;

  // debug
  TIME_DEBUG_OUT << "calculate the all line over !" << endl;

  sort(m_test_subset.begin(),
       m_test_subset.end(),
       cmp_pruning);

  // debug
  TIME_DEBUG_OUT << "master in sematric pruning has read & sort the subset:" << endl;
  for (int i=0;i<m_test_subset.size();i++){
    Subset_Info si=m_test_subset[i];
    TIME_DEBUG_OUT << "Subset " << si.save_index 
		   << " with label " << si.label_1 << " vs " << si.label_2
		   << " in process " << si.process_1 << " & " << si.process_2
		   << " start at " << si.start_1 << " & " << si.start_2
		   << " and end at " << si.end_1 << " & " << si.end_2
		   << endl;
  }

  if (!m3_parameter->m3_multilabel)
    make_pipe_info_pruning();
  else multilabel_make_pipe_info_pruning();

  for (int i=0;i<m3_parameter->running_process_num;i++)
    m_pipe_process_use.push_back(false);

  // sort the pipe by the total level for the put sth in bag use greedy
  sort(m_pipe_info.begin(),
       m_pipe_info.end());

  float quest_arr[3];
  vector<bool> finish_pipe;
  for (int i=0;i<m_pipe_info.size();i++)
    finish_pipe.push_back(false);
  int base_ps=1;

  while (1){
    int i=-1;
    bool all_done=true;

    for (i=finish_pipe.size()-1;i>=0;i--)
      if (!finish_pipe[i]){
	all_done=false;

	if (m_pipe_info[i].total_level>=
	    m3_parameter->running_process_num) // more than one level in one process
	  break;

	if (m_pipe_info[i].total_level<=
	    (m3_parameter->running_process_num-base_ps+1)){ // one level in one process must be put in bag
	  break;
	}
      }

    if (all_done)
      break;

    if (i<0 || base_ps>m3_parameter->running_process_num){
      base_ps=1;
      continue;
    }

    finish_pipe[i]=true;

    m_pipe_info[i].process_transfor(base_ps);

    base_ps+=m_pipe_info[i].total_level;		   

    quest_arr[0]=i+1;
    quest_arr[1]=m_pipe_info[i].label_1;
    quest_arr[2]=m_pipe_info[i].label_2;

    for (int j=1;j<=m3_parameter->running_process_num;j++)
      if (m_pipe_info[i].use_process(j)){
	MPI_Send(quest_arr,
		 3,
		 MPI_FLOAT,
		 j,
		 M3_TAG,
		 MPI_COMM_WORLD);
      }

    send_pipe_info_pruning(m_pipe_info[i]);   
  }

  // debug
  TIME_DEBUG_OUT << "master sent all run process task is over!" << endl;

  quest_arr[0]=-1;		// <0 means over
  for (int j=1;j<=m3_parameter->running_process_num;j++)
    MPI_Send(quest_arr,
	     3,
	     MPI_FLOAT,
	     j,
	     M3_TAG,
	     MPI_COMM_WORLD); 


  if (m3_parameter->m3_pruning_combine_score!=0){

    // debug
    TIME_DEBUG_OUT << "master do the score in test step!" << endl;

    // debug
    // the cross validation interface
    vector<bool> test_flag;
    for (int i=0;i<M3::testLen;i++)
      test_flag.push_back(true);

    handle_pipe_score(test_flag);
  }

}

void M3::M3_Master::classify_test_data(){

  M3::testLen= M3::wc_l(m3_parameter->classify_data);

  for (int i=1;i<=m3_parameter->running_process_num;i++)
    MPI_Send(&M3::testLen,
	     1,
	     MPI_INT,
	     i,
	     M3_TAG,
	     MPI_COMM_WORLD);

  if (m3_parameter->m3_pruning_mode==0){
    classify_test_data_nonpruning(m3_parameter->classify_data);
  }
  else if (m3_parameter->m3_pruning_mode==1){
    classify_test_data_pruning();
  }
  else if (m3_parameter->m3_pruning_mode==2){
    classify_test_data_pruning();
  }
}

double M3::M3_Master::get_min_score_min_vector(ifstream * fin,
					       double sm,
					       bool flag){
  double sc;
  (*fin) >> sc;
  if (flag)
    return sc;
  else return min(sm,sc);
}

double M3::M3_Master::get_min_score_full(int ll,
					 int rr,
					 ifstream * fin,
					 double sm,
					 bool flag){
  double tmp=sm;

  for (int k=ll;k<=rr;k++){
    double sc;
    (*fin) >> sc;
    if (flag)
      tmp=sc;
    else tmp=min(tmp,sc);	// MIN-MAX: MIN
    flag=false;
  }

  return tmp;
}

void M3::M3_Master::score_test_data_nonpruning(vector<bool> test_flag){

  ofstream result(RESULT.c_str());
  ofstream result_matrix(RESULT_MATRIX.c_str());

  vector<ifstream*> score_file;
  vector<size_t> score_seek;
  score_file.clear();
  for (int i=0;i<m_test_subset.size();i++){
    ifstream * file=new ifstream;
    char tmp[10];
    score_file.push_back(file);
    score_seek.push_back(0);
  }

  int label_len=m_index_to_label.size();

  double ** score_matrix=new double*[label_len];
  for (int i=0;i<label_len;i++)
    score_matrix[i]=new double[label_len];
  map<Block_Index,double> ** score_vector=new map<Block_Index,double>* [label_len];
  for (int i=0;i<label_len;i++)
    score_vector[i]=new map<Block_Index,double>[label_len];
  map<Block_Index,double>::iterator it;

  for (int tms=0;tms<test_flag.size();tms++)
    if (test_flag[tms]){

      if (tms==test_flag.size() || !((tms+1)%100))
	cout << "score " << tms 
	     << " @ " << 100.0*(tms+1)/test_flag.size() << "%" << endl;
      
      for (int i=0;i<label_len;i++)
	for (int j=0;j<label_len;j++)
	  score_vector[i][j].clear();

      for (int i=0;i<m_test_subset.size();i++){
	ifstream * fin=score_file[i];
	int test_data_index;
	int li=m_label_to_index[m_test_subset[i].label_1];
	int lj=m_label_to_index[m_test_subset[i].label_2];

	char tmp[10];
	sprintf(tmp,"%d",m_test_subset[i].save_index);
	string sn=SCORE_DIR+tmp;
	(*fin).open(sn.c_str());
	(*fin).seekg(score_seek[i],ios::beg);
	
	(*fin) >> test_data_index;
	
	for (int j=m_test_subset[i].start_1;j<=m_test_subset[i].end_1;j++){
	  Block_Index bi;
	  bi.process=m_test_subset[i].process_1;
	  bi.index=j;

	  double flag=(score_vector[li][lj].find(bi)==score_vector[li][lj].end());
	  double sm=(flag)?0:score_vector[li][lj][bi];

	  if (m3_parameter->m3_min_max_mode==0){
	    sm=get_min_score_full(m_test_subset[i].start_2,
				  m_test_subset[i].end_2,
				  fin,
				  sm,
				  flag);
	  }
	  else if (m3_parameter->m3_min_max_mode==1){
	    sm=get_min_score_min_vector(fin,
					sm,
					flag);
	  }
	  
	  // 	  // debug
	  // 	  TIME_DEBUG_OUT << sm << " " << bi.process << " " << bi.index << endl;
	  // 	  if (score_vector[li][lj].find(bi)!=score_vector[li][lj].end())
	  // 	    TIME_DEBUG_OUT << " " << score_vector[li][lj][bi] << endl;
	  // 	  else TIME_DEBUG_OUT << " nil" << endl;
	  
	  score_vector[li][lj][bi]=sm;

	  // 	  // debug
	  // 	  TIME_DEBUG_OUT << score_vector[li][lj][bi] << endl;

	}

	score_seek[i]=(*fin).tellg();
	(*fin).close();
      }
      for (int i=0;i<label_len;i++)
	for (int j=0;j<label_len;j++){
	  double sm;
	  it=score_vector[i][j].begin();
	  sm=(*it).second;
	  for (;it!=score_vector[i][j].end();it++)
	    sm=max(sm,(*it).second); // MIN-MAX: MAX

	  score_matrix[i][j]=sm;
	}

      Voter * voter=M3_Factory::create_voter(m3_parameter->voter_rank);//new HC_Voter;
      voter->matrix_inverse(m_index_to_label,
			    score_matrix);
      result_matrix << tms << ": " << endl;
      for (int ii=0;ii<label_len;ii++){
	for (int jj=0;jj<label_len;jj++)
	  result_matrix << score_matrix[ii][jj] << " ";
	result_matrix << endl;
      }
      result_matrix << endl;
      result << tms << ": " 
	     << voter->vote(m_index_to_label,
			    score_matrix) << endl;
      
    }

  result.close();
  result_matrix.close();

  for (int i=0;i<score_file.size();i++){
    (*(score_file[i])).close();
    delete score_file[i];
  }
  score_file.clear();
  for (int i=0;i<label_len;i++){
    for (int j=0;j<label_len;j++)
      score_vector[i][j].clear();
    delete [] score_matrix[i];
    delete [] score_vector[i];
  }

  delete [] score_matrix;
  delete [] score_vector;
}

void M3::M3_Master::score_file_combine_pruning(){

  for (int i=0;i<m_index_to_label.size()-1;i++){
    char cmb_name[100];
    sprintf(cmb_name,"%f",m_index_to_label[i]);
    ofstream file_out((SCORE_DIR+cmb_name).c_str());
    ifstream * file_in=new ifstream[m_index_to_label.size()];

    for (int j=i+1;j<m_index_to_label.size();j++){
      char name[10];
      sprintf(name,"%f",m_index_to_label[j]);
      file_in[j].open((SCORE_DIR+cmb_name+"_"+name).c_str());
    }

    float sc;
    int index;

    while (file_in[i+1] >> index){
      file_in[i+1] >> sc;

      file_out << index << " " << sc << " ";

      for (int j=i+2;j<m_index_to_label.size();j++){
	file_in[j] >> index >> sc;
	file_out << sc << " ";
      }
      file_out << endl;
    }

    for (int j=i+1;j<m_index_to_label.size();j++)
      file_in[j].close();
    delete [] file_in;
    file_out.close();
  }
}

void M3::M3_Master::score_test_data_pruning(vector<bool> test_flag){
  ofstream result(RESULT.c_str());
  ofstream result_matrix(RESULT_MATRIX.c_str());

  int label_len=m_index_to_label.size();

  double ** score_matrix=new double*[label_len];
  for (int i=0;i<label_len;i++)
    score_matrix[i]=new double[label_len];

  score_file_combine_pruning();

  ifstream * file_in=new ifstream[label_len-1];
  for (int i=0;i<label_len-1;i++){
    char name[100];
    sprintf(name,"%f",m_index_to_label[i]);
    file_in[i].open((SCORE_DIR+name).c_str());
  }

  // debug
  // runing flag
  cout << "master begin to get matrix" << endl;
  cout << test_flag.size() << endl;

  int index;
  for (int tms=0;tms<test_flag.size();tms++)
    if (test_flag[tms]){

      for (int i=0;i<label_len-1;i++){
	file_in[i] >> index;
	for (int j=i+1;j<label_len;j++){
	  file_in[i] >> score_matrix[i][j];
	}
      }
      
      Voter * voter=M3_Factory::create_voter(m3_parameter->voter_rank);//new HC_Voter;
      voter->matrix_inverse(m_index_to_label,
			    score_matrix);
      result_matrix << index << ": " << endl;
      for (int ii=0;ii<label_len;ii++){
	for (int jj=0;jj<label_len;jj++)
	  result_matrix << score_matrix[ii][jj] << " ";
	result_matrix << endl;
      }
      result_matrix << endl;
      result << index << ": " 
	     << voter->vote(m_index_to_label,
			    score_matrix) << endl;
    }

  for (int i=0;i<label_len-1;i++)
    file_in[i].close();
  delete [] file_in;
  for (int i=0;i<label_len;i++)
    delete [] score_matrix[i];
  delete [] score_matrix;
}

void M3::M3_Master::score_multilabel_pruning(vector<bool> test_flag){
    
  ofstream result(RESULT.c_str());

  int label_len=m_index_to_label.size();

  ifstream * file_in=new ifstream[label_len];
  for (int i=0;i<label_len;i++){
    char name[100];
    sprintf(name,"%f_rest",m_index_to_label[i]);
    file_in[i].open((SCORE_DIR+name).c_str());
  }

  // debug
  cout << "Beging to score the result " << test_flag.size() << endl;

  for (int id=0;id<test_flag.size();id++){
      bool flag=false;
      bool pos=false;
      double msc=-1;
      int msci=0;
      result << id << " ";
      for (int i=0;i<label_len;i++){    
          int index; 
          double sc;
          file_in[i] >> index >> sc;
          if (sc>=0){
              if (flag)
                  result << ",";
              result << m_index_to_label[i];
              pos=true;
              flag=true;
          }
          if (msc<sc){
              msc=sc;
              msci=i;
          }
      }
      if (!pos)
          result << m_index_to_label[msci];
      result << endl;
  }

  // debug
  cout << "Score done " << endl;


  result.close();
  for (int i=0;i<label_len;i++)
    file_in[i].close();
  delete [] file_in;
}
void M3::M3_Master::score_test_data(){
  load_subset_config();

  if (testLen==-1)
    M3::testLen = M3::wc_l(m3_parameter->classify_data);

  vector<bool> test_flag;
  test_flag.clear();
  size_t hossL = M3::testLen;
  for (int i=0;i<hossL;i++)
    test_flag.push_back(true);

  if (m3_parameter->m3_multilabel)
      score_multilabel_pruning(test_flag);
  else{
      if (m3_parameter->m3_pruning_mode==0)
          score_test_data_nonpruning(test_flag);
      else if (m3_parameter->m3_pruning_mode!=0 && m3_parameter->m3_pruning_combine_score==0)
          score_test_data_pruning(test_flag);
  }

  // debug
  TIME_DEBUG_OUT << " score over" << endl;


}

void M3::M3_Master::compare_true_singlelabel(const string &filename){
  ifstream m3_out(RESULT.c_str());
  ifstream true_out(filename.c_str());
  ofstream result(RESULT_COMPARE.c_str());

  map<double,int> m3_number,true_number,same_number;
  string tmp;

  double m3_label,true_label;
  while (m3_out >> tmp >> m3_label){
    true_out >> true_label;

    result << tmp << " " << m3_label << " " << true_label << endl;

    if (m3_number.find(m3_label)==m3_number.end())
      m3_number[m3_label]=0;
    m3_number[m3_label]++;

    if (true_number.find(true_label)==true_number.end())
      true_number[true_label]=0;
    true_number[true_label]++;

    if (m3_label==true_label){
      if (same_number.find(m3_label)==same_number.end())
	same_number[m3_label]=0;
      same_number[m3_label]++;
    }
  }

  result << endl << endl << endl;

  int all_same=0,all=0;
  double macro_precition=0;
  double macro_recall=0;
  double macro_f1=0;
  for (int i=0;i<m_index_to_label.size();i++){
    double ll=m_index_to_label[i];
    double precition=1.0*same_number[ll]/m3_number[ll];
    double recall=1.0*same_number[ll]/true_number[ll];
    double F1=1.0*precition*recall*2/(precition+recall);
    result << "Label " << ll  
	   << "\t\tm3 : "  << m3_number[ll]
	   << "\t\ttrue : " << true_number[ll]
	   << "\t\tright : " << same_number[ll]
	   << "\t\tprecition : " << precition*100.0 << "%"
	   << "\t\trecall : " << recall*100.0 << "%" 
	   << "\t\tF1 : " << F1*100.0 << "%"
	   << endl;

    macro_precition+=precition;
    macro_recall+=recall;

    all_same+=same_number[ll];
    all+=true_number[ll];
  }

  macro_precition/=m_index_to_label.size();
  macro_recall/=m_index_to_label.size();
  macro_f1=macro_precition*macro_recall*2/(macro_precition+macro_recall);

  result << endl
	 << "all accurace : " << 100.0*all_same/all << "%" << endl;
  result << "macro_precition : " << 100.0*macro_precition << "%" << endl;
  result << "macro_recall : " << 100.0*macro_recall << "%" << endl;
  result << "macro_f1 : " << 100.0*macro_f1 << "%" << endl;

  m3_out.close();
  true_out.close();
  result.close();
}
void M3::M3_Master::compare_true_multilabel(const string & filename){
  ifstream m3_out(RESULT.c_str());
  ifstream true_out(filename.c_str());
  ofstream result(RESULT_COMPARE.c_str());

  map<double,int> m3_number,true_number,same_number;
  string tmp;

  double m3_label,true_label;
  string m3_mlabel,true_mlabel;
  while (m3_out >> tmp >> m3_mlabel){
      true_out >> true_mlabel;

      result << tmp << " " << m3_mlabel << " " << true_mlabel << endl;

      set<float> mlabels;
      mlabels.clear();
      int pri=0;

      for (int i=0;pri<=m3_mlabel.size();i++)
          if (i>=m3_mlabel.size() || m3_mlabel[i]==','){
              m3_label=atof(m3_mlabel.substr(pri,i-pri).c_str());
              pri=i+1;
              if (m3_number.find(m3_label)==m3_number.end())
                  m3_number[m3_label]=0;
              m3_number[m3_label]++;
              mlabels.insert(m3_label);
          }


      pri=0;  
      for (int i=0;pri<=true_mlabel.size();i++)
          if (i>=true_mlabel.size() || true_mlabel[i]==','){
              true_label=atof(true_mlabel.substr(pri,i-pri).c_str());
              pri=i+1;
              if (true_number.find(true_label)==true_number.end())
                  true_number[true_label]=0;
              true_number[true_label]++;


              if (mlabels.find(true_label)!=mlabels.end()){
                  if (same_number.find(m3_label)==same_number.end())
                      same_number[true_label]=0;
                  same_number[true_label]++;
              }
          }
  }

  result << endl << endl << endl;

  int all_same=0,all=0;
  double macro_precition=0;
  double macro_recall=0;
  double macro_f1=0;
  for (int i=0;i<m_index_to_label.size();i++){
    double ll=m_index_to_label[i];
    double precition=1.0*same_number[ll]/m3_number[ll];
    double recall=1.0*same_number[ll]/true_number[ll];
    double F1=1.0*precition*recall*2/(precition+recall);
    result << "Label " << ll  
	   << "\t\tm3 : "  << m3_number[ll]
	   << "\t\ttrue : " << true_number[ll]
	   << "\t\tright : " << same_number[ll]
	   << "\t\tprecition : " << precition*100.0 << "%"
	   << "\t\trecall : " << recall*100.0 << "%" 
	   << "\t\tF1 : " << F1*100.0 << "%"
	   << endl;

    macro_precition+=precition;
    macro_recall+=recall;

    all_same+=same_number[ll];
    all+=true_number[ll];
  }

  macro_precition/=m_index_to_label.size();
  macro_recall/=m_index_to_label.size();
  macro_f1=macro_precition*macro_recall*2/(macro_precition+macro_recall);

  result << endl
	 << "all accurace : " << 100.0*all_same/all << "%" << endl;
  result << "macro_precition : " << 100.0*macro_precition << "%" << endl;
  result << "macro_recall : " << 100.0*macro_recall << "%" << endl;
  result << "macro_f1 : " << 100.0*macro_f1 << "%" << endl;

  m3_out.close();
  true_out.close();
  result.close();
}
void M3::M3_Master::compare_true_label(const string & filename){

    if (!m3_parameter->m3_multilabel){
        compare_true_singlelabel(filename);
    }
    else {
        compare_true_multilabel(filename);
    }

}
//M3_Slave




M3::M3_Slave::M3_Slave(){
  m_sample_link_head=NULL;
  m_sample_link_tail=NULL;

  m_train_data_num=0;
  m_memory_enough=true;
  m_il_process_rank=-1;
}

M3::M3_Slave::~M3_Slave(){

  TIME_DEBUG_OUT << "come here to delete the slave" << endl;

  for (int i=0;i<m_sample_arr_pool.size();i++)
    if (m_sample_arr_pool[i]!=NULL){
    delete [] m_sample_arr_pool[i];
    m_divide_situation_pool[i].clear();
  }
  m_sample_arr_pool.clear();
  m_divide_situation.clear();

  Sample_Link * sl,* sl_tmp;
  for (sl=m_sample_link_head;sl!=NULL;){
    Data_Sample * ds=sl->sample_head;
    delete [] (ds[0].data_vector);
    delete [] ds;
    sl_tmp=sl;
    sl=sl->next;
    delete sl_tmp;
  }
}

// As name
float M3::M3_Slave::string_to_float(char * str,
				    int ll,
				    int rr){
  char temp = str[rr];
  str[rr] = 0;
  float res = atof(&(str[ll]));
  str[rr] = temp;
  return res;
}

// As name
int M3::M3_Slave::string_to_int(char * str,
				int ll,
				int rr){
  char temp = str[rr];
  str[rr] = 0;
  int res = atoi(&(str[ll]));
  str[rr] = temp;
  return res;
}

// Parse input data to our format.
// NOTE: NOT THE SAME AS THE MASTER_LOAD_SERIAL
void M3::M3_Slave::parse_data(char * rbuf,Data_Sample * dsp){
    int i=0,pri=0;
    int len=0;

    while (rbuf[i]!=' ') i++; 
    dsp->index=string_to_float(rbuf,pri,i);

    pri=++i;
    while (rbuf[i]!=' ') i++; 
    dsp->label=string_to_float(rbuf,pri,i);

    pri=++i;
    while (rbuf[i]!=' ') i++; 
    dsp->mlabel_len=string_to_int(rbuf,pri,i);

    dsp->mlabel=new float[dsp->mlabel_len];
    for (int k=0;k<dsp->mlabel_len;k++){
        pri=++i;
        while (rbuf[i]!=' ' && rbuf[i]!=',') i++;
        dsp->mlabel[k]=string_to_float(rbuf,pri,i);
    }

    while (rbuf[i]){
        pri=++i;
        while (rbuf[i] && rbuf[i]!=':') i++;
        if (!rbuf[i]) break;
        dsp->data_vector[len].index=string_to_int(rbuf,pri,i);
        pri=++i;
        while (rbuf[i] && rbuf[i]!=' ') i++;
        dsp->data_vector[len].value=string_to_float(rbuf,pri,i);
        len++;
    }
    dsp->data_vector_length=len;
}

// Unpackage the load data to our struct.
void M3::M3_Slave::data_unpackage(Data_Sample * sample_buf,
				  Data_Node * node_buf,
				  int sp_buf_len,
				  int nd_buf_len){
  if (m_sample_link_tail==NULL){
    m_sample_link_head=m_sample_link_tail=new Sample_Link;
    m_sample_link_tail->sample_head=sample_buf;
    m_sample_link_tail->next=NULL;
  }
  else {
    m_sample_link_tail->next=new Sample_Link;
    m_sample_link_tail=m_sample_link_tail->next;
    m_sample_link_tail->sample_head=sample_buf;
    m_sample_link_tail->next=NULL;
  }

  m_sample_link_tail->length=sp_buf_len;
  m_train_data_num+=sp_buf_len;

  int nb_offset=0;
  int i;
  for (i=0;i<sp_buf_len;i++){	// point to local address
    sample_buf[i].data_vector=&node_buf[nb_offset];
    nb_offset+=sample_buf[i].data_vector_length;
  }
}

// Print middle information.
void M3::M3_Slave::check_load_data(){
  Sample_Link * sl;
  int tms=0;
  for (sl=m_sample_link_head;sl!=NULL;sl=sl->next){
    TIME_DEBUG_OUT << "In slave_process: " 
		   << m3_my_rank 
		   << " sample_link: " 
		   << ++tms 
		   << endl;
    int i;
    Data_Sample * ds=sl->sample_head;
    for (i=0;i<sl->length;i++){
      TIME_DEBUG_OUT << "slave_process[" 
		     << m3_my_rank 
		     << "][" <<tms << "][" << i << "]:"
		     << ds[i].index << " " 
		     << ds[i].label << " " 
		     << ds[i].data_vector_length << endl;
      int j;
      Data_Node * dn=ds[i].data_vector;
      for (j=0;j<ds[i].data_vector_length;j++)
	debug_out << "(" << dn[j].index << "," << dn[j].value << ")";
      debug_out << endl;
    }
  }
}

// Main load block.
void M3::M3_Slave::load_train_data_serial(){

  // debug
  TIME_DEBUG_OUT << "come in the process: " 
		 << m3_my_rank 
		 <<" load_train_data" 
		 << endl;

  MPI_Status mpi_status;
  int will_do;
  bool read_done_flag=false;
  Data_Sample * sample_buf;
  Data_Node * node_buf;
  int sb_len,nb_len;

  while (!read_done_flag){		// ask & answer
    // Get the control.
    MPI_Recv(&will_do,
	     1,
	     MPI_INT,
	     M3_MASTER_RANK,
	     M3_TAG,
	     MPI_COMM_WORLD,
	     &mpi_status);

    // debug
    TIME_DEBUG_OUT << "slave_process:" 
		   << m3_my_rank 
		   << " receive the CTRL: " 
		   << will_do 
		   << endl;

    // Load is over.
    if (will_do==CTRL_READ_DONE){ // will_do==0
      read_done_flag=true;
    }
    // Must to ask whether the memory is enough.
    else if (will_do==CTRL_ALLOC_MEMORY){ // will_do==1
      // Handle the memory is not enough
      void (* old_handler)()=set_new_handler(M3::M3_Slave::local_memory_scarcity);

      int ask_len[2];
      // Get the ask information.
      MPI_Recv(ask_len,
	       2,
	       MPI_INT,
	       M3_MASTER_RANK,
	       M3_TAG+1,
	       MPI_COMM_WORLD,
	       &mpi_status);
      sb_len=ask_len[0];
      nb_len=ask_len[1];

      // debug
      TIME_DEBUG_OUT << "slave_procss: " 
		     << m3_my_rank 
		     << " require memroy: " 
		     << sb_len 
		     << " & " 
		     << nb_len 
		     << endl;

      // Ask memory.
      try{
	sample_buf=new Data_Sample[sb_len];
      } catch (exception e){
	m_memory_enough=false;
      }
      try{
	node_buf=new Data_Node[nb_len];
      } catch (exception e){
	delete [] sample_buf;
	m_memory_enough=false;
      }
      set_new_handler(old_handler);


      // debug
      // for test if there is no enough memory on one process
      // if (sb_len+m_train_data_num>DEBUG_MEMORY_CAPACITY)
      // 	m_memory_enough=false;
      //       else 
      // 	m_train_data_num+=sb_len;

      // Return response.
      if (m_memory_enough)
	MPI_Send(&CTRL_MEMORY_ENOUGH,
		 1,
		 MPI_INT,
		 M3_MASTER_RANK,
		 M3_TAG, 
		 MPI_COMM_WORLD);
      else 
	MPI_Send(&CTRL_MEMORY_SCARCITY,
		 1,
		 MPI_INT,
		 M3_MASTER_RANK,
		 M3_TAG,
		 MPI_COMM_WORLD);
    }
    // Get data and unpackage them.
    else if (will_do==CTRL_GET_DATA){	// will_do==2

      // debug
      TIME_DEBUG_OUT << "slave_process: " 
		     << m3_my_rank 
		     << " begin to receive sample ,len:" 
		     <<sb_len << endl;

      MPI_Recv(sample_buf,
	       sb_len,
	       MPI_Data_Sample,
	       M3_MASTER_RANK,
	       M3_TAG+1,
	       MPI_COMM_WORLD,
	       &mpi_status);

      m_my_label=sample_buf[0].label;

      // debug
      TIME_DEBUG_OUT << "slave_process: " 
		     << m3_my_rank 
		     << " begin to receive node ,len:" 
		     << nb_len 
		     << endl;

      MPI_Recv(node_buf,
	       nb_len,
	       MPI_Data_Node,
	       M3_MASTER_RANK,
	       M3_TAG+2,
	       MPI_COMM_WORLD,
	       &mpi_status);
      data_unpackage(sample_buf,
		     node_buf,
		     sb_len,
		     nb_len);

      // debug
      // check receive buf
      TIME_DEBUG_OUT << "slave_process: " 
		     << m3_my_rank 
		     << " check receive buf" 
		     << endl;
      for (int i=0;i<sb_len;i++)
	TIME_DEBUG_OUT << sample_buf[i].index << " "
		       << sample_buf[i].label << " "
		       << sample_buf[i].data_vector_length << endl;
      for (int i=0;i<nb_len;i++){
	if (i%8==0) debug_out << endl;
	debug_out << "(" << node_buf[i].index << "," << node_buf[i].value << ")";
      }
      debug_out << endl;

    }
  }

  // debug
  TIME_DEBUG_OUT << "slave_process: " 
		 << m3_my_rank 
		 << " read done " 
		 << endl;

  // debug
  //  check_load_data();

}

// do something to complete it
// true denote memory enough,o.t.w false
// one way :using /proc/meminfo 
bool M3::M3_Slave::memory_test(int sp_buf_len,
			       int nd_buf_len){
  void (*old_handler)()=set_new_handler(local_memory_scarcity);
  Data_Node *dn;
  try{
    dn=new Data_Node[nd_buf_len];
  }catch (exception e){
    set_new_handler(old_handler);
    return false;
  }

  set_new_handler(old_handler);
  delete [] dn;
  return true;
}

void M3::M3_Slave::increase_learning_pre_load(){


  char buf[100];
  sprintf(buf,"%s%d",DIVID_DATA_IL_DIR.c_str(),m3_my_rank);
  ifstream conf(buf);

  // debug
  TIME_DEBUG_OUT << "slave_process " << m3_my_rank << " lode pre config frome " << buf << endl;

  int pool_len;
  int subset_num;
  string tmp;

  conf >> tmp >> pool_len;
  conf >> tmp >> m_train_data_num;
  conf >> tmp >> subset_num;

  for (int i=0;i<pool_len;i++){
    Data_Sample ** ds=new Data_Sample*[m_train_data_num];
    m_sample_arr_pool.push_back(ds);
    conf >> tmp;
    for (int j=0;j<m_train_data_num;j++){
      int id;
      conf >> id;
      if (m_il_index_inverse.find(id)==m_il_index_inverse.end()){
	vector<int> vi;
	vi.clear();
	m_il_index_inverse[id]=vi;
      }
      m_il_index_inverse[id].push_back(j);
    }

    conf >> tmp;
    int count=0;
    vector<Divide_Info> vdi;
    vdi.clear();
    for (int j=0;j<subset_num;j++){
      int def;
      conf >> def;
      Divide_Info di;
      di.start_offset=count;
      di.end_offset=count+def-1;
      di.length=def;
      vdi.push_back(di);
    }
    m_divide_situation_pool.push_back(vdi);
  }
  conf.close();

  memset(buf,0,sizeof(buf));
  sprintf(buf,"%s%d.dat",DIVID_DATA_IL_DIR.c_str(),m3_my_rank);
  ifstream data(buf);

  // debug
  TIME_DEBUG_OUT << "slave_process " << m3_my_rank << " lode pre data frome " << buf << endl;

  char * read_buf=new char[READ_BUF_SIZE];

  while (true){
    memset(read_buf,0,sizeof(char)*READ_BUF_SIZE);
    if (!data.getline(read_buf,READ_BUF_SIZE))
      break;

    int node_len=0;
    for (int i=0;read_buf[i];i++)
      node_len+=(read_buf[i]==':');

    // debug
    BEGIN_DEBUG;
    TIME_DEBUG_OUT << "the buf: " << read_buf << endl;
    END_DEBUG;

    Data_Sample * ds=new Data_Sample;
    Data_Node * dn=new Data_Node[node_len];
    ds->data_vector=dn;
    
    parse_data(read_buf,
	       ds);

    m_my_label=ds->label;

    data_unpackage(ds,
		   dn,
		   1,
		   node_len);

    int id=ds->index;
    for (int i=0;i<pool_len;i++){
      int iid=m_il_index_inverse[id][i];
      m_sample_arr_pool[i][iid]=ds;
    }

  }
  delete [] read_buf;
  data.close();

  // debug
  BEGIN_DEBUG;
  check_load_data();
  END_DEBUG;

}

// Slave load use parallel IO
void M3::M3_Slave::load_train_data_parallel(){
  // information define: int info[4];char label_file[1000];
  // info[0]=situation tag
  // info[1]=process rank
  // info[2]=now file offset
  // info[3]=sample num

  MPI_Status mpi_status;

  if (m3_parameter->m3_increase_learning){
    MPI_Recv(&m_il_process_rank,
	     1,
	     MPI_INT,
	     M3_MASTER_RANK,
	     M3_TAG,
	     MPI_COMM_WORLD,
	     &mpi_status);
  }

  if (m_il_process_rank!=-1){

    // debug
    TIME_DEBUG_OUT << "slave_process " << m3_my_rank << " is begin to read the pre done data" << endl;

    increase_learning_pre_load();

    // debug
    TIME_DEBUG_OUT << "slave_process " << m3_my_rank << " pre load done" << endl;
    return;
  }


  // debug
  TIME_DEBUG_OUT << "slave_process " << m3_my_rank
		 << "beging to load train data parallel" << endl;

  int info[4];
  char label_file[1000];
  char * read_buf=new char[READ_BUF_SIZE];


  MPI_Recv(info,
	   4,
	   MPI_INT,
	   M3_MASTER_RANK,
	   M3_TAG,
	   MPI_COMM_WORLD,
	   &mpi_status);

  // debug
  TIME_DEBUG_OUT << "slave_process " << m3_my_rank
		 << "has receive the infomation : with situation=" << info[0]
		 << " file_offset=" << info[2] << endl;

  if (info[0]==CTRL_READ_DONE){

    // debug
    TIME_DEBUG_OUT << "slave_process " << m3_my_rank
		   << "read done " << endl;

  }

  else {

    int situation=info[0];
    int my_rank=info[1];
    int file_offset=info[2];
    int sample_num=0;		// info[3]

    MPI_Recv(label_file,
	     1000,
	     MPI_CHAR,
	     M3_MASTER_RANK,
	     M3_TAG,
	     MPI_COMM_WORLD,
	     &mpi_status);

    // debug
    TIME_DEBUG_OUT << "slave_process " << m3_my_rank
		   << "receive file_name=" << label_file << endl;

    bool read_done_flag=true;
    FILE * data_file=fopen(label_file,"r");
    fseek(data_file,
	  file_offset,
	  SEEK_SET);

    int memory_test_tms=0;

	while (!feof(data_file)){

		if (!memory_test(1,NODE_SIMPLE_BUF_SIZE)){

			// debug
			TIME_DEBUG_OUT << "slave_process " << m3_my_rank
				<< " memory is not more enough " << endl;

			read_done_flag=false;
			break;
		}

		//       // debug
		//       // debug
		//       // debug
		//       memory_test_tms++;
		//       if (memory_test_tms>=150){

		// 	// debug
		// 	TIME_DEBUG_OUT << "slave_process " << m3_my_rank
		// 		       << " memory is not more enough " << endl;

		// 	read_done_flag=false;
		// 	break;
		//       }


		memset(read_buf,0,sizeof(char)*READ_BUF_SIZE);
		int index=0;
		int node_len=0;

		char cc;
		while (1){			// igore "nl" && "er"
			cc=getc(data_file);
			if (cc!=10 && cc!=13)
				break;
		}
		read_buf[index++]=cc;
		if (cc==EOF)		// file over
			break;
		while (1){
			cc=getc(data_file);
			if (cc==10 || cc==13 || cc==EOF)
				break;
			node_len+=(cc==':');
			read_buf[index++]=cc;
		}

 //     // debug
 //      TIME_DEBUG_OUT << "slave_process " << m3_my_rank << "has read the buf: " << read_buf << endl;

      Data_Sample * sample=new Data_Sample;
      Data_Node * node=new Data_Node[node_len];
      sample->data_vector=node;

      parse_data(read_buf,sample);

      m_my_label=sample->label;

      BEGIN_DEBUG;
      // debug
      TIME_DEBUG_OUT << "slave_process " << m3_my_rank
		     << " has parse the buf " << endl;
      END_DEBUG;

      data_unpackage(sample,
		     node,
		     1,
		     node_len);

      sample_num++;

      // debug
//       TIME_DEBUG_OUT << "slave_process " << m3_my_rank 
// 		     << " " << sample->index 
// 		     << " " << sample->label
// 		     << " " << sample->data_vector_length
// 		     << endl;
//       TIME_DEBUG_OUT;
//       for (int ii=0;ii<sample->data_vector_length;ii++)
// 	debug_out << "(" << sample->data_vector[ii].index
// 		  << "," << sample->data_vector[ii].value
// 		  << ")" ;
//       debug_out << endl;

      BEGIN_DEBUG;
      // debug
      TIME_DEBUG_OUT << "slave_process " << m3_my_rank 
		     << " has link the sample " << endl;
      END_DEBUG;

    }

    file_offset=ftell(data_file);
    if (read_done_flag)
      situation=CTRL_READ_DONE;
    else situation=CTRL_TRAIN_LOAD_FULL;

    info[0]=situation;
    info[1]=my_rank;
    info[2]=file_offset;
    info[3]=sample_num;

    MPI_Send(info,
	     4,
	     MPI_INT,
	     M3_MASTER_RANK,
	     M3_SUBSET_INFO_TAG,
	     MPI_COMM_WORLD);
  }

  delete [] read_buf;

  // debug
  //  check_load_data();

}

// Slave load_train_data interface
void M3::M3_Slave::load_train_data(){

  if (m3_parameter->m3_load_mode==0){
    load_train_data_parallel();
  }
  else if (m3_parameter->m3_load_mode==1){
    load_train_data_serial();
  }

}

// Make the data to be ** Data_Sample to divide.
void M3::M3_Slave::pre_divide(){
  Sample_Link * sl;

  m_sample_arr=new Data_Sample*[m_train_data_num];
  int i=0,index=0;
  Data_Sample * ds;
  for (sl=m_sample_link_head;sl!=NULL;sl=sl->next){
    ds=sl->sample_head;
    for (i=0;i<sl->length;i++)
      m_sample_arr[index++]=&(sl->sample_head[i]);
  }
}

// Print middle information.
void M3::M3_Slave::check_divide_data(){
  TIME_DEBUG_OUT << "Process " 
		 << m3_my_rank
		 << " has already divid "
		 << m_train_data_num 
		 << " data(s) " 
		 << " to "
		 << m_divide_situation.size()
		 << " block(s)" 
		 << endl;

  int i;
  for (i=0;i<m_divide_situation.size();i++){
    Divide_Info di=m_divide_situation[i];
    TIME_DEBUG_OUT << "Block "
		   << i 
		   << " : "
		   << di.start_offset
		   << " to "
		   << di.end_offset
		   << endl;
  }
}


void M3::M3_Slave::multilabel_get_all_label(){
    if (!m3_parameter->m3_multilabel)
        return;
    int len;
    MPI_Status mpi_status;
    MPI_Recv(&len,
        1,
        MPI_INT,
        M3_MASTER_RANK,
        M3_TAG,
        MPI_COMM_WORLD,
        &mpi_status);
    float * mlabel=new float[len];
    MPI_Recv(mlabel,
        len,
        MPI_FLOAT,
        M3_MASTER_RANK,
        M3_TAG,
        MPI_COMM_WORLD,
        &mpi_status);
    m_index_to_label.clear();
    for (int i=0;i<len;i++)
        m_index_to_label.push_back(mlabel[i]);
    delete mlabel;
}


void M3::M3_Slave::multilabel_sent_divide_info(){
    if (!m3_parameter->m3_multilabel)
        return ;
    int len=m_index_to_label.size();
    int *num_sub=new int[len];
    for (int i=0;i<len;i++){
        int ti=i;
        if (ti>=m_divide_situation_pool.size())
            ti=m_divide_situation_pool.size()-1;
        num_sub[i]=m_divide_situation_pool[ti].size();
    }

    // debug
    TIME_DEBUG_OUT << "multilabel divide info: " ;
    for (int i=0;i<len;i++)
        debug_out << num_sub[i] << " ";
    debug_out << endl;

    MPI_Send(num_sub,
        len,
        MPI_INT,
        M3_MASTER_RANK,
        M3_TAG,
        MPI_COMM_WORLD);
    delete [] num_sub;
}

void M3::M3_Slave::divide_train_data()
{
    pre_divide();

    // debug
    TIME_DEBUG_OUT << "slave_process " << m3_my_rank << " now begin to divide data " << endl;

    int send_info[2];

    if (m_il_process_rank!=-1){
        send_info[0]=m3_my_rank;
        send_info[1]=m_divide_situation_pool[0].size();
        MPI_Send(send_info,
            2,
            MPI_INT,
            M3_MASTER_RANK,
            M3_TAG,
            MPI_COMM_WORLD);

        // debug
        TIME_DEBUG_OUT << "save for next" << endl;

        //save_for_increase_learning();

        // debug
        TIME_DEBUG_OUT << "divide done" << endl;

        return ;
    }

    // Make divider and others(havn't new)
    Divider * m3_divider=M3_Factory::create_divider(m3_parameter->divider_rank); //new Hyper_Plane();
    if (m_train_data_num>0)
    {

        multilabel_get_all_label();

        m3_divider->parse("divide.config");
        //     m_divide_situation=m3_divider->divide(m_sample_arr,
        // 					  m_train_data_num,
        // 					  m3_subset_size);//added by hoss (be a parameter)
       if (!m3_parameter->m3_multilabel)
            m3_divider->divide(m_sample_arr,
            m_train_data_num,
            m3_subset_size,
            m_my_label,
            m_sample_arr_pool,
            m_divide_situation_pool,
            "divide_info.config");             
        else m3_divider->multi_label_divide(m_sample_arr,
            m_train_data_num,
            m3_subset_size,
            m_my_label,
            m_sample_arr_pool,
            m_divide_situation_pool,
            "divide_info.config",
            m_index_to_label);
       

        delete [] m_sample_arr;
    }
    delete m3_divider;

    // debug
    TIME_DEBUG_OUT << "slave_process " 
        << m3_my_rank 
        << " now finish the divide " 
        << endl;

    // Send divide informatino.

    send_info[0]=m3_my_rank;
    if (m_divide_situation_pool.size()>0)
        send_info[1]=m_divide_situation_pool[0].size();
    else 
        return;			// no need to response

    MPI_Send(send_info,
        2,
        MPI_INT,
        M3_MASTER_RANK,
        M3_TAG,
        MPI_COMM_WORLD);

    multilabel_sent_divide_info();

    // debug
    TIME_DEBUG_OUT << "slave_process " 
        << m3_my_rank 
        << " now has sent all info " 
        << endl;

    // debug
    for (int i=0;i<m_divide_situation_pool.size();i++){
        TIME_DEBUG_OUT << "pool " << i << endl;
        m_divide_situation=m_divide_situation_pool[i];
        check_divide_data();
    }

    save_for_increase_learning();
}

void M3::M3_Slave::save_data_for_increase_learning(ofstream & os,
						   Data_Sample & ds){
  os << ds.index << " " << ds.label << " ";
  for (int i=0;i<ds.data_vector_length;i++){
    os << ds.data_vector[i].index << ":" << ds.data_vector[i].value;
    if (i!=ds.data_vector_length-1)
      os  << " ";
  }
  os << endl;
}

void M3::M3_Slave::save_for_increase_learning(){
  if (!m3_parameter->m3_save_for_increase_learning)
    return;

  // debug
  TIME_DEBUG_OUT << "save for increase learning" << endl;

  int pl=m_divide_situation_pool.size();

  if (pl<=0)
    return;

  char buf[100];
  sprintf(buf,"%s%d",DIVID_DATA_IL_DIR,m3_my_rank);
  ofstream savef(buf);

  savef << "pool_len= " << pl << endl;

  savef << "data_num= "<<  m_train_data_num << endl;
  
  vector<Divide_Info> vdi=m_divide_situation_pool[0];
  savef << "divide_num= " << vdi.size() << endl;
  
  for (int i=0;i<m_divide_situation_pool.size();i++){

    Data_Sample ** ds=m_sample_arr_pool[i];

    // debug
    BEGIN_DEBUG;
    TIME_DEBUG_OUT << "i:" << i << endl;
    TIME_DEBUG_OUT << "ds:" << ds << endl;
    TIME_DEBUG_OUT << "pool[" << i << "]: ";
    for (int j=0;j<m_train_data_num;j++)
      debug_out << ds[j]->index << " ";
    debug_out << endl;
    END_DEBUG;

    savef << "pool[" << i << "]: ";
    for (int j=0;j<m_train_data_num;j++)
      savef << ds[j]->index << " ";
    savef << endl;
    
    vdi=m_divide_situation_pool[i];

    // debug
    BEGIN_DEBUG;
    TIME_DEBUG_OUT << "divide_arr: ";
    for (int j=0;j<vdi.size();j++)
      debug_out << vdi[j].length << " ";
    debug_out << endl;
    END_DEBUG;

    savef << "divide_arr: ";
    for (int j=0;j<vdi.size();j++)
      savef << vdi[i].length << " ";
    savef << endl;
  }  
  savef.close();
  
  memset(buf,0,sizeof(buf));
  sprintf(buf,"%s%d.dat",DIVID_DATA_IL_DIR,m3_my_rank);
  ofstream saved(buf);
  for (Sample_Link * sl=m_sample_link_head;sl!=NULL;sl=sl->next){
    for (int j=0;j<sl->length;j++)
      save_data_for_increase_learning(saved,sl->sample_head[j]);
  }
  saved.close();
}

// Package sample which is be train as continue memory space to send.
void M3::M3_Slave::subset_sample_package(int ll,
					 int rr,
					 Data_Sample * sample_buf,
					 int & nb_len){
  nb_len=0;
  int len=0;
  int i;
  for (i=ll;i<=rr;i++){
    int j;
    for (j=m_divide_situation[i].start_offset;
	 j<=m_divide_situation[i].end_offset;
	 j++){
      sample_buf[len++]=(*(m_sample_arr[j]));
      nb_len+=(*(m_sample_arr[j])).data_vector_length;
    }
  }
}

// Package node which is be train as continue memory space to send.
void M3::M3_Slave::subset_node_package(int sb_len,
				       Data_Sample * sample_buf,
				       Data_Node * node_buf){
  int len=0;
  int i;
  for (i=0;i<sb_len;i++){
    int j;
    for (j=0;j<sample_buf[i].data_vector_length;j++)
      node_buf[len++]=sample_buf[i].data_vector[j];
  }
}

// Main train block.
void M3::M3_Slave::training_train_data(){

  int will_do;
  MPI_Status mpi_status;

  while (1){
    // Get control.
    MPI_Recv(&will_do,
	     1,
	     MPI_INT,
	     M3_MASTER_RANK,
	     M3_TAG,
	     MPI_COMM_WORLD,
	     &mpi_status);

    // debug
    TIME_DEBUG_OUT << "slave_process " 
		   << m3_my_rank 
		   << " receive the CTRL: " 
		   << will_do 
		   << endl;

    // Train is over.
    if (will_do==CTRL_TRAIN_DONE){

      // debug
      TIME_DEBUG_OUT << "slave_process " << m3_my_rank << " train done" << endl;

      break;
    }
    // Train is not over.
    else if (will_do==CTRL_TRAIN_CONTINUE){

      int sent_process;
      int subset_left,subset_right;
      Data_Sample * sample_buf;
      int * subset_len;
      Data_Node * node_buf;
      int sb_len,nb_len;
      int subset_num;
      int to_label_id;
      
      // debug
      TIME_DEBUG_OUT << "slave_process " << m3_my_rank << " get relate information" << endl;

      // Receive send information : which data send to which process.
      int cmd[4];
      MPI_Recv(cmd,
	       4,
	       MPI_INT,
	       M3_MASTER_RANK,
	       M3_TAG,
	       MPI_COMM_WORLD,
	       &mpi_status);
      subset_left=cmd[0];
      subset_right=cmd[1];
      sent_process=cmd[2];
      to_label_id=cmd[3];
      subset_num=subset_right-subset_left+1;

      // debug
      TIME_DEBUG_OUT << "get done" << endl;

      int sap,dsp;
      sap=to_label_id;
      dsp=to_label_id;
      if (sap>=m_sample_arr_pool.size())
	sap=m_sample_arr_pool.size()-1;
      if(dsp>=m_divide_situation_pool.size())
	dsp=m_divide_situation_pool.size()-1;

      m_sample_arr=m_sample_arr_pool[sap];
      m_divide_situation=m_divide_situation_pool[dsp];

      // debug
      TIME_DEBUG_OUT << "slave_process " << m3_my_rank 
		     << " recv the infomation: send to process " << sent_process
		     << " the subset from " << subset_left
		     << " to " << subset_right << endl;

      // debug
      TIME_DEBUG_OUT << "slave_process " << m3_my_rank << " send subset length arr" << endl;

      // Send to the train slave the every subset length.
      subset_len=new int[subset_num];
      sb_len=0;
      int i;
      for (i=subset_left;i<=subset_right;i++){
	subset_len[i-subset_left]=m_divide_situation[i].length;
	sb_len+=m_divide_situation[i].length;
      }

      // debug
      BEGIN_DEBUG;
      TIME_DEBUG_OUT << "subset_len arr:";
      for (i=subset_left;i<=subset_right;i++){
          debug_out << subset_len[i-subset_left] << " ";
      }
      debug_out << endl;
      END_DEBUG;

      MPI_Send(subset_len,
	       subset_num,
	       MPI_INT,
	       sent_process,
	       M3_TAG,
	       MPI_COMM_WORLD);

      // debug
      TIME_DEBUG_OUT << "slave_process " 
		     << m3_my_rank 
		     << " package sample_buf to send " 
		     << endl;

      // Package sample_buf.
      sample_buf=new Data_Sample[sb_len];
      subset_sample_package(subset_left,
			    subset_right,
			    sample_buf,
			    nb_len);

      // debug
      TIME_DEBUG_OUT << "slave_process " 
		     << m3_my_rank 
		     << " package sample_buf ok! now to send " 
		     << endl;
      TIME_DEBUG_OUT << "slave_process " << m3_my_rank
		     << " need to send " << sb_len
		     << " sample(s) & " << nb_len
		     << " node(s) to process " << sent_process << endl;

      BEGIN_DEBUG;
      for (int i=0;i<sb_len;i++){
	Data_Sample & ds=sample_buf[i];
	TIME_DEBUG_OUT << ds.index << "()"
		       << ds.label << "()"
		       << ds.data_vector_length << endl;
      }
      END_DEBUG;

      // Send sample_buf.
      MPI_Send(sample_buf,
	       sb_len,
	       MPI_Data_Sample,
	       sent_process,
	       M3_TAG,
	       MPI_COMM_WORLD);

      // debug
      TIME_DEBUG_OUT << "slave_process " << m3_my_rank << " package node_buf to send " << endl;

      // Package node_buf.
      node_buf=new Data_Node[nb_len];
      subset_node_package(sb_len,
			  sample_buf,
			  node_buf);

      // debug
      TIME_DEBUG_OUT << "slave_process " 
		     << m3_my_rank 
		     << " package node_buf ok! now to send " 
		     << endl;

       // Send node_buf.
      MPI_Send(node_buf,
	       nb_len,
	       MPI_Data_Node,
	       sent_process,
	       M3_TAG,
	       MPI_COMM_WORLD);

      delete [] subset_len;
      delete [] sample_buf;
      delete [] node_buf;

      // debug
      TIME_DEBUG_OUT << "slave_process " 
		     << m3_my_rank 
		     << " all things has been send & buf has been deleted " 
		     << endl;

    }
  }

  // debug
  TIME_DEBUG_OUT << "slave_process " << m3_my_rank << " train over!" << endl;

}






































// M3_Run

// !!!WARNING:The train process must to new some buf before load block.
// !!!WARNING:But I only new in runing.
// !!!WARNING:There may be some mistake that be handle by parameter.
// !!!WARNING:All "new" and "delete" must be replaced.^_^

M3::M3_Run::M3_Run(){
}

M3::M3_Run::~M3_Run(){
}

// As name
float M3::M3_Run::string_to_float(char * str,
				  int ll,
				  int rr){
  char temp = str[rr];
  str[rr] = 0;
  float res = atof(&(str[ll]));
  str[rr] = temp;
  return res;
}

// As name
int M3::M3_Run::string_to_int(char * str,
			      int ll,
			      int rr){
  char temp = str[rr];
  str[rr] = 0;
  int res = atoi(&(str[ll]));
  str[rr] = temp;
  return res;
}

// Parse input data to our format.
void M3::M3_Run::parse_data(char * rbuf,
			    Data_Sample * dsp){
  int i=0,pri=0;
  int len=0;

  dsp->mlabel_len=1;
  while (rbuf[i]!=' ') {
     i++; 
    dsp->mlabel_len+=(rbuf[i]==',');
  }

  dsp->mlabel=new float[dsp->mlabel_len];
  i=-1;
  for (int k=0;k<dsp->mlabel_len;k++){
      pri=++i;
      while (rbuf[i]!=' ' && rbuf[i]!=',') i++;
      dsp->mlabel[k]=string_to_float(rbuf,pri,i);
  }
  dsp->label=dsp->mlabel[0];

  while (rbuf[i]){
        pri=++i;
        while (rbuf[i] && rbuf[i]!=':') i++;
        if (!rbuf[i]) break;
        dsp->data_vector[len].index=string_to_int(rbuf,pri,i);
        pri=++i;
        while (rbuf[i] && rbuf[i]!=' ') i++;
        dsp->data_vector[len].value=string_to_float(rbuf,pri,i);
        len++;
    }
    dsp->data_vector_length=len;
}

// Unpackage sample_buf as our struct.
void M3::M3_Run::subset_sample_unpackage(int subset_num,
					 int * subset_len,
					 Data_Sample * sample_buf,
					 Data_Sample ** sample_subset,
					 int & node_len){
  node_len=0;
  int i;
  int index=0;
  for (i=0;i<subset_num;i++){
    sample_subset[i]=&sample_buf[index];
    int j;
    for (j=0;j<subset_len[i];j++){
      node_len+=sample_buf[index+j].data_vector_length;

      BEGIN_DEBUG;
      // debug
      TIME_DEBUG_OUT << i << " " << j << "--" << node_len << endl;
      TIME_DEBUG_OUT << sample_buf[index+j].data_vector_length 
		     << " " <<sample_buf[index+j].index
		     << " " <<sample_buf[index+j].label
		     << " " <<(&sample_buf[index+j]) << endl;
      END_DEBUG;

    }
    index+=subset_len[i];
  }
}

// Unpackage sample_buf as our struct.
void M3::M3_Run::subset_node_unpackage(Data_Sample * sample_buf,
				       Data_Node * node_buf,
				       int sb_len){
  int i;
  int index=0;
  for (i=0;i<sb_len;i++){
    sample_buf[i].data_vector=&node_buf[index];
    index+=sample_buf[i].data_vector_length;
  }
}

Classifier * M3::M3_Run::make_classifier(){
  Classifier_Parameter* hossTemPara = M3_Factory::create_parameter(m3_parameter->classifier_parameter_rank);
  int * f_argc;
  char ** f_argv;
  m3_parameter->parse_as_cmd("classifier.config",&f_argc,&f_argv);
  hossTemPara->Parse(*f_argc,f_argv);
  m3_parameter->rm_parse_as_cmd(&f_argc,&f_argv);
  Classifier* cler = M3_Factory::create_classifier(m3_parameter->classifier_rank,hossTemPara);//new libsvm(hossTemPara);
  delete hossTemPara;
  return cler;
}

// Main train block.
void M3::M3_Run::training_train_data(){
  int will_do;
  MPI_Status mpi_status;
  Subset_Info subset_info;
  subset_info.process_rank=m3_my_rank;
  subset_info.save_index=-1;
  while (1){

    // debug
    TIME_DEBUG_OUT << "train_process " << m3_my_rank << " send free" << endl;

    // Send to master that I'm free.
    MPI_Send(&subset_info,
	     1,
	     MPI_Subset_Info,
	     M3_MASTER_RANK,
	     M3_SUBSET_INFO_TAG,
	     MPI_COMM_WORLD); // I'm free!

    // Get control.
    MPI_Recv(&will_do,
	     1,
	     MPI_INT,
	     M3_MASTER_RANK,
	     M3_TAG,
	     MPI_COMM_WORLD,
	     &mpi_status);

    BEGIN_DEBUG;
    // debug
    TIME_DEBUG_OUT << "train_process " << m3_my_rank << " recv CTRL: " << will_do << endl;
    END_DEBUG;

    if (will_do==CTRL_TRAIN_DONE){

      // debug
      TIME_DEBUG_OUT << "train_process " << m3_my_rank << " train done " << endl;

      break;
    }

    else if (will_do==CTRL_TRAIN_CONTINUE){

      int save_index;

      BEGIN_DEBUG;
      // debug
      TIME_DEBUG_OUT << "train_process " 
		     << m3_my_rank 
		     << " recv relate information" << endl;
      END_DEBUG;

      // Get data information: which data from which process.
      MPI_Recv(&subset_info,
	       1,
	       MPI_Subset_Info,
	       M3_MASTER_RANK,
	       M3_TAG,
	       MPI_COMM_WORLD,
	       &mpi_status);
      m_data_process_1=subset_info.process_1;
      m_data_process_2=subset_info.process_2;
      m_data_subset_num_1=subset_info.subset_num_1;
      m_data_subset_num_2=subset_info.subset_num_2;
      save_index=subset_info.save_index;

      // debug
      TIME_DEBUG_OUT << "train_process " << m3_my_rank 
		     << " relate information_1: " 
		     << "from process " << m_data_process_1
		     << " to get " << m_data_subset_num_1 << " subset(s)" << endl;
      TIME_DEBUG_OUT << "train_process " << m3_my_rank 
		     << " relate information_2: " 
		     << "from process " << m_data_process_2
		     << " to get " << m_data_subset_num_2 << " subset(s)" << endl;

      BEGIN_DEBUG;
      // debug
      TIME_DEBUG_OUT << "train_process " << m3_my_rank << " get subset len arr " << endl;
      END_DEBUG;

      // Get every subset length from two data_slave.
      m_sample_subset_len_1=new int[m_data_subset_num_1];
      m_sample_subset_len_2=new int[m_data_subset_num_2];
      MPI_Recv(m_sample_subset_len_1,
	       m_data_subset_num_1,
	       MPI_INT,
	       m_data_process_1,
	       M3_TAG,
	       MPI_COMM_WORLD,
	       &mpi_status);
      MPI_Recv(m_sample_subset_len_2,
	       m_data_subset_num_2,
	       MPI_INT,
	       m_data_process_2,
	       M3_TAG,
	       MPI_COMM_WORLD,
	       &mpi_status);

      // debug
//      BEGIN_DEBUG;
      TIME_DEBUG_OUT << "train_process " << m3_my_rank << " has revc the len arr " << endl;
      
//      END_DEBUG;

      m_sample_len_1=0;
      m_sample_len_2=0;
      for (int i=0;i<m_data_subset_num_1;i++)
	m_sample_len_1+=m_sample_subset_len_1[i];
      for (int i=0;i<m_data_subset_num_2;i++)
	m_sample_len_2+=m_sample_subset_len_2[i];

      // debug
      //BEGIN_DEBUG;    
      TIME_DEBUG_OUT << "train_process " << m3_my_rank << " get sample " 
		     << m_sample_len_1 << " & " << m_sample_len_2<< endl;
      //END_DEBUG;
 
      // Get sample_buf from two data_slave.
      m_sample_buf_1=new Data_Sample[m_sample_len_1];
      m_sample_buf_2=new Data_Sample[m_sample_len_2];
      MPI_Recv(m_sample_buf_1,
	       m_sample_len_1,
	       MPI_Data_Sample,
	       m_data_process_1,
	       M3_TAG,
	       MPI_COMM_WORLD,
	       &mpi_status);
      MPI_Recv(m_sample_buf_2,
	       m_sample_len_2,
	       MPI_Data_Sample,
	       m_data_process_2,
	       M3_TAG,
	       MPI_COMM_WORLD,
	       &mpi_status);
      m_data_label_1=m_sample_buf_1[0].label;
      m_data_label_2=m_sample_buf_2[0].label;

      BEGIN_DEBUG;
      for (int i=0;i<m_sample_len_1;i++){
	Data_Sample & ds=m_sample_buf_1[i];
	TIME_DEBUG_OUT << ds.index << "()"
		       << ds.label << "()"
		       << ds.data_vector_length << endl;
      }
      for (int i=0;i<m_sample_len_2;i++){
	Data_Sample & ds=m_sample_buf_2[i];
	TIME_DEBUG_OUT << ds.index << "()"
		       << ds.label << "()"
		       << ds.data_vector_length << endl;
      }
      END_DEBUG;

      BEGIN_DEBUG;
      // debug
      TIME_DEBUG_OUT <<"train_process " 
		     << m3_my_rank 
		     << " has recv sample!now to unpackage " 
		     << endl;
      END_DEBUG;

      // Unpackage sample_buf.
      m_sample_subset_1=new Data_Sample * [m_data_subset_num_1];
      m_sample_subset_2=new Data_Sample * [m_data_subset_num_2];
      subset_sample_unpackage(m_data_subset_num_1,
			      m_sample_subset_len_1,
			      m_sample_buf_1,
			      m_sample_subset_1,
			      m_node_len_1);
      subset_sample_unpackage(m_data_subset_num_2,
			      m_sample_subset_len_2,
			      m_sample_buf_2,
			      m_sample_subset_2,
			      m_node_len_2);

      BEGIN_DEBUG;
      // debug
      TIME_DEBUG_OUT << "train_process " << m3_my_rank << "has unpackage sample!" << endl;
      TIME_DEBUG_OUT << "train_process " << m3_my_rank 
		     << " from process " << m_data_process_1 
		     << " recv " << m_sample_len_1 
		     << " sample(s) & " << m_node_len_1
		     << " node(s) " << endl;
      TIME_DEBUG_OUT << "train_process " << m3_my_rank 
		     << " from process " << m_data_process_2 
		     << " recv " << m_sample_len_2 
		     << " sample(s) & " << m_node_len_2
		     << " node(s) " << endl;
      END_DEBUG;

      BEGIN_DEBUG;
      // debug
      TIME_DEBUG_OUT << "train_process " << m3_my_rank << " get node " << endl;
      END_DEBUG;

      // Get node_buf from two data_slave.
      m_node_buf_1=new Data_Node[m_node_len_1];
      m_node_buf_2=new Data_Node[m_node_len_2];
      MPI_Recv(m_node_buf_1,
	       m_node_len_1,
	       MPI_Data_Node,
	       m_data_process_1,
	       M3_TAG,
	       MPI_COMM_WORLD,
	       &mpi_status);
      MPI_Recv(m_node_buf_2,
	       m_node_len_2,
	       MPI_Data_Node,
	       m_data_process_2,
	       M3_TAG,
	       MPI_COMM_WORLD,
	       &mpi_status);

      BEGIN_DEBUG;
      // debug
      TIME_DEBUG_OUT << "train_process " 
		     << m3_my_rank 
		     << "has recv node!now unpackage!" << endl;
      END_DEBUG;

      // Unpackage node_buf.
      subset_node_unpackage(m_sample_buf_1,
			    m_node_buf_1,
			    m_sample_len_1);
      subset_node_unpackage(m_sample_buf_2,
			    m_node_buf_2,
			    m_sample_len_2);

      BEGIN_DEBUG;
      // debug
      TIME_DEBUG_OUT << "train_process " << m3_my_rank << "hsa unpackaged! " << endl;
      END_DEBUG;

      BEGIN_DEBUG;
      // debug
      TIME_DEBUG_OUT << "train_process " << m3_my_rank << "begin to train!" << endl;
      END_DEBUG;

      // do something to train
      char save_tmp[20];
      sprintf(save_tmp,"%d",save_index);
      string save_dir=SUBSET_DIR+save_tmp;

      //       // debug
      //       // for test libsvm some data bug
      //       char tmp_name[10];
      //       sprintf(tmp_name,"%d",save_index);
      //       string _debug_test_file_name=(string)"_debug_test_data_"+tmp_name;
      //       ofstream _debug_test_file(_debug_test_file_name.c_str());
      //       // debug
      //       TIME_DEBUG_OUT << "train_process " << m3_my_rank << " check data: " << endl;
      //       for (int i=0;i<m_data_subset_num_1;i++){
      // 	TIME_DEBUG_OUT << "subset " << i <<" frome process " << m_data_process_1
      // 		       << " has " << m_sample_subset_len_1[i] << " sample(s)" << endl;
      // 	for (int j=0;j<m_sample_subset_len_1[i];j++){
      // 	  Data_Sample & ds=m_sample_subset_1[i][j];
      // 	  TIME_DEBUG_OUT << "----@" << j << "   "
      // 			 << "SAMPLE["<< ds.index
      // 			 << "," << ds.label << "]: ";
      // 	  _debug_test_file << ds.label << " ";
      // 	  Data_Node * dn=ds.data_vector;
      // 	  for (int k=0;k<ds.data_vector_length;k++){
      // 	    debug_out << "(" << dn[k].index << "," << dn[k].value << ")";
      // 	    _debug_test_file << dn[k].index << ":" << dn[k].value << " ";
      // 	  }
      // 	  _debug_test_file << endl;
      // 	  debug_out << endl;
      // 	}
      //       }
      //       // debug
      //       TIME_DEBUG_OUT << "train_process " << m3_my_rank << " check data: " << endl;
      //       for (int i=0;i<m_data_subset_num_2;i++){
      // 	TIME_DEBUG_OUT << "subset " << i <<" frome process " << m_data_process_2
      // 		       << " has " << m_sample_subset_len_2[i] << " sample(s)" << endl;
      // 	for (int j=0;j<m_sample_subset_len_2[i];j++){
      // 	  Data_Sample & ds=m_sample_subset_2[i][j];
      // 	  TIME_DEBUG_OUT << "----@" << j << "   "
      // 			 << "SAMPLE["<< ds.index
      // 			 << "," << ds.label << "]: ";
      // 	  _debug_test_file << ds.label << " ";
      // 	  Data_Node * dn=ds.data_vector;
      // 	  for (int k=0;k<ds.data_vector_length;k++){
      // 	    debug_out << "(" << dn[k].index << "," << dn[k].value << ")";
      // 	    _debug_test_file << dn[k].index << ":" << dn[k].value << " ";
      // 	  }
      // 	  _debug_test_file << endl;
      // 	  debug_out << endl;
      // 	}
      //       }
      //       // debug
      //       _debug_test_file.close();

      Classifier * tempClass=make_classifier();

      double dtm_1=MPI_Wtime();

      subset_info.subset_memory=tempClass->train(m_sample_subset_1,m_sample_subset_2,
						 m_data_subset_num_1,m_data_subset_num_2,
						 m_sample_subset_len_1,m_sample_subset_len_2,
						 save_dir.c_str());

      double dtm_2=MPI_Wtime();

      link_train_time+=dtm_2-dtm_1;

      delete tempClass;
      // do something to handle the memory which is the space to store

      // debug
      TIME_DEBUG_OUT << "train_process " << m3_my_rank << "this times is train ok!" << endl;

      delete [] m_sample_subset_1;
      delete [] m_sample_subset_2;
      delete [] m_sample_subset_len_1;
      delete [] m_sample_subset_len_2;
      delete [] m_sample_buf_1;
      delete [] m_sample_buf_2;
      delete [] m_node_buf_1;
      delete [] m_node_buf_2;
    }
  }

  // debug
  TIME_DEBUG_OUT << "train_process " << m3_my_rank << " train over! " << endl;

}

void M3::M3_Run::score_test_data_full(vector<Classifier*> & classifier,
				      vector<Subset_Info> & subset_info,
				      vector<ofstream*> & middle_score,
				      Data_Sample * sample_buf){

  // debug
  TIME_DEBUG_OUT << "run_process " << m3_my_rank << " score full" << endl;
      
  for (int i=0;i<subset_info.size();i++){
    int len=subset_info[i].subset_num_1*subset_info[i].subset_num_2;
    double * score;

    double dtm_1=MPI_Wtime();

    // do somthing to get this score
    // like: (*(classifier[i])).classifier(sample_buf,score);
    ////////////////////////////////////////added by hoss/////
    score = classifier[i]->predict(sample_buf);
    //////////////////////////////////////////////////////////

    double dtm_2=MPI_Wtime();

    link_predict_time+=dtm_2-dtm_1;
    
    (*(middle_score[i])) << sample_buf->index << " ";
    
    for (int j=0;j<len;j++)
      (*(middle_score[i])) << score[j] << " ";
    (*(middle_score[i])) << endl;
    
    delete [] score;
  }
}

void M3::M3_Run::score_test_data_min_vector(vector<Classifier*> & classifier,
					    vector<Subset_Info> & subset_info,
					    vector<ofstream*> & middle_score,
					    Data_Sample * sample_buf){
  // debug
  TIME_DEBUG_OUT << "run_process " << m3_my_rank << " score min_vector" << endl;

  for (int i=0;i<subset_info.size();i++){
    int len=subset_info[i].subset_num_1*subset_info[i].subset_num_2;
    double * score;

    double dtm_1=MPI_Wtime();

    // do somthing to get this score
    // like: (*(classifier[i])).classifier(sample_buf,score);
    ////////////////////////////////////////added by hoss/////
    score = classifier[i]->predict(sample_buf);
    //////////////////////////////////////////////////////////

    double dtm_2=MPI_Wtime();

    link_predict_time+=dtm_2-dtm_1;
    
    (*(middle_score[i])) << sample_buf->index << " ";
    
    for (int j=0;j<subset_info[i].subset_num_1;j++){
      double sc=0;
      bool flag=true;
      int tmp=j*subset_info[i].subset_num_2;
      for (int k=0;k<subset_info[i].subset_num_2;k++){
	if (flag){
	  flag=false;
	  sc=score[tmp+k];
	}
	else sc=min(sc,score[tmp+k]);
      }

      (*(middle_score[i])) << sc << " ";
    }
    (*(middle_score[i])) << endl;
    delete [] score;
  }
}

void M3::M3_Run::only_classify_test_data_serial(MPI_Status & mpi_status,
						vector<Classifier*> & classifier,
						vector<Subset_Info> & subset_info,
						vector<ofstream*> & middle_score,
						Data_Sample * sample_buf,
						Data_Node * node_buf){

  // debug
  TIME_DEBUG_OUT << "test_process " << m3_my_rank << " get test data! " << endl;

  int sp_index,nd_index;

  MPI_Recv(&sp_index,
	   1,
	   MPI_INT,
	   M3_MASTER_RANK,
	   M3_TAG,
	   MPI_COMM_WORLD,
	   &mpi_status);
  MPI_Recv(sample_buf,
	   TEST_SAMPLE_BUF_SIZE,
	   MPI_Data_Sample,
	   M3_MASTER_RANK,
	   M3_TAG,
	   MPI_COMM_WORLD,
	   &mpi_status);
  MPI_Recv(node_buf,
	   TEST_NODE_BUF_SIZE,
	   MPI_Data_Node,
	   M3_MASTER_RANK,
	   M3_TAG,
	   MPI_COMM_WORLD,
	   &mpi_status);

  nd_index=0;
  for (int i=0;i<sp_index;i++){
    sample_buf[i].data_vector=&node_buf[nd_index];
    nd_index+=sample_buf[i].data_vector_length;
  }

//   // debug
//   TIME_DEBUG_OUT << "test_process " << m3_my_rank << " has recved data num:" 
// 		 << sp_index << endl;
//   TIME_DEBUG_OUT << "They are: " << endl;
//   for (int i=0;i<sp_index;i++){
//     TIME_DEBUG_OUT << sample_buf[i].index << " : ";
//     for (int j=0;j<sample_buf[i].data_vector_length;j++)
//       debug_out << "(" << sample_buf[i].data_vector[j].index 
// 		<< "," << sample_buf[i].data_vector[j].value
// 		<< ")";
//     debug_out << endl;
//   }

  for (int k=0;k<sp_index;k++){
    if (m3_parameter->m3_min_max_mode==0){
      score_test_data_full(classifier,
			   subset_info,
			   middle_score,
			   &sample_buf[k]);
    }
    else if (m3_parameter->m3_min_max_mode==1){
      score_test_data_min_vector(classifier,
				 subset_info,
				 middle_score,
				 &sample_buf[k]);
    }
  }

  // debug
  TIME_DEBUG_OUT << "test_process " << m3_my_rank << " has scored over " << endl;
}

void M3::M3_Run::only_classify_test_data_parallel(string & file_name,
						  vector<bool> & test_flag,
						  vector<Classifier*> & classifier,
						  vector<Subset_Info> & subset_info,
						  vector<ofstream*> & middle_score,
						  Data_Sample * sample_buf,
						  Data_Node * node_buf){

  // debug
  TIME_DEBUG_OUT << "test_process " << m3_my_rank << " read test data! " << endl;

  int index=0;
  char * read_buf=new char[READ_BUF_SIZE];
  ifstream file_in(file_name.c_str());

  sample_buf->data_vector=node_buf;

  while (true){

    if (index>=test_flag.size()){

      // debug
      TIME_DEBUG_OUT << "test_process " << m3_my_rank << " test over at this time" << endl;

      break;
    }

    // running flag
    if (index==test_flag.size()-1 || !(index%100))
      cout << "test_process " << m3_my_rank << " has scored "  << index 
	   << " over ; @ " << 100.0*(1.0+index)/test_flag.size() << " %" << endl;


    if (!test_flag[index++])
      continue;

    file_in.getline(read_buf,
		    READ_BUF_SIZE);

    parse_data(read_buf,
	       sample_buf);
    sample_buf->index=index-1;

//     //    debug
//     TIME_DEBUG_OUT << "test_process " << m3_my_rank << " has parsed the test data:" << endl;
//     TIME_DEBUG_OUT << sample_buf->index << " : ";
//     for (int j=0;j<sample_buf->data_vector_length;j++)
//       debug_out << "(" << sample_buf->data_vector[j].index 
//     		<< "," << sample_buf->data_vector[j].value
// 		<< ")";
//     debug_out << endl;
	
    if (m3_parameter->m3_min_max_mode==0){
      score_test_data_full(classifier,
			   subset_info,
			   middle_score,
			   sample_buf);
    }
    else if (m3_parameter->m3_min_max_mode==1){
      score_test_data_min_vector(classifier,
				 subset_info,
				 middle_score,
				 sample_buf);
    }
  }
  file_in.close();

  // debug
  TIME_DEBUG_OUT << "test_process " << m3_my_rank << " has scored over " << endl;

  delete [] read_buf;
}

void M3::M3_Run::classify_test_data_nonpruning(){

  int will_do;
  MPI_Status mpi_status;
  vector<Classifier*> classifier;
  vector<Subset_Info> subset_info;
  vector<ofstream*> middle_score;

  classifier.clear();
  subset_info.clear();
  middle_score.clear();

  while (true){
    MPI_Recv(&will_do,
	     1,
	     MPI_INT,
	     M3_MASTER_RANK,
	     M3_TAG,
	     MPI_COMM_WORLD,
	     &mpi_status);

    // debug
    TIME_DEBUG_OUT << "test_process " << m3_my_rank << " recv CTRL: " << will_do << endl;

    if (will_do==CTRL_TEST_DONE){

      // debug
      TIME_DEBUG_OUT << "test_process " << m3_my_rank << " test over! " << endl;

      break;
    }

    else if (will_do==CTRL_TEST_CLEAR){

      // debug
      TIME_DEBUG_OUT << "test_process " << m3_my_rank << " begin to clear memory! " << endl;

      for (int i=0;i<classifier.size();i++)
	delete classifier[i];
      classifier.clear();

      for (int i=0;i<middle_score.size();i++){
	(*(middle_score[i])).close();
	delete middle_score[i];
      }
      middle_score.clear();

      subset_info.clear();

      // debug
      TIME_DEBUG_OUT << "test_process " << m3_my_rank << " clear memory! " << endl;

    }

    else if (will_do==CTRL_LOAD_SUBSET){

      Subset_Info si;
      si.process_rank=m3_my_rank;
      while (true) {

	// debug
	TIME_DEBUG_OUT << "test_process " << m3_my_rank << " send free! " << endl;

	MPI_Send(&si,
		 1,
		 MPI_Subset_Info,
		 M3_MASTER_RANK,
		 M3_SUBSET_INFO_TAG,
		 MPI_COMM_WORLD);
	MPI_Recv(&si,
		 1,
		 MPI_Subset_Info,
		 M3_MASTER_RANK,
		 M3_TAG,
		 MPI_COMM_WORLD,
		 &mpi_status);

	// debug
	TIME_DEBUG_OUT << "test_process " 
		       << m3_my_rank << " get subset information:" << endl;
	TIME_DEBUG_OUT << "subset " << si.save_index << " : "
		       << si.label_1 << " "
		       << si.label_2 << " "
		       << si.subset_num_1 << " "
		       << si.subset_num_2 << " "
		       << si.subset_memory << endl;
	
	if (si.save_index<0){

	  // debug
	  TIME_DEBUG_OUT << "test_process " 
			 << m3_my_rank 
			 << " load subset over! " << endl;

	  break;
	}

	bool memory_flag=true;
	// do something to handle memory alloc
	// like:
	// Classifier * cler=new ...
	// memory_flag=cler->look at memory(si)
	// if (memory_flag)
	// classifier.push_back(cler);
	// else delete cler;
	////////////////////////////////////added by hoss(no concern memory lack)/////////////
	//libsvm_parameter hossTemPara;	
	Classifier *cler=make_classifier();
	char hossStr[20];
	sprintf(hossStr,"%d",si.save_index);
	string save_dir = SUBSET_DIR + hossStr;
	//////////////////////////////////////////////////////////////////////////////////////

	// debug
	TIME_DEBUG_OUT << "test_process " << m3_my_rank 
		       << " has done to test load model" << endl;

	if (memory_flag){
	  subset_info.push_back(si);

	  char tmp[20];
	  sprintf(tmp,"%d",si.save_index);
	  string file_name=SCORE_DIR+tmp;
	  ofstream * out_tmp=new ofstream(file_name.c_str());
	  middle_score.push_back(out_tmp);

	  MPI_Send(&CTRL_MEMORY_ENOUGH,
		   1,
		   MPI_INT,
		   M3_MASTER_RANK,
		   M3_TAG,
		   MPI_COMM_WORLD);

	  // debug
	  TIME_DEBUG_OUT << "slave_process " << m3_my_rank << " load subset" << endl;

	  cler->load_model(save_dir.c_str());
	  classifier.push_back(cler);
	}
	else {
	  MPI_Send(&CTRL_MEMORY_SCARCITY,
		   1,
		   MPI_INT,
		   M3_MASTER_RANK,
		   M3_TAG,
		   MPI_COMM_WORLD);
	  break;
	}

	// debug
	TIME_DEBUG_OUT << "test_process " << m3_my_rank 
		       << " response to master the memory situation" << endl;

      }
    }

    else if (will_do==CTRL_CLASSIFY_DATA){

      if (m3_parameter->m3_classify_mode==0){
	Data_Sample * sample_buf=new Data_Sample;
	Data_Node * node_buf=new Data_Node[TEST_NODE_BUF_SIZE];
	string file_name=m3_parameter->classify_data;
	vector<bool> test_flag;
	test_flag.clear();
	size_t len = M3::testLen;
	for (int i=0;i<len;i++)
	  test_flag.push_back(true);
	only_classify_test_data_parallel(file_name,
					 test_flag,
					 classifier,
					 subset_info,
					 middle_score,
					 sample_buf,
					 node_buf);
	delete sample_buf;
	delete [] node_buf;

      }
      else if (m3_parameter->m3_classify_mode==1){
	Data_Sample * sample_buf=new Data_Sample[TEST_SAMPLE_BUF_SIZE];
	Data_Node * node_buf=new Data_Node[TEST_NODE_BUF_SIZE];
	only_classify_test_data_serial(mpi_status,
				       classifier,
				       subset_info,
				       middle_score,
				       sample_buf,
				       node_buf);
	delete [] sample_buf;
	delete [] node_buf;
      }
      
    }
  }

  vector<Classifier*>::iterator itc;
  vector<ofstream*>::iterator itm;

  for (itc=classifier.begin();itc!=classifier.end();itc++)
    delete (*itc);
  classifier.clear();

  for (itm=middle_score.begin();itm!=middle_score.end();itm++){
    (*(*itm)).close();
    delete (*itm);
  }
  middle_score.clear();
}


void M3::M3_Run::make_level_info_classifier(){
  for (int i=0;i<m_level_info.size();i++){

    Level_Info * li=m_level_info[i];
    li->classifier=new Classifier*[li->length];

    for (int j=0;j<li->length;j++){
      Subset_Info si=li->si[j];

      Classifier *cler=make_classifier();

      li->classifier[j]=cler;
      char hossStr[20];
      sprintf(hossStr,"%d",si.save_index);
      string save_dir = SUBSET_DIR + hossStr;

      cler->load_model(save_dir.c_str());

    }
  }
}

int M3::M3_Run::get_level_info_pruning(){
  int quest[2];
  MPI_Status mpi_status;
  MPI_Recv(quest,
	   2,
	   MPI_INT,
	   M3_MASTER_RANK,
	   M3_TAG,
	   MPI_COMM_WORLD,
	   &mpi_status);

  // debug
  TIME_DEBUG_OUT << "pipe info head: " << quest[0] << " " << quest[1] << endl;

  if (quest[0]<0){
    if (quest[1]>0)
      return -1;		// the last 
    else return 0;		// over
  }

  int pre_post[2];

  Level_Info * li=new Level_Info(quest[0],quest[1]);
  MPI_Recv(pre_post,
	   2,
	   MPI_INT,
	   M3_MASTER_RANK,
	   M3_TAG,
	   MPI_COMM_WORLD,
	   &mpi_status);
  li->pre_process=pre_post[0];
  li->post_process=pre_post[1];

  MPI_Recv(li->x,
	   quest[1],
	   MPI_INT,
	   M3_MASTER_RANK,
	   M3_TAG,
	   MPI_COMM_WORLD,
	   &mpi_status);
  MPI_Recv(li->y,
	   quest[1],
	   MPI_INT,
	   M3_MASTER_RANK,
	   M3_TAG,
	   MPI_COMM_WORLD,
	   &mpi_status);
  MPI_Recv(li->si,
	   quest[1],
	   MPI_Subset_Info,
	   M3_MASTER_RANK,
	   M3_TAG,
	   MPI_COMM_WORLD,
	   &mpi_status);

  // debug
  TIME_DEBUG_OUT << "run_process " << m3_my_rank 
		 << " has receive the level " << li->level
		 << " with " << li->length << " subset " << endl;
  for (int i=0;i<li->length;i++)
    TIME_DEBUG_OUT << "subset " << li->si[i].save_index 
		   << " at "  << li->x[i] << "," << li->y[i] << endl;

  m_level_info.push_back(li);

  return 1;			// continue
}

void M3::M3_Run::pipe_get_mlisp(int len,
				Level_Info * li){
  Mid_Level_Info_Sematric_Pruning & ml=m_mlisp[m_mlisp_now];
  MPI_Status mpi_status;

  MPI_Recv(ml.x,
	   len,
	   MPI_INT,
	   li->pre_process,
	   M3_TAG,
	   MPI_COMM_WORLD,
	   &mpi_status);
  MPI_Recv(ml.y,
	   len,
	   MPI_INT,
	   li->pre_process,
	   M3_TAG,
	   MPI_COMM_WORLD,
	   &mpi_status);
  MPI_Recv(ml.X,
	   len,
	   MPI_INT,
	   li->pre_process,
	   M3_TAG,
	   MPI_COMM_WORLD,
	   &mpi_status);
  MPI_Recv(ml.Y,
	   len,
	   MPI_INT,
	   li->pre_process,
	   M3_TAG,
	   MPI_COMM_WORLD,
	   &mpi_status);
  MPI_Recv(ml.dirct,
	   len,
	   MPI_FLOAT,
	   li->pre_process,
	   M3_TAG,
	   MPI_COMM_WORLD,
	   &mpi_status);
}

void M3::M3_Run::pipe_give_mlisp(int len,
				 Level_Info * li){
  Mid_Level_Info_Sematric_Pruning & ml=m_mlisp[m_mlisp_now];

  MPI_Send(ml.x,
	   len,
	   MPI_INT,
	   li->post_process,
	   M3_TAG,
	   MPI_COMM_WORLD);
  MPI_Send(ml.y,
	   len,
	   MPI_INT,
	   li->post_process,
	   M3_TAG,
	   MPI_COMM_WORLD);
  MPI_Send(ml.X,
	   len,
	   MPI_INT,
	   li->post_process,
	   M3_TAG,
	   MPI_COMM_WORLD);
  MPI_Send(ml.Y,
	   len,
	   MPI_INT,
	   li->post_process,
	   M3_TAG,
	   MPI_COMM_WORLD);
  MPI_Send(ml.dirct,
	   len,
	   MPI_FLOAT,
	   li->post_process,
	   M3_TAG,
	   MPI_COMM_WORLD);
}

void M3::M3_Run::pipe_get_mliap(int len,
				Level_Info * li){
  MPI_Status mpi_status;

  MPI_Recv(m_mliap.min_vector,
	   m_mliap.len,
	   MPI_FLOAT,
	   li->pre_process,
	   M3_TAG,
	   MPI_COMM_WORLD,
	   &mpi_status);
}

void M3::M3_Run::pipe_give_mliap(int len,
				 Level_Info * li){
  MPI_Send(m_mliap.min_vector,
	   m_mliap.len,
	   MPI_FLOAT,
	   li->post_process,
	   M3_TAG,
	   MPI_COMM_WORLD);
}


void M3::M3_Run::pipe_level_classify_sematric_pruning(int mlevel,
						      Data_Sample * sample_buf,
						      int sb_len){
  int nx,ny,nX,nY;
  float nd;

  for (int i=0;i<sb_len;i++){
    if (m_level_info[mlevel]->level==0){
      nx=ny=nX=nY=0;
    }
    else {
      nx=m_mlisp[m_mlisp_now].getn_x();
      ny=m_mlisp[m_mlisp_now].getn_y();
      nX=m_mlisp[m_mlisp_now].getn_X();
      nY=m_mlisp[m_mlisp_now].getn_Y();
      nd=m_mlisp[m_mlisp_now].getn_d();
    }

    int cid=m_level_info[mlevel]->classifier_id(nX,nY);

//     // debug
//     TIME_DEBUG_OUT << m_mlisp[m_mlisp_now].get_X() << " "
// 		   << m_mlisp[m_mlisp_now].get_Y() << " "
// 		   << m_mlisp[m_mlisp_now].get_x() << " "
// 		   << m_mlisp[m_mlisp_now].get_y() << " "
// 		   << m_mlisp[m_mlisp_now].get_d() << " " << endl;

//     // debug
//     TIME_DEBUG_OUT << mlevel << " " << cid << endl;

//     // debug
//     TIME_DEBUG_OUT << "run_process " << m3_my_rank
// 		   << " test sample " << sample_buf[i].index
// 		   << " at (" << nX << "," << nY << ")"
// 		   << " in from " << nx << "-" << ny 
// 		   << " & dirc " << nd << endl;

    double dtm_1=MPI_Wtime();

    if (cid!=-1)
      m_level_info[mlevel]->classifier[cid]->predict(&sample_buf[i],
						    nx,
						    ny,
						    nd);

    double dtm_2=MPI_Wtime();

    link_predict_time+=dtm_2-dtm_1;

//     // debug
//     TIME_DEBUG_OUT << "run_process " << m3_my_rank
// 		   << " get result " << sample_buf[i].index
// 		   << " at (" << nX << "," << nY << ")"
// 		   << " out from " << nx << "-" << ny 
// 		   << " at dirc " << nd << endl;

    m_mlisp[m_mlisp_next].put(nx,ny,nX,nY,nd);

    m_mlisp[m_mlisp_next].updata();
    m_mlisp[m_mlisp_now].updata();
  }
}

void M3::M3_Run::pipe_level_classify_asematric_pruning(int mlevel,
						       Data_Sample * sample_buf,
						       int sb_len){
  for (int i=0;i<sb_len;i++){
    int start=m_mliap.address(i);
    int offset=0;

    Level_Info * li=m_level_info[mlevel];
    for (int j=0;j<li->length;j++){
      float * min_v=&(m_mliap.min_vector[start+offset]);

      double dtm_1=MPI_Wtime();

      li->classifier[j]->predict(&sample_buf[i],
				 min_v);

      double dtm_2=MPI_Wtime();

      link_predict_time+=dtm_2-dtm_1;

      offset+=li->si[j].subset_num_1;
    }

    // debug
    BEGIN_DEBUG;
    TIME_DEBUG_OUT << "sample " << sample_buf[i].index << " min vector: " << endl;
    offset=0;
    for (int j=0;j<li->length;j++){
      float * min_v=&(m_mliap.min_vector[start+offset]);
      TIME_DEBUG_OUT;
      for (int k=0;k<li->si[j].subset_num_1;k++)
	debug_out << min_v[k] << " ";      
      debug_out << endl;
      offset+=li->si[j].subset_num_1;
    }
    END_DEBUG;

  }
}

void M3::M3_Run::pipe_get_sample_buf_from_file(ifstream & file_in,
					       char * read_buf,
					       Data_Sample * sample_buf,
					       Data_Node * node_buf,
					       int & sb_len,
					       int & nb_len,
					       int & index,
					       vector<bool> & test_flag){
  while (true){
    
    if (index>=test_flag.size()){
      
      // debug
      TIME_DEBUG_OUT << "test_process " << m3_my_rank << " read test data over at this time" << endl;
      
      break;
    }
    
    // running flag
    if (index==test_flag.size()-1 || !(index%(TEST_SAMPLE_BUF_SIZE*100)))
      cout << "test_process " << m3_my_rank << " has scored "  << index 
	   << " over ; @ " << 100.0*(1.0+index)/test_flag.size() << " %" << endl;
    
    if (!test_flag[index++])
      continue;
    
    file_in.getline(read_buf,
		    READ_BUF_SIZE);
    
    sample_buf[sb_len].data_vector=&node_buf[nb_len];
    parse_data(read_buf,
	       &sample_buf[sb_len]);
    sample_buf[sb_len].index=index-1;
    
    nb_len+=sample_buf[sb_len].data_vector_length;
    sb_len++;
    
    if (sb_len>=TEST_SAMPLE_BUF_SIZE)
      break;
  }
  
}

void M3::M3_Run::make_all_test_sample_buf(string file_name,
					  vector<bool> test_flag){

  int len=0;
  for (int i=0;i<test_flag.size();i++)
    len+=test_flag[i];

  m_atsb.len=len;
  m_atsb.buf=new Data_Sample[len];

  ifstream file_in(file_name.c_str());

  char * read_buf=new char[READ_BUF_SIZE];

  int index=0;
  int ptr=0;

  memset(read_buf,0,sizeof(char)*READ_BUF_SIZE);
  while (file_in.getline(read_buf,
			 READ_BUF_SIZE)){

    if (!test_flag[index++]){
      memset(read_buf,0,sizeof(char)*READ_BUF_SIZE);
      continue;
    }

    len=0;
    for (int i=0;read_buf[i];i++)
      len+=(read_buf[i]==':');

    m_atsb.buf[ptr].data_vector=new Data_Node[len];

    parse_data(read_buf,
	       &m_atsb.buf[ptr]);

    m_atsb.buf[ptr].index=index-1;

    ptr++;
    memset(read_buf,0,sizeof(char)*READ_BUF_SIZE);
  }

  delete [] read_buf;

  file_in.close();
}

void M3::M3_Run::make_all_test_sample_buf(ifstream & file_in,
					  vector<bool> test_flag,
					  int & index){

  int ts_size=TEST_AND_SCORE_ALL_DATA_BUF_SIZE;
  int l_len=0,len;
  for (int i=index;i<test_flag.size();i++){
    l_len+=test_flag[i];
    if (l_len>=ts_size)
      break;
  }

  // debug
  BEGIN_DEBUG;
  TIME_DEBUG_OUT << "make test_flag done " << endl;
  END_DEBUG;

  m_atsb.len=l_len;
  m_atsb.buf=new Data_Sample[l_len];

  char * read_buf=new char[READ_BUF_SIZE];

  int ptr=0;

  memset(read_buf,0,sizeof(char)*READ_BUF_SIZE);
  while (file_in.getline(read_buf
			 ,READ_BUF_SIZE)){

    if (!test_flag[index++]){
      memset(read_buf,0,sizeof(char)*READ_BUF_SIZE);
      continue;
    }

    len=0;
    for (int i=0;read_buf[i];i++)
      len+=(read_buf[i]==':');

    m_atsb.buf[ptr].data_vector=new Data_Node[len];

    parse_data(read_buf,
	       &m_atsb.buf[ptr]);

    m_atsb.buf[ptr].index=index-1;

    ptr++;
    memset(read_buf,0,sizeof(char)*READ_BUF_SIZE);

    if (ptr>=l_len)
      break;
  }

  // debug
  BEGIN_DEBUG;
  TIME_DEBUG_OUT << "load done" << endl;
  END_DEBUG;

  delete [] read_buf;
}

Data_Sample * M3::M3_Run::pipe_get_sample_buf_from_all_data_buf(int & sb_len){
  sb_len=0;
  if (m_atsb.ptr>=m_atsb.len) {

    // debug
    TIME_DEBUG_OUT << "test_process " << m3_my_rank << " read test data over at this time" << endl;

    return NULL;
  }

  sb_len=min(TEST_SAMPLE_BUF_SIZE,m_atsb.len-m_atsb.ptr);

  int ptr=m_atsb.ptr;

  m_atsb.ptr+=sb_len;

  // running flag
  if (m_atsb.ptr==m_atsb.len || !(m_atsb.ptr%(TEST_SAMPLE_BUF_SIZE*100)))
    cout << "test_process " << m3_my_rank << " begin to score "  << m_atsb.ptr
	 << " over ; @ " << 100.0*(m_atsb.ptr)/m_atsb.len << " %" << endl;

  return &m_atsb.buf[ptr];
}

void M3::M3_Run::pipe_classify_test_data_sematric_pruning(string file_name,
							  vector<bool> test_flag,
							  bool mid_score_save){

  ofstream fout;
  if (mid_score_save){
    char name[100];
    sprintf(name,"%f_%f",m_level_info[0]->si[0].label_1,m_level_info[0]->si[0].label_2);
    fout.open((SCORE_DIR+name).c_str());
  }

  // debug
  TIME_DEBUG_OUT << "test_process " << m3_my_rank << "pipe read test data! " << endl;

  int index=0;
  char * read_buf;
  ifstream file_in;
  Data_Sample * sample_buf;
  Data_Node * node_buf;
  int sb_len,nb_len;


  if (m3_parameter->m3_pruning_speed_mode==0){
    read_buf=new char[READ_BUF_SIZE];
    sample_buf=new Data_Sample[TEST_SAMPLE_BUF_SIZE];
    node_buf=new Data_Node[TEST_NODE_BUF_SIZE];
    file_in.open(file_name.c_str());
  }
    
  m_mlisp[0].new_mlisp(TEST_SAMPLE_BUF_SIZE);
  m_mlisp[1].new_mlisp(TEST_SAMPLE_BUF_SIZE);

  while (true){
    
    sb_len=0;
    nb_len=0;
    m_mlisp_now=0;
    m_mlisp_next=1;

    if (m3_parameter->m3_pruning_speed_mode==0){
      pipe_get_sample_buf_from_file(file_in,
				    read_buf,
				    sample_buf,
				    node_buf,
				    sb_len,
				    nb_len,
				    index,
				    test_flag);
    }
    else if (m3_parameter->m3_pruning_speed_mode==1){
      sample_buf=pipe_get_sample_buf_from_all_data_buf(sb_len);
    }

    if (sb_len==0){

      // debug
      TIME_DEBUG_OUT << "test_process " << m3_my_rank << " test over at this time" << endl;

      break;
    }

    BEGIN_DEBUG;
    // debug
    TIME_DEBUG_OUT << "test_process " << m3_my_rank << "recv the pre pipe level info" << endl;
    END_DEBUG;

    if (m_level_info[0]->level!=0)
      pipe_get_mlisp(sb_len,
		     m_level_info[0]);

    BEGIN_DEBUG;
    // debug
    TIME_DEBUG_OUT << "test_process " << m3_my_rank << "do the level test " << endl;
    END_DEBUG;

    for (int i=0;i<m_level_info.size();i++){
      m_mlisp[m_mlisp_now].init();
      m_mlisp[m_mlisp_next].init();

      pipe_level_classify_sematric_pruning(i,
					   sample_buf,
					   sb_len);
      
      m_mlisp_now=1-m_mlisp_now;
      m_mlisp_next=1-m_mlisp_next;
    }

    BEGIN_DEBUG;
    // debug
    TIME_DEBUG_OUT << "test_process " << m3_my_rank << "send post pipe level info" << endl;
    END_DEBUG;

    if (mid_score_save){
      for (int i=0;i<sb_len;i++){
	float mv=m_mlisp[m_mlisp_now].dirct[i];
	if (m3_parameter->m3_pruning_combine_score!=1){
	  fout << sample_buf[i].index << " " << mv << endl;
	}
	if (m3_parameter->m3_pruning_combine_score!=0){
	  m_pipe_pool.push(mv);
	}
      }
    }
    else pipe_give_mlisp(sb_len,
			 m_level_info[m_level_info.size()-1]);
    
  }

  // debug
  TIME_DEBUG_OUT << "test_process " << m3_my_rank << " has scored over " << endl;

  if (mid_score_save)
    fout.close();

  m_mlisp[0].del_mlisp();
  m_mlisp[1].del_mlisp();

  if (m3_parameter->m3_pruning_speed_mode==0){
    file_in.close();
    delete [] read_buf;
    delete [] sample_buf;
    delete [] node_buf;
  }

}


void M3::M3_Run::pipe_classify_test_data_asematric_pruning(string file_name,
							   vector<bool> test_flag,
							   bool mid_score_save){
  ofstream fout;
  if (mid_score_save){
    char name[100];
    if (!m3_parameter->m3_multilabel)
        sprintf(name,"%f_%f",m_level_info[0]->si[0].label_1,m_level_info[0]->si[0].label_2);
    else 
        sprintf(name,"%f_rest",m_level_info[0]->si[0].label_1);
    fout.open((SCORE_DIR+name).c_str());
  }

  // debug
  TIME_DEBUG_OUT << "test_process " << m3_my_rank << "pipe read test data! " << endl;

  int index=0;
  char * read_buf;
  ifstream file_in;
  Data_Sample * sample_buf;
  Data_Node * node_buf;
  int sb_len,nb_len;

  if (m3_parameter->m3_pruning_speed_mode==0){
    read_buf=new char[READ_BUF_SIZE];
    sample_buf=new Data_Sample[TEST_SAMPLE_BUF_SIZE];
    node_buf=new Data_Node[TEST_NODE_BUF_SIZE];
    file_in.open(file_name.c_str());
  }

  int line_num=0;
  for (int i=0;i<m_level_info[0]->length;i++)
    line_num+=m_level_info[0]->si[i].subset_num_1; // calc the line number in one row

  m_mliap.new_mliap(TEST_SAMPLE_BUF_SIZE,line_num);

  while (true){

    sb_len=0;
    nb_len=0;

    if (m3_parameter->m3_pruning_speed_mode==0){
      pipe_get_sample_buf_from_file(file_in,
				    read_buf,
				    sample_buf,
				    node_buf,
				    sb_len,
				    nb_len,
				    index,
				    test_flag);
    }
    else if (m3_parameter->m3_pruning_speed_mode==1){
      sample_buf=pipe_get_sample_buf_from_all_data_buf(sb_len);
    }


    if (sb_len==0){

      // debug
      TIME_DEBUG_OUT << "test_process " << m3_my_rank << " test over at this time" << endl;

      break;
    }

    BEGIN_DEBUG;
    // debug
    TIME_DEBUG_OUT << "test_process " << m3_my_rank << "recv the pre pipe level info" << endl;
    END_DEBUG;

    if (m_level_info[0]->level!=0)
      pipe_get_mliap(sb_len,
		     m_level_info[0]);
    else m_mliap.clear();

    BEGIN_DEBUG;
    // debug
    TIME_DEBUG_OUT << "test_process " << m3_my_rank << "do the level test " << endl;
    END_DEBUG;

    for (int i=0;i<m_level_info.size();i++){
      pipe_level_classify_asematric_pruning(i,
					    sample_buf,
					    sb_len);
    }

    BEGIN_DEBUG;
    // debug
    TIME_DEBUG_OUT << "test_process " << m3_my_rank << "send post pipe level info" << endl;
    END_DEBUG;

    if (mid_score_save){

      for (int i=0;i<sb_len;i++){
	float mv=m_mliap.max_vector(i);
	if (m3_parameter->m3_pruning_combine_score!=1){
	  fout << sample_buf[i].index << " " << mv << endl;
	}
	if (m3_parameter->m3_pruning_combine_score!=0){
	  m_pipe_pool.push(mv);
	}
      }
    }
    else pipe_give_mliap(sb_len,
			 m_level_info[m_level_info.size()-1]);
    
  }

  // debug
  TIME_DEBUG_OUT << "test_process " << m3_my_rank << " has scored over " << endl;

  if (mid_score_save)
    fout.close();

  m_mliap.del_mliap();

  if (m3_parameter->m3_pruning_speed_mode==0){
    file_in.close();
    delete [] read_buf;
    delete [] sample_buf;
    delete [] node_buf;
  }
}

void M3::M3_Run::classify_test_data_pruning(string file_name,
					    vector<bool> test_flag){

  if (m3_parameter->m3_pruning_speed_mode==1){
    make_all_test_sample_buf(file_name,
			     test_flag);

    BEGIN_DEBUG;
    // debug
    TIME_DEBUG_OUT << "All test buf : " << endl;
    for (int i=0;i<m_atsb.len;i++){
      TIME_DEBUG_OUT << "Sample: " << m_atsb.buf[i].index 
		     << " has " << m_atsb.buf[i].data_vector_length
		     << " nodes " << endl;
    }
    END_DEBUG;

  }

  m_level_info.clear();

  while (1){
    bool mid_score_save=false;
    float label_1,label_2;
    MPI_Status mpi_status;
    float quest_arr[3];

    MPI_Recv(quest_arr,
	     3,
	     MPI_FLOAT,
	     M3_MASTER_RANK,
	     M3_TAG,
	     MPI_COMM_WORLD,
	     &mpi_status);

    if (quest_arr[0]<0){

      // debug
      TIME_DEBUG_OUT << "run_process " << m3_my_rank << " classify over!!" << endl;

      break;
    }

    label_1=quest_arr[1];
    label_2=quest_arr[2];

    // debug
    TIME_DEBUG_OUT << "run_process " << m3_my_rank << " run label " << label_1 << " vs " << label_2 << endl;

    while (true){
      int flag=get_level_info_pruning();
      if (flag==-1)
	mid_score_save=true;
      if (flag<=0)
	break;
    }

    // debug
    TIME_DEBUG_OUT << "run_process " << m3_my_rank << " save score: " << mid_score_save << endl;

    make_level_info_classifier();

    if (m3_parameter->m3_pruning_speed_mode==1){
      m_atsb.init();
    }
    
    if (m3_parameter->m3_pruning_mode==1) // select pruning mode
      pipe_classify_test_data_sematric_pruning(file_name,
					       test_flag,
					       mid_score_save);
    else if (m3_parameter->m3_pruning_mode==2)
      pipe_classify_test_data_asematric_pruning(file_name,
						test_flag,
						mid_score_save);


     for (int i=0;i<m_level_info.size();i++)
       delete m_level_info[i];
     m_level_info.clear();
  }

  if (m3_parameter->m3_pruning_speed_mode==1){
    m_atsb.clear();
  }

}

void M3::M3_Run::classify_and_score_test_data_pruning(string file_name,
						      vector<bool> test_flag){
  bool mid_score_save=false;
  float label_1,label_2;
  MPI_Status mpi_status;

  m_level_info.clear();
  while (true){

    mid_score_save=false;

    float quest_arr[3];

    MPI_Recv(quest_arr,
	     3,
	     MPI_FLOAT,
	     M3_MASTER_RANK,
	     M3_TAG,
	     MPI_COMM_WORLD,
	     &mpi_status);

    if (quest_arr[0]<0){

      // debug
      TIME_DEBUG_OUT << "run_process " << m3_my_rank << " get pipe over!!" << endl;

      break;
    }

    label_1=quest_arr[1];
    label_2=quest_arr[2];

    // debug
    TIME_DEBUG_OUT << "run_process " << m3_my_rank << " get pipe label " << label_1 << " vs " << label_2 << endl;

    while (true){
      int flag=get_level_info_pruning();
      if (flag==-1)
	mid_score_save=true;
      if (flag<=0)
	break;
    }

    // debug
    TIME_DEBUG_OUT << "this pipe get done" << endl;

    make_level_info_classifier();

    // debug
    TIME_DEBUG_OUT << "this pipe classifier make done" << endl;

    m_pipe_pool.add(m_level_info,mid_score_save);

    m_level_info.clear();

  }

  // debug
  // check the pipe pool
  TIME_DEBUG_OUT << " check the pipe pool " << endl;
  for (int i=0;i<m_pipe_pool.level_info.size();i++){
    TIME_DEBUG_OUT << "pool " << i 
		   << " have " << m_pipe_pool.level_info[i].size() 
		   << " level " << endl;
    TIME_DEBUG_OUT << " response to master : " << m_pipe_pool.mid_save[i] << endl;
    for (int j=0;j<m_pipe_pool.level_info[i].size();j++){
      Level_Info * li=m_pipe_pool.level_info[i][j];
      TIME_DEBUG_OUT << "(lp: " 
		     << li->si->label_1 << " vs "
		     << li->si->label_2 << ")"
		     << "(id:"
		     << li->si->save_index << ")"
		     << "(pre:" 
		     << li->pre_process << ")"
		     << "(post:"
		     << li->post_process << ")" << endl;
    }
  }
  TIME_DEBUG_OUT << "check done " << endl;

  m_pipe_pool.init(TEST_AND_SCORE_ALL_DATA_BUF_SIZE);

  // debug
  TIME_DEBUG_OUT << "pipe pool init done" << endl;

  ifstream file_in(file_name.c_str());
  int index=0;
  while (true){
    int ctrl;
    MPI_Recv(&ctrl,
	     1,
	     MPI_INT,
	     M3_MASTER_RANK,
	     M3_TAG,
	     MPI_COMM_WORLD,
	     &mpi_status);

    // debug
    BEGIN_DEBUG;
    TIME_DEBUG_OUT << " get the cmd: " << ctrl << endl;
    END_DEBUG;

    if (ctrl==CTRL_TS_DONE){

      // debug
      TIME_DEBUG_OUT << " all test and score done!!!" << endl;

      break;
    }
    
    make_all_test_sample_buf(file_in,
			     test_flag,
			     index);

    // debug
    TIME_DEBUG_OUT << "make all test data buf done " << endl;

    // debug
    BEGIN_DEBUG;
    for (int ii=0;ii<m_atsb.len;ii++)
      TIME_DEBUG_OUT << " sample " << m_atsb.buf[ii].index
		     << " have " << m_atsb.buf[ii].data_vector_length 
		     << " nodes " << endl;
    END_DEBUG;

    m_pipe_pool.clear_rid();

    // debug
    TIME_DEBUG_OUT << "beging to classify and test the buf : " << m_atsb.len << endl;

    for (int i=0;i<m_pipe_pool.level_info.size();i++){

      m_atsb.init();
      m_pipe_pool.clear();

      m_level_info=m_pipe_pool.level_info[i];
      mid_score_save=m_pipe_pool.mid_save[i];

      label_1=m_level_info[0]->si[0].label_1;
      label_2=m_level_info[0]->si[0].label_2;

      // debug
      TIME_DEBUG_OUT << "do the test pair : " 
		     << label_1
		     << " vs " 
		     << label_2 << endl;

      if (m3_parameter->m3_pruning_mode==1) // select pruning mode
	pipe_classify_test_data_sematric_pruning(file_name,
						 test_flag,
						 mid_score_save);
      else if (m3_parameter->m3_pruning_mode==2)
	pipe_classify_test_data_asematric_pruning(file_name,
						  test_flag,
						  mid_score_save);

      m_pipe_pool.next_rid();
    }

    for (int i=0;i<m_pipe_pool.level_info.size();i++){
      
      m_level_info=m_pipe_pool.level_info[i];
      mid_score_save=m_pipe_pool.mid_save[i];
      
      label_1=m_level_info[0]->si[0].label_1;
      label_2=m_level_info[0]->si[0].label_2;
      
      if (mid_score_save){
	
	// debug
	TIME_DEBUG_OUT << "must send the buf to master" << endl;
	
	// debug
	TIME_DEBUG_OUT << "sent the test pair : " 
		       << label_1
		       << " vs " 
		       << label_2 << endl;


	MPI_Send(&m3_my_rank,
		 1,
		 MPI_INT,
		 M3_MASTER_RANK,
		 M3_TAG,
		 MPI_COMM_WORLD);

	float label[2];
	label[0]=label_1;
	label[1]=label_2;

	MPI_Send(label,
		 2,
		 MPI_FLOAT,
		 M3_MASTER_RANK,
		 M3_TAG,
		 MPI_COMM_WORLD);

	MPI_Send(m_pipe_pool.result[i],
		 m_atsb.len,
		 MPI_FLOAT,
		 M3_MASTER_RANK,
		 M3_TAG,
		 MPI_COMM_WORLD);
      }
    }

    // debug
    TIME_DEBUG_OUT << " this buf is done. do next " << endl;

    m_atsb.clear();
  }
  file_in.close();

}

void M3::M3_Run::classify_test_data(){

  string file_name=m3_parameter->classify_data;
  vector<bool> test_flag;
  test_flag.clear();
  MPI_Status mpi_status;
  MPI_Recv(&M3::testLen,
	   1,
	   MPI_INT,
	   M3_MASTER_RANK,
	   M3_TAG,
	   MPI_COMM_WORLD,
	   &mpi_status);

  size_t len = M3::testLen;

  for (int i=0;i<len;i++)
    test_flag.push_back(true);

  if (m3_parameter->m3_pruning_mode==0){
    classify_test_data_nonpruning();
  }
  else if (m3_parameter->m3_pruning_mode==1){
    if (m3_parameter->m3_pruning_combine_score==0)
      classify_test_data_pruning(file_name,
				 test_flag);
    else classify_and_score_test_data_pruning(file_name,
					      test_flag);
  }
  else if (m3_parameter->m3_pruning_mode==2){
    if (m3_parameter->m3_pruning_combine_score==0)
      classify_test_data_pruning(file_name,
				 test_flag);
    else classify_and_score_test_data_pruning(file_name,
					      test_flag);
  }
}

#undef TIME_DEBUG_OUT

#endif
