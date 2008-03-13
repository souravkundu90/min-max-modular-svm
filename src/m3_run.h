#ifndef _M3_RUN_H
#define _M3_RUN_H

#include "M3.h"


class M3::M3_Run{  
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

#endif