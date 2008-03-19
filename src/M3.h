#ifndef _M3_H
#define _M3_H

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <set>
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
#include "m3_macro.h"
#include "user_config.h"
/* #include "hc_voter.h" */
/* #include "hyper_plane.h" */
/* #include "libsvm_parameter.h" */
/* #include "libsvm.h" */
/* #include "svm_light.h" */

using namespace std;

namespace M3{

    class M3_Master;
    class M3_Slave;
    class M3_Run;

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

    // debug
    // debug for get the time
    static double link_predict_time=0;
    static double link_train_time=0;


    /////////////////////////////////add by hoss
    static int trainLen=-1;
    static int testLen=-1;
    static size_t wc_l(const string &filename)
    {
        size_t result = 0;
        ifstream fin(filename.c_str());
        //    char line[READ_BUF_SIZE];
        string line;
        //    while(fin.getline(line,READ_BUF_SIZE)) ++result;
        while (getline(fin,line)) ++result;
        fin.close();
        return result;
    }
    /////////////////////////////////add by hoss

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
