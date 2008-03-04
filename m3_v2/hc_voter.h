#ifndef _HC_VOTER_H
#define _HC_VOTER_H

#include "voter.h"
using namespace std;

class HC_Voter:public Voter{
 public:
  
  float vote(vector<float> label,
	     double ** score_matrix);

  float vote(vector<float> label,
	     double ** score_matrix,
	     double & confident);
  
  void matrix_inverse(vector<float> label,
		      double ** score_matrix);
};

#endif
