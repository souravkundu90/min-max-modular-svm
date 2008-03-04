#ifndef _DATA_SPLIT_CPP
#define _DATA_SPLIT_CPP

#include "data_split.h"

using namespace std;

Data_Split::Data_Split(string f_name){
  init(f_name,1024*1024);
}

Data_Split::Data_Split(string f_name,
		       int max_line){
  init(f_name,max_line);
}

void Data_Split::init(string f_name,
		      int max_line){
  DATA_CONFIG="data.config";
  SPLIT_DIR="Split_Data/";
  system("mkdir Split_Data");

  file_name=f_name;
  buf_size=max_line;
  file_in.open(file_name.c_str());
  true_label.open((SPLIT_DIR+"true_label").c_str());

  read_buf=new char[buf_size];
  data_config.open((SPLIT_DIR+DATA_CONFIG).c_str());

  label_file.clear();
  flag_file.clear();
}

Data_Split::~Data_Split(){
  delete [] read_buf;

  label_size.clear();
  file_in.close();
  data_config.close();
  true_label.close();


  map<double,ofstream* >::iterator it;

  for (it=label_file.begin();it!=label_file.end();it++){
    (*(*it).second).close();
    delete (*it).second;
  }
  label_file.clear();

  for (it=flag_file.begin();it!=flag_file.end();it++){
    (*(*it).second).close();
    delete (*it).second;
  }
  flag_file.clear();
 }

void Data_Split::split(){
  double l;
  int index=0;

  while (file_in >> l){

  file_in.getline(read_buf,buf_size);

    if (label_file.find(l)==label_file.end()){
       char tmp[1000];
       sprintf(tmp,"%lf",l);
       string out_name=SPLIT_DIR+file_name+"_L"+tmp;
       string rank_name=out_name+".rank";

       ofstream * tmp_file=new ofstream;
       (*tmp_file).open(out_name.c_str());
       label_file[l]=tmp_file;
       
       data_config << l << " " << out_name << endl;
    }
    true_label << l << endl;
    (*(label_file[l])) << index++ << " " << l << read_buf << endl;
  }
}

#endif
