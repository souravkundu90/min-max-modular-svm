#ifndef _DATA_SPLIT_H
#define _DATA_SPLIT_H

#include <iostream>
#include <fstream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

using namespace std;

class Data_Split{

 public:

  Data_Split(string f_name);

  Data_Split(string f_name,int max_line);

  ~Data_Split();

  void init(string f_name,int max_line);

  void split();

  void split(vector<bool> t_flag);

 private:

  string DATA_CONFIG;
  string SPLIT_DIR;

  string file_name;
  int buf_size;
  char * read_buf;

  ifstream file_in;
  ofstream data_config;
  ofstream true_label;
  map<double,ofstream* > label_file,flag_file;
  map<double,int > label_size;
};

#endif
