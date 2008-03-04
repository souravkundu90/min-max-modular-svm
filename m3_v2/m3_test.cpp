#include <iostream>
#include <fstream>
#include <stdio.h>
using namespace std;

#include "util.h"
#include "M3.h"
#include "divider.h"
#include "hyper_plane.h"
#include "libsvm.h"
#include "voter.h"
#define Malloc(type,n) (type *)malloc((n)*sizeof(type))
class Exchange
{
public :
	float string_to_float(char * str,
				     int ll,
				     int rr)
	{
		/*
		int i;
		float res_r=0,res_c=0;
		bool dot=false;
		for (i=ll;i<rr && str[i]!='.';i++)
	    	res_r=res_r*10.0+str[i]-'0';
		for (i=rr-1;i>=ll && str[i]!='.';i--)
	    	res_c=res_c/10.0+str[i]-'0';
	  	res_c/=10;
	  	if (i<=ll) res_c=0;
	  	return res_r+res_c;
		*/
//////////////////////////////////////////////added by hoss//////////////////////////
		char temp = str[rr];
		str[rr] = 0;
		float res = atof(&(str[ll]));
		str[rr] = temp;
		return res;
/////////////////////////////////////////////////////////////////////////////////////
	}

	int string_to_int(char * str,
					 int ll,
					 int rr)
	{
		/*
		int i;
		int res=0;
		for (i=ll;i<rr;i++)
		{
		  res=res*10+str[i]-'0';
		}
		return res;
		*/
//////////////////////////////////////////////added by hoss///////////////////////////
		char temp = str[rr];
		str[rr] = 0;
		int res = atoi(&(str[ll]));
		str[rr] = temp;
		return res;
//////////////////////////////////////////////////////////////////////////////////////
	}


	void parse_data(char * rbuf,
				       Data_Sample * dsp)
	{
		int i=0,pri=0;
		int len=0;
		while (rbuf[i]!=' ') i++; 
		dsp->label=string_to_float(rbuf,pri,i);
		while (rbuf[i])
		{
			pri=++i;
			while (rbuf[i]!=':') i++;
			dsp->data_vector[len].index=string_to_int(rbuf,pri,i);
			pri=++i;
		  	while (rbuf[i] && rbuf[i]!=' ') i++;
			dsp->data_vector[len].value=string_to_float(rbuf,pri,i);
			len++;
		}
		dsp->data_vector_length=len;
	}

};

class M3Test
{
public :
	void testFloat(float data)
	{
		char str[20];
		sprintf(str,"%f",data);
		int i = 0, j = strlen(str)+1;
		Exchange tmp;
		float result = 0;
	    result = tmp.string_to_float(str,i,j);
		if((double)result - data<0.00001 && (double)data - result<0.00001)
		{
			cout<<data<<" "<<"Pass!"<<endl;
		}
		else
		{
			cout<<data<<"  "<<"failed!"<<endl;
		}
	}
	void testInt(int data)
	{
		char str[20];
		sprintf(str,"%d",data);
		int i = 0, j = strlen(str);
		Exchange tmp;
		int result = 0;
	    result = tmp.string_to_int(str,i,j);
		if(result==data)
		{
			cout<<data<<" "<<"Pass!"<<endl;
		}
		else
		{
			cout<<data<<"  "<<"failed!"<<endl;
		}

	}
	void testParse(string filename)
	{
		void numToStr(char*,int);
		void numToStr(char*,float);
		ifstream inf(filename.c_str(),ios::in|ios::binary);
		string buffer;
		string result("");
		char tempChar[20];
		Exchange tmp;
		Data_Sample tempSample;
		Data_Node* tempNodes = new Data_Node[128];
		tempSample.data_vector = tempNodes;	
		while(getline(inf,buffer))
		{
			result = "";
			cout<<buffer<<endl;
			tmp.parse_data(const_cast<char*>(buffer.c_str()),&tempSample);
			int leng = tempSample.data_vector_length;
			numToStr(tempChar,tempSample.label);
			result += tempChar;
			for (int i = 0 ; i<leng ; i++)
			{
				numToStr(tempChar,tempSample.data_vector[i].index);
				result += " ";
				result += tempChar;
				numToStr(tempChar,tempSample.data_vector[i].value);
				result += ":";
				result += tempChar;
			}
			cout<<result<<endl;
			if(result == buffer)
			{
				cout<<"Pass!"<<endl;
			}
			else
			{
				cout<<"Failed!"<<endl;
			}
		}
		delete[] tempNodes;
		inf.close();
	}
	void testDivide(const string& filename)
	{
		svm_problem prob;
		Data_Sample** problem;
		readProblem(filename,prob,problem);
		size_t length = prob.l;
		Hyper_Plane divider;
		vector<Divide_Info> tempInfo = divider.divide(problem,length,2);
		vector<Divide_Info>::iterator inter = tempInfo.begin();
		vector<Divide_Info>::iterator endInter = tempInfo.end();
		while(inter != endInter)
		{
			cout<<inter->start_offset<<"    "<<inter->end_offset<<"    "<<endl;
			++inter;
		}
		showProblem(prob,problem);
		deleteProblem(prob,problem);
	}
	int testLibsvm(const string &filenameP,const string &filenameN,const string& classFile,int set1Num,int set2Num)
	{
		libsvm_parameter para;
		libsvm classifier(para);
		svm_problem probP,probN;
		Data_Sample** problemP;
		Data_Sample** problemN;
		int* p;
		int* n;
		makeTrainProblem(filenameP.c_str(),probP,problemP,set1Num,p);
		makeTrainProblem(filenameN.c_str(),probN,problemN,set2Num,n);
//		showTrainData(1,p,problemP);
//		showTrainData(1,n,problemN);
//		cout<<"p: "<<p[0]<<" n: "<<n[0]<<endl;
		classifier.train(problemP,problemN,set1Num,set2Num,p,n,"model");
		classifier.load_model("model");
		svm_problem testPro;
		Data_Sample** testProblem;
		readProblem(classFile.c_str(),testPro,testProblem);
		size_t length = testPro.l;
		Data_Sample* tempSample = new Data_Sample[length];
		for(int i=0;i<length;++i)
		{
			tempSample[i] = *(testProblem[i]);
		}
		size_t setLen = set1Num * set2Num;
		double* result = new double[setLen];
		memset(result,0,sizeof(double)*setLen);
		bool pass = true;
		for(int i=0;i<length;++i)
		{
			result = classifier.predict(&(tempSample[i]));
			cout<<tempSample[i].label<<":";
			for(int tempI = 0;tempI<setLen;++tempI)
			{
				int res = result[tempI]>0?1:-1;
				cout<<res<<" ";
				if(res != tempSample[i].label)
				{pass = false;}
			}
			cout<<endl;
		}
		if(pass == true)
		{
			cout<<"-------------------------------------------Pass!"<<endl;
		}
		else
		{
			cout<<"+++++++++++++++++++++++++++++++++++++++++++Failed!"<<endl;
		}
		delete[] p;
		delete[] n;
		delete[] tempSample;
		deleteProblem(testPro,testProblem);
		delete[] result;
		delete[] n;
		delete[] p;
		deleteProblem(probN,problemN,set1Num);
		deleteProblem(probP,problemP,set2Num);
	}
private:
	void showTrainData(const int subNum,const int* eachNum,Data_Sample const * const * const &problem)
	{
		for(int i=0;i<subNum;++i)
		{
			for(int j=0;j<eachNum[i];++j)
			{
				Data_Sample tempDS = problem[i][j];
				cout<<tempDS.label;
				size_t len = tempDS.data_vector_length;
				for(int k=0;k<len;++k)
				{
					cout<<" "<<tempDS.data_vector[k].index<<":"
						<<tempDS.data_vector[k].value;
				}
				cout<<endl;
			}
		}
	}
	void makeTrainProblem(const string& filename,svm_problem& prob,Data_Sample**& problem,const int subsetNum,int*& setLength)
	{
		read_problem(filename,prob);
		size_t length = prob.l;
		problem = new Data_Sample*[subsetNum];
		float numInBlock = length/(float)subsetNum;
		size_t num = 0;
		setLength = new int[subsetNum];
		for(int i=0;i<subsetNum;++i)
		{
			size_t ditoNum = static_cast<size_t>(numInBlock * (i+1));
			size_t eachNum = ditoNum - num;
			setLength[i] = eachNum;
			problem[i] = new Data_Sample[eachNum];
			for(int j=0;j<eachNum;++j)
			{
				size_t ind = num + j;
				problem[i][j].index = ind;
				problem[i][j].label = prob.y[ind];
				problem[i][j].data_vector = reinterpret_cast<Data_Node*>(prob.x[ind]);
				for(int k=0;;++k)
				{
					if(prob.x[ind][k].index == -1)
					{
						problem[i][j].data_vector_length = k;
						break;
					}
				}
			}
			num = ditoNum;
		}
	}
	void read_problem(const string& filename,svm_problem& prob);
	void readProblem(const string& filename,svm_problem& prob,Data_Sample**& problem)
	{
		read_problem(filename,prob);
		size_t length = prob.l;
		length = prob.l;
		problem = new Data_Sample*[length];
		for(int i=0;i<length;++i)
		{
			problem[i] = new Data_Sample[1];
			problem[i]->index = i;
			problem[i]->label = prob.y[i];
			problem[i]->data_vector = reinterpret_cast<Data_Node*>(prob.x[i]);
			for(int j=0;;++j)
			{
				if(prob.x[i][j].index == -1)
				{
					problem[i]->data_vector_length = j;
					break;
				}
			}
		}
	}
	void deleteProblem(svm_problem& prob,Data_Sample**& problem,size_t len = 0)
	{
		delete[] prob.x[0];
		delete[] prob.x;
		delete[] prob.y;
		if(len == 0)
		{len = prob.l;}
		for(int i=0;i<len;++i)
		{
			delete[] problem[i];
		}
		delete[] problem;
	}
	void showProblem(const svm_problem& prob,Data_Sample const*const* const &problem)
	{
		size_t length = prob.l;
		for(int i=0;i<length;i++)
		{
			Data_Sample const* const &tempDS = problem[i];
			size_t len = tempDS->data_vector_length;
			cout<<tempDS->label<<" "<<tempDS->index;
			for(int j=0;j<len;j++)
			{
				Data_Node* tempDN = tempDS->data_vector;
				cout<<" "<<tempDN[j].index<<":"<<tempDN[j].value;
			}
			cout<<endl;
		}	
	}

};

void M3Test::read_problem(const string& filename,svm_problem& prob)
{
	int elements, max_index, i, j;
	FILE *fp = fopen(filename.c_str(),"r");

	if(fp == NULL)
	{
		fprintf(stderr,"can't open input file %s\n",filename.c_str());
		exit(1);
	}

	prob.l = 0;
	elements = 0;
	while(1)
	{
		int c = fgetc(fp);
		switch(c)
		{
			case '\n':
				++prob.l;
			case ':':
				++elements;
				break;
			case EOF:
				goto out;
			default:
				;
		}
	}
out:
	rewind(fp);

	prob.y = new double[prob.l];//Malloc(double,prob.l);
	prob.x = new svm_node*[prob.l];// Malloc(Data_Node *,prob.l);
	svm_node* x_space = new svm_node[elements];//Malloc(Data_Node,elements);

	max_index = 0;
	j=0;
	for(i=0;i<prob.l;i++)
	{
		double label;
		prob.x[i] = &x_space[j];
		fscanf(fp,"%lf",&label);
		prob.y[i] = label;
		while(1)
		{
			int c;
			do 
			{
				c = getc(fp);
				if(c=='\n') goto out2;
			} while(isspace(c));
			ungetc(c,fp);
			fscanf(fp,"%d:%f",&(x_space[j].index),&(x_space[j].value));
			++j;
		}	
out2:
		if(j>=1 && x_space[j-1].index > max_index)
			max_index = x_space[j-1].index;
		x_space[j++].index = -1;
	}
	fclose(fp);
}

void numToStr(char* str,int num)
{
	memset(str,0,20);
	sprintf(str,"%d",num);
}
void numToStr(char* str,float num)
{
	memset(str,0,20);
	sprintf(str,"%g",num);
}
void floatEqual()
{
	M3Test test;
	test.testFloat(0);
	test.testFloat(12345);
	test.testFloat(-12345);
	test.testFloat(12345.123);
	test.testFloat(-12345.123);
	test.testFloat(0.1234);
	test.testFloat(-0.1234);
	test.testFloat(0.0123);
	test.testFloat(-0.0123);

}
void floatBound()
{
	M3Test test;

	test.testFloat(-0.00001);
	test.testFloat(0.00001);
	test.testFloat(1.00001);
	test.testFloat(-1.00001);


}
void floatRandom()
{
	M3Test test;
	for(int i=0;i<10;i++)
	{
		float testData = (-(float)rand())/10000;
		test.testFloat(testData);
	}
}
void testFloat()
{	
	floatEqual();
	floatBound();
	floatRandom();
}
void intEqual()
{

	M3Test test;
	test.testInt(1234);
	test.testInt(-1234);
	test.testInt(0);

}
void intBound()
{
	M3Test	test;
	test.testInt(1);
	test.testInt(-1);
	test.testInt(2147483647);
	test.testInt(-2147483647);
	test.testInt(2147483646);
	test.testInt(-2147483646);

}
void intRandom()
{
	M3Test test;
	for(int i=0;i<10;i++)
	{
		int testData = -rand();
		test.testInt(testData);
	}
}
void testInt()
{
	intEqual();
	intBound();
	intRandom();
}
bool ifEqual(string file1,string file2)
{
	ifstream inF(file1.c_str(),ios::in|ios::binary);
	inF.seekg(0L,ios::end);
	size_t len1 = inF.tellg();
	inF.seekg(0L,ios::beg);
	char *buffer1 = new char[len1+1];
	inF.read(buffer1,len1);
	buffer1[len1] = '\0';
	inF.close();
	inF.open(file2.c_str(),ios::in|ios::binary);
	inF.seekg(0L,ios::end);
	size_t len2 = inF.tellg();
	if(len1 != len2)
	{
		delete[] buffer1;
		return false;
	}
	inF.seekg(0L,ios::beg);
	char* buffer2 = new char[len2+1];
	inF.read(buffer2,len2);
	buffer2[len2] = '\0';
	inF.close();
	for(int i=0;i<len1;i++)
	{
		if(buffer1[i]!=buffer2[i])
		{
			delete[] buffer1;
			delete[] buffer2;
			return false;
		}
	}
	delete[] buffer2;
	delete[] buffer1;
	inF.close();
	return true;
}
void testParse()
{
	M3Test test;
	test.testParse("hoss.txt");
}
void testDivide()
{
	M3Test test;
	test.testDivide("divide.txt");
}
void testLibsvm()
{
	M3Test test;
	test.testLibsvm("trainP.txt","trainN.txt","testfile.txt",1,1);
}
void testM3(int argc,char* argv[])
{
	M3::initialize(argc,argv);
	M3::load_train_data(argv[1]);
	M3::divide_train_data();
	M3::training_train_data();
	M3::classify_test_data(argv[2]);
	M3::score_test_data();
	M3::finalize();
}
int main(int argc,char* argv[])
{
//	testFloat();
//	testInt();
//	testParse();
//	testDivide();
//	testLibsvm();
	testM3(argc,argv);
	return 0;
}
