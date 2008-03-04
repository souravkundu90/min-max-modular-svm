#ifndef _UTIL_H
#define _UTIL_H
#include "mpi.h"

struct Data_Node{
  int index;
  float value;
};

struct Data_Sample{
  int index;
  float label;
  int data_vector_length;
  Data_Node * data_vector;
};

struct Divide_Info{
  int start_offset,end_offset;
  int length;
};

struct Subset_Info{
  float label_1,label_2;
  int subset_num_1,subset_num_2;
  int save_index;
  int subset_memory;
  int process_rank;
  int process_1,process_2;
  int start_1,start_2;
  int end_1,end_2;
};

#endif
