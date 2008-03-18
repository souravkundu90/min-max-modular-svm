#ifndef _M3_MASTER_H
#define _M3_MASTER_H

#include "M3.h"

namespace M3{

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

            bool in_multilabel_pipe(Subset_Info & si){
                return si.label_1==label_1;
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


        map<int, vector<int>> m_process_train_subset_multinum; 
        // the subset number of all divide in a process is not the same when does multi-label


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

        void multilabel_sent_all_label();        void multilabel_get_deivide_info();

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

        void multilabel_make_train_info();

        void save_for_increase_learning();

        void test_ctrl(int ctrl);

        void load_subset_config();

        bool ask_load_subset(int index);

        void classify_test_data_serial(string file_name,
            vector<bool> test_flag);

        void classify_test_data_parallel();

        void classify_test_data_nonpruning(const string &);

        void make_pipe_info_pruning();

        void multilabel_make_pipe_info_pruning();

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

        void score_file_combine_pruning(); // faint for the finit file point in OS!!!!!!!!

        void score_test_data_pruning(vector<bool> test_flag);

    public:
        M3_Master();
        void load_train_data(const string &);
        void divide_train_data();
        void training_train_data();
        void classify_test_data();
        void score_test_data();
        void compare_true_label(const string &);
    };
}

#endif