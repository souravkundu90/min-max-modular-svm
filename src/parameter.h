#ifndef _PARAMETER_H
#define _PARAMETER_H

#include <string>
using namespace std;


class M3_Parameter{
  void init(string pfile_name);	/* initilalize */

 public:

  M3_Parameter();

  M3_Parameter(string pfile_name); /* default "m3.config" */

  ~M3_Parameter();

  void parse_increase_learning();

  void parse_as_cmd(string pfile_name,int ** argc,char *** argv); /* for parse classifier parameter */

  void rm_parse_as_cmd(int ** argc,char *** argv); /* remove the pointer */

  int running_process_num;      /* m3_start_slave_process_rank='this'+1 
				 * how many process be used to 
				 * be running but not store data
				 */

  int subset_size;		/* m3_subset_size='this' 
				 * how many sample be a subset
				 */

  int modular_size;		/* m3_continue_subset_size='this'
				 * how many subset be store as modul
				 * for save disk's space
				 */

  int divider_rank;		/* define by user in factory */

  int classifier_rank;		/* define by user in factory */

  int classifier_parameter_rank;/* define by user in factory */

  int voter_rank;		/* define by user in factory */

  bool flag_train;		/* denote the job */
  string train_data;

  bool flag_classify;		/* denote the job*/
  string classify_data;

  bool flag_score;

  bool flag_compare;		/* denote the job */
  string compare_label_data;

  int m3_min_max_mode;		/* 0--save all mid score 
				 * 1--save min vector to max
				 */

  int m3_load_mode;		/* 0--parallel 
				 * 1--serial
				 */

  int m3_classify_mode;		/* 0--parallel
				 * 1--serial
				 */
  int m3_pruning_mode;		/* 0--no pruning 
				 * 1--sematric pruning(classify mode && score mode are nothing at all)
				 */

  int m3_pruning_speed_mode;	/* 0--no  
				 * 1--load test data in memory.the size of the data must be able to load by user!!!!!!!!!!!!
				 */

  int m3_pruning_combine_score;	/* 0--only mid score file 
				 * 1--only voter by master
				 * 2--both
				 */

  bool m3_save_for_increase_learning;

  bool m3_increase_learning;
  string increase_learning_config; /* the increase learning config file */
  /* ------------------------------------------- */
  string pre_subset_config;
  int pre_data_process;
  int * pre_data_process_rank;
  float * pre_data_label;
  int * pre_data_num;
  int * pre_data_divide_num;	/* the num of block in every process */
  /* ------------------------------------------- */


};

class Classifier_Parameter//added by hoss
{
public :
	virtual ~Classifier_Parameter(){}
	virtual void Parse(int argc,char ** argv){}
	virtual void print_help(void){}
};


#endif
