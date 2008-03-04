#include "svm_light_parameter.h"

svm_light_parameter::svm_light_parameter(void)
{
  strcpy (learn_parm.predfile, "trans_predictions");
  strcpy (learn_parm.alphafile, "");
  verbosity=1;
  learn_parm.biased_hyperplane=1;
  learn_parm.remove_inconsistent=0;
  learn_parm.skip_final_opt_check=0;
  learn_parm.svm_maxqpsize=10;
  learn_parm.svm_newvarsinqp=0;
  learn_parm.svm_iter_to_shrink=-9999;
  kernel_cache_size=40;
  learn_parm.svm_c=1;
  learn_parm.eps=0.1;
  learn_parm.transduction_posratio=-1.0;
  learn_parm.svm_costratio=1.0;
  learn_parm.svm_costratio_unlab=1.0;
  learn_parm.svm_unlabbound=1E-5;
  learn_parm.epsilon_crit=0.001;
  learn_parm.epsilon_a=1E-15;
  learn_parm.compute_loo=0;
  learn_parm.rho=1.0;
  learn_parm.xa_depth=0;
  learn_parm.type = CLASSIFICATION;
  kernel_parm.kernel_type=2;
  kernel_parm.poly_degree=3;
  kernel_parm.rbf_gamma=1.0;
  kernel_parm.coef_lin=1;
  kernel_parm.coef_const=1;
  strcpy(kernel_parm.custom,"empty");
}

svm_light_parameter::~svm_light_parameter(void)
{
}
void print_help(void);
void svm_light_parameter::Parse(int argc, char *argv[])
{
  long i;
  char type[100];
  
  /* set default */
 strcpy(type,"c");

  for(i=1;(i<argc) && ((argv[i])[0] == '-');i++) {
    switch ((argv[i])[1]) 
      { 
      case '?': print_help();break;
      case 'z': i++; strcpy(type,argv[i]); break;
      case 'v': i++; verbosity =atol(argv[i]); break;
      case 'b': i++; learn_parm.biased_hyperplane=atol(argv[i]); break;
      case 'i': i++; learn_parm.remove_inconsistent=atol(argv[i]); break;
      case 'f': i++; learn_parm.skip_final_opt_check=!atol(argv[i]); break;
      case 'q': i++; learn_parm.svm_maxqpsize=atol(argv[i]); break;
      case 'n': i++; learn_parm.svm_newvarsinqp=atol(argv[i]); break;
      case 'h': i++; learn_parm.svm_iter_to_shrink=atol(argv[i]); break;
      case 'm': i++; kernel_cache_size =atol(argv[i]); break;
      case 'c': i++; learn_parm.svm_c=atof(argv[i]); break;
      case 'w': i++; learn_parm.eps=atof(argv[i]); break;
      case 'p': i++; learn_parm.transduction_posratio=atof(argv[i]); break;
      case 'j': i++; learn_parm.svm_costratio=atof(argv[i]); break;
      case 'e': i++; learn_parm.epsilon_crit=atof(argv[i]); break;
      case 'o': i++; learn_parm.rho=atof(argv[i]); break;
      case 'k': i++; learn_parm.xa_depth=atol(argv[i]); break;
      case 'x': i++; learn_parm.compute_loo=atol(argv[i]); break;
      case 't': i++; kernel_parm.kernel_type=atol(argv[i]); break;
      case 'd': i++; kernel_parm.poly_degree=atol(argv[i]); break;
      case 'g': i++; kernel_parm.rbf_gamma=atof(argv[i]); break;
      case 's': i++; kernel_parm.coef_lin=atof(argv[i]); break;
      case 'r': i++; kernel_parm.coef_const=atof(argv[i]); break;
      case 'u': i++; strcpy(kernel_parm.custom,argv[i]); break;
      case 'l': i++; strcpy(learn_parm.predfile,argv[i]); break;
      case 'a': i++; strcpy(learn_parm.alphafile,argv[i]); break;
      default: printf("\nUnrecognized option %s!\n\n",argv[i]);
	       print_help();
	       exit(0);
      }
  }
  if(learn_parm.svm_iter_to_shrink == -9999) {
    if(kernel_parm.kernel_type == LINEAR) 
      learn_parm.svm_iter_to_shrink=2;
    else
      learn_parm.svm_iter_to_shrink=100;
  }
  if(strcmp(type,"c")==0) {
    learn_parm.type=CLASSIFICATION;
  }
  else if(strcmp(type,"r")==0) {
    learn_parm.type=REGRESSION;
  }
  else {
    printf("\nUnknown type '%s': Valid types are 'c' (classification) and 'r' regession.\n",type);
    print_help();
  }    
  if((learn_parm.skip_final_opt_check) 
     && (kernel_parm.kernel_type == LINEAR)) {
    printf("\nIt does not make sense to skip the final optimality check for linear kernels.\n\n");
    learn_parm.skip_final_opt_check=0;
  }    
  if((learn_parm.skip_final_opt_check) 
     && (learn_parm.remove_inconsistent)) {
    printf("\nIt is necessary to do the final optimality check when removing inconsistent \nexamples.\n");
    print_help();
  }    
  if((learn_parm.svm_maxqpsize<2)) {
    printf("\nMaximum size of QP-subproblems not in valid range: %ld [2..]\n",learn_parm.svm_maxqpsize); 
    print_help();
  }
  if((learn_parm.svm_maxqpsize<learn_parm.svm_newvarsinqp)) {
    printf("\nMaximum size of QP-subproblems [%ld] must be larger than the number of\n",learn_parm.svm_maxqpsize); 
    printf("new variables [%ld] entering the working set in each iteration.\n",learn_parm.svm_newvarsinqp); 
    print_help();
  }
  if(learn_parm.svm_iter_to_shrink<1) {
    printf("\nMaximum number of iterations for shrinking not in valid range: %ld [1,..]\n",learn_parm.svm_iter_to_shrink);
    print_help();
  }
  if(learn_parm.svm_c<0) {
    printf("\nThe C parameter must be greater than zero!\n\n");
    print_help();
  }
  if(learn_parm.transduction_posratio>1) {
    printf("\nThe fraction of unlabeled examples to classify as positives must\n");
    printf("be less than 1.0 !!!\n\n");
  }
  if(learn_parm.svm_costratio<=0) {
    printf("\nThe COSTRATIO parameter must be greater than zero!\n\n");
    print_help();
  }
  if(learn_parm.epsilon_crit<=0) {
    printf("\nThe epsilon parameter must be greater than zero!\n\n");
    print_help();
  }
  if(learn_parm.rho<0) {
    printf("\nThe parameter rho for xi/alpha-estimates and leave-one-out pruning must\n");
    printf("be greater than zero (typically 1.0 or 2.0, see T. Joachims, Estimating the\n");
    printf("Generalization Performance of an SVM Efficiently, ICML, 2000.)!\n\n");
    print_help();
  }
  if((learn_parm.xa_depth<0) || (learn_parm.xa_depth>100)) {
    printf("\nThe parameter depth for ext. xi/alpha-estimates must be in [0..100] (zero\n");
    printf("for switching to the conventional xa/estimates described in T. Joachims,\n");
    printf("Estimating the Generalization Performance of an SVM Efficiently, ICML, 2000.)\n");
    print_help();
  }
}
void svm_light_parameter::print_help(void)
{
  printf("\nSVM-light parameters help");
  printf("General options:\n");
  printf("         -?          -> this help\n");
  printf("         -v [0..3]   -> verbosity level (default 1)\n");
  printf("Learning options:\n");
  printf("         -z {c,r}    -> select between classification and regression\n");
  printf("                        (default classification)\n");
  printf("         -c float    -> C: trade-off between training error\n");
  printf("                        and margin (default [avg. x*x]^-1)\n");
  printf("         -w [0..]    -> epsilon width of tube for regression\n");
  printf("                        (default 0.1)\n");
  printf("         -j float    -> Cost: cost-factor, by which training errors on\n");
  printf("                        positive examples outweight errors on negative\n");
  printf("                        examples (default 1) (see [4])\n");
  printf("         -b [0,1]    -> use biased hyperplane (i.e. x*w+b>0) instead\n");
  printf("                        of unbiased hyperplane (i.e. x*w>0) (default 1)\n");
  printf("         -i [0,1]    -> remove inconsistent training examples\n");
  printf("                        and retrain (default 0)\n");
  printf("Performance estimation options:\n");
  printf("         -x [0,1]    -> compute leave-one-out estimates (default 0)\n");
  printf("                        (see [5])\n");
  printf("         -o ]0..2]   -> value of rho for XiAlpha-estimator and for pruning\n");
  printf("                        leave-one-out computation (default 1.0) (see [2])\n");
  printf("         -k [0..100] -> search depth for extended XiAlpha-estimator \n");
  printf("                        (default 0)\n");
  printf("Transduction options (see [3]):\n");
  printf("         -p [0..1]   -> fraction of unlabeled examples to be classified\n");
  printf("                        into the positive class (default is the ratio of\n");
  printf("                        positive and negative examples in the training data)\n");
  printf("Kernel options:\n");
  printf("         -t int      -> type of kernel function:\n");
  printf("                        0: linear (default)\n");
  printf("                        1: polynomial (s a*b+c)^d\n");
  printf("                        2: radial basis function exp(-gamma ||a-b||^2)\n");
  printf("                        3: sigmoid tanh(s a*b + c)\n");
  printf("                        4: user defined kernel from kernel.h\n");
  printf("         -d int      -> parameter d in polynomial kernel\n");
  printf("         -g float    -> parameter gamma in rbf kernel\n");
  printf("         -s float    -> parameter s in sigmoid/poly kernel\n");
  printf("         -r float    -> parameter c in sigmoid/poly kernel\n");
  printf("         -u string   -> parameter of user defined kernel\n");
  printf("Optimization options (see [1]):\n");
  printf("         -q [2..]    -> maximum size of QP-subproblems (default 10)\n");
  printf("         -n [2..q]   -> number of new variables entering the working set\n");
  printf("                        in each iteration (default n = q). Set n<q to prevent\n");
  printf("                        zig-zagging.\n");
  printf("         -m [5..]    -> size of cache for kernel evaluations in MB (default 40)\n");
  printf("                        The larger the faster...\n");
  printf("         -e float    -> eps: Allow that error for termination criterion\n");
  printf("                        [y [w*x+b] - 1] >= eps (default 0.001)\n");
  printf("         -h [5..]    -> number of iterations a variable needs to be\n"); 
  printf("                        optimal before considered for shrinking (default 100)\n");
  printf("         -f [0,1]    -> do final optimality check for variables removed\n");
  printf("                        by shrinking. Although this test is usually \n");
  printf("                        positive, there is no guarantee that the optimum\n");
  printf("                        was found if the test is omitted. (default 1)\n");
}
