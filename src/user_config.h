#ifndef _USER_CONFIG_H
#define _USER_CONFIG_H

#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstdio>

using namespace std;

#define RET(i) if (line[i]=='#') return;

class user_config{
private:

    int run_mode;       //0--user,1--debug

    map<string,string> user_to_debug;
    map<string,map<string,string> > opt;
    map<string,string> default_opt;

    ifstream fin;
    ofstream _p_m3,_p_divider,_p_classifier;

    vector<string> _m3,_divider,_classifier;

    string line;
 

    void init(){
        run_mode=0;

        user_to_debug.clear();
        user_to_debug["train_file"]="train";
        user_to_debug["test_file"]="classify";
        user_to_debug["label_file"]="compare";
        user_to_debug["m3_running_process_num"]="running_process_num";
        user_to_debug["m3_subset_size"]="subset_size";
        user_to_debug["m3_module_size"]="modular_size";
        user_to_debug["m3_test_mode"]="m3_pruning_mode";
        user_to_debug["m3_test_preload_all_data"]="m3_pruning_speed_mode";
        user_to_debug["m3_divider_type"]="divider_rank";
        user_to_debug["m3_classifier_type"]="classifier_rank";
        user_to_debug["m3_voter_type"]="voter_rank";

        opt.clear();
        default_opt.clear();
        map<string,string> m3_test_mode;
        m3_test_mode.clear();
        m3_test_mode["sematric"]="1";
        m3_test_mode["asematric"]="2";
        m3_test_mode["none"]="0";
        opt["m3_test_mode"]=m3_test_mode;
        default_opt["m3_test_mode"]="0";
        map<string,string> m3_test_preload_all_data;
        m3_test_preload_all_data.clear();
        m3_test_preload_all_data["open"]="1";
        m3_test_preload_all_data["close"]="0";
        opt["m3_test_preload_all_data"]=m3_test_preload_all_data;
        default_opt["m3_test_preload_all_data"]="0";
        map<string,string> m3_divider_type;
        m3_divider_type.clear();
        m3_divider_type["hyper_plane"]="1";
        m3_divider_type["random"]="2";
        m3_divider_type["centroid"]="3";
        m3_divider_type["prior"]="4";
        opt["m3_divider_type"]=m3_divider_type;
        default_opt["m3_divider_type"]="1";
        map<string,string> m3_classifier_type;
        m3_classifier_type.clear();
        m3_classifier_type["libsvm"]="1";
        m3_classifier_type["svm_light"]="2";
        opt["m3_classifier_type"]=m3_classifier_type;
        default_opt["m3_classifier_type"]="2";
        map<string,string> m3_voter_type;
        m3_voter_type.clear();
        m3_voter_type["hc_voter"]="1";
        opt["m3_voter_type"]=m3_voter_type;
        default_opt["m3_voter_type"]="1";

        fin.open("configure.txt");
        _p_m3.open("m3.config");
        _p_divider.open("divider.config");
        _p_classifier.open("classifier.config");
    }

    void destory(){
        fin.close();
        _p_m3.close();
        _p_divider.close();
        _p_classifier.close();
    }

    void parse_run(){
        int i,j;
        for (i=0;i<line.size() && line[i]==' ';i++) RET(i);
        for (j=i;j<line.size() && line[j]!=':';j++) RET(j);
        string ll=line.substr(i,j-i);
        if (ll!="run_mode") return;
        for (i=j+1;i<line.size() && line[i]!='[';i++) RET(i);
        for (j=i+1;j<line.size() && line[j]!=']';j++) RET(j);
        string rr=line.substr(i+1,j-i-1);
        if (rr=="user")
            run_mode=0;
        if (rr=="debug")
            run_mode=1;
    }

    void parse_m3(){
        int i,j;
        for (i=0;i<line.size() && line[i]==' ';i++) RET(i);
        for (j=i;j<line.size() && line[j]!=':';j++) RET(j);
        string ll=line.substr(i,j-i);
        if (user_to_debug.find(ll)==user_to_debug.end()) return;
        for (i=j+1;i<line.size() && line[i]!='[';i++) RET(i);
        for (j=i+1;j<line.size() && line[j]!=']';j++) RET(j);
        string rr=line.substr(i+1,j-i-1);

        string wll=user_to_debug[ll];
        
        if (opt.find(ll)==opt.end()){
            _m3.push_back(wll+"="+rr);
            return;
        }

        string wrr=default_opt[ll];
        if (opt[ll].find(rr)!=opt[ll].end())
            wrr=opt[ll][rr];
        _m3.push_back(wll+"="+wrr);
    }

    void parse_classifier(){
        int i,j;
        for (i=0;i<line.size() && line[i]==' ';i++) RET(i);
        for (j=0;j<line.size() && line[j]!='#';j++);
        if (j<=i) return;
        string w=line.substr(i,j-i);

        _classifier.push_back(w);
    }

    void parse_divider(){
        int i,j;
        for (i=0;i<line.size() && line[i]==' ';i++) RET(i);
        for (j=0;j<line.size() && line[j]!='#';j++);
        if (j<=i) return;
        string w=line.substr(i,j-i);

        _divider.push_back(w);
    }
    

    void parse(){
        int now=0; 
        int i;
        while (getline(fin,line)){
            if (now==0){
                if (line[0]!='$'){
                    continue;
                }
            }
            if (line[0]=='$'){
                for (i=1;i<line.size() && line[i]!='$';i++);
                string change_mode=line.substr(1,i-1);
                if (change_mode=="run_mode")
                    now=1;
                else if (change_mode=="m3_parameter")
                    now=2;
                else if (change_mode=="classifier_parameter")
                    now=3;
                else if (change_mode=="divider_parameter")
                    now=4;
                continue;
            }
            switch (now){
                case 1:parse_run();break;
                case 2:parse_m3();break;
                case 3:parse_classifier();break;
                case 4:parse_divider();break;
            }
        }
    }

    void write(){
        if (run_mode==1) return;
        for (int i=0;i<_m3.size();i++)
            _p_m3 << _m3[i] << endl;
        for (int i=0;i<_classifier.size();i++)
            _p_classifier << _classifier[i] << endl;
        for (int i=0;i<_divider.size();i++)
            _p_divider << _divider[i] << endl;
    }

public:
    user_config(){
        init();
        parse();
        write();
    }

    ~user_config(){
        destory();
    }

};

#endif