#ifndef _M3_SLAVE_H
#define _M3_SLAVE_H

#include "M3.h"

namespace M3{
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

        vector<float> m_index_to_label;//the m_index_to_label vector int master that be used in multi-label

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

        void multilabel_get_all_label();

        void multilabel_sent_divide_info();

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
}

#endif