#include "M3.h"
#include "util.h"
#include <fstream>
#include <iostream>
using namespace std;
using namespace M3;
int main(int argc,char ** argv){

  MPI_Init(&argc,
	   &argv);

  M3::parse();			// parse m3.config

  M3::initialize(argc,argv);	// initialize

  if (M3::flag_train()){
    M3::load_train_data();
    M3::divide_train_data();
    M3::training_train_data();
  }

  if (M3::flag_classify()){
    M3::classify_test_data();
  }

  if (M3::flag_score()){
    M3::score_test_data();
  }

  if (M3::flag_compare()){
    M3::compare_true_label();
  }

  M3::finalize();

  return 0;
}
