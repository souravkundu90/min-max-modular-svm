#ifndef _VOTER_H
#define _VOTER_H

#include <vector>
#include <map>

using namespace std;

class Voter{
 public :
  /*   virtual float vote(vector<int ,double>*, */
  /* 		     map<class Block_Index,double>**) = 0; */
  /*   virtual ~Voter(){} */
  
  virtual float vote(vector<float> label,
		     double ** score_matrix)=0;  /* for an n*n(n is the num of label) *full* score matrix
						 * get the result ot the test data
						 */

  virtual float vote(vector<float> label,
		    double ** score_matrix,
		    double & confident)=0; /* the confident of the label */
  
  virtual void matrix_inverse(vector<float> label,
			      double ** score_matrix)=0; /* I only give you a *up_triangle* score matrix
							 * notice: the element not at the m[i][j](i<j)
							 *         but at the m[i][j](label[i]<label[j])
							 * You must fill the matrix as you rule
							 * unless the vote does not need the full matrix
							 */
};

#endif
