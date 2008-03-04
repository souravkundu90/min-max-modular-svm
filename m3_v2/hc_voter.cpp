#ifndef _HC_VOTER_CPP
#define _HC_VOTER_CPP

#include "hc_voter.h"

#include <iostream>
using namespace std;

float HC_Voter::vote(vector<float> label,
		     double ** score_matrix){

  int i,j;
  int len=label.size();
  int * tms=new int[len];

  memset(tms,0,sizeof(int)*len);

  for (i=0;i<len;i++)
    for (j=0;j<len;j++)
      if (score_matrix[i][j]>0) tms[i]++;
      else tms[j]++;

  int bj,max_tms=0;
  for (i=0;i<len;i++)
    if (max_tms<tms[i]){
      bj=i;
      max_tms=tms[i];
    }

  delete [] tms;

  return label[bj];
}

float HC_Voter::vote(vector<float> label,
		     double ** score_matrix,
		     double & confident){

  int i,j;
  int len=label.size();
  int * tms=new int[len];

  memset(tms,0,sizeof(int)*len);

  for (i=0;i<len;i++)
    for (j=0;j<len;j++)
      if (i!=j)
	if (score_matrix[i][j]>0) tms[i]++;
	else tms[j]++;

  int bj,max_tms=0;
  for (i=0;i<len;i++)
    if (max_tms<tms[i]){
      bj=i;
      max_tms=tms[i];
    }

  delete [] tms;

  confident=max_tms*1.0/2.0/(label.size()-1);

  return label[bj];
}

void HC_Voter::matrix_inverse(vector<float> label,
			      double ** score_matrix){

  int i,j;
  int len=label.size();

  //score rule: -1 to 1
  for (i=0;i<len;i++)
    for (j=0;j<len;j++){
      if (i==j) 
	score_matrix[i][j]=0;
      else if (label[i]>label[j])
	score_matrix[i][j]=-score_matrix[j][i];
    }

  // debug
//   for (i=0;i<len;i++){
//     for (j=0;j<len;j++)
//       cout << score_matrix[i][j] << " ";
//     cout << endl;
//   }

  //   // score rule: 0 to 1
  //   for (i=0;i<len;i++)
  //     for (j=0;j<len;j++)
  //       if (i==j) 
  // 	score_matrix[i][j]=0.5;
  //       else if (label[i]>label[j])
  // 	score_matrix[i][j]=1-score_matrix[j][i];

}

#endif
