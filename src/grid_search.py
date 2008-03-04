#! /usr/bin/python

# =======================================================================================
# Purpose: Grid search to find the best parameter.
# Author: hoss
# Date: 2007.10.21
# Version: 1.0.0
# =======================================================================================


import os
import sys
import re
import random
import time


# =======================================================================================
#									My defination
# =======================================================================================

global LABEL;
global p;
global trainCMD,testCMD;
global class_pare;										# How many one-vs-one problem
global class_compute;									# Create class--number pare
global samp_pos;										# Train file's position

LABEL = "^-?\d*\.?\d*"
p = re.compile(LABEL);
trainCMD = "svm_learn";
testCMD = "svm_classify";

# =======================================================================================
#									Computer error rate
# =======================================================================================


def compare(testfile,result):
	test = open(testfile);
	res = open(result);
	rightN = 0;
	allN = 0;
	res_value = 0;
	while(True):
		tline = test.readline();
		rline = res.readline();
		if(len(tline)==0):
			break;
		allN += 1;
		label = float(p.match(tline).group());
		preL = float(p.match(rline).group());
		if(preL>0):
			preL = 1;
		else:
			preL = -1;
		if(abs(label - preL)<0.000001):
			rightN += 1;
	res.close();
	test.close();
	return float(allN - rightN)/allN;


# =======================================================================================
# 									Preprocess train file
# =======================================================================================


def compute_pare():
	global class_pare,class_compute;
	class_pare = [];
	keyList = class_compute.keys();
	length = len(keyList)-1;
	for i in range(length):
		j = i + 1;
		while (j<=length):
			keyA = keyList[i];
			keyB = keyList[j];
			class_pare.append((keyA,keyB));
			j += 1;

def per_process(filename):
	global class_compute,samp_pos;
	class_compute = {};
	samp_pos = {};
	inf = open(filename);
	pos = inf.tell();
	line = inf.readline();
	while(len(line)!=0):
		label = p.match(line).group();
		if (label in class_compute.keys()):
			class_compute[label] += 1;
			samp_pos[label].append(pos);
		else:
			class_compute[label] = 1;
			samp_pos[label] = [pos]
		pos = inf.tell();
		line = inf.readline();
	inf.close();
	compute_pare();
	

# =======================================================================================
# 									Create subset from one-vs-one problem
# =======================================================================================


def draw_subset(filename, classA, classB, subnum, subsize):
	global p,LABEL,class_compute;
	sA = str(classA);
	sB = str(classB);
	lA = class_compute[sA];
	lB = class_compute[sB];
	for i in range(subnum):
		trainfile = open(filename);
		subsetnameTrain = "_".join([sA,sB,str(i),"train","temp"]);
		subsetnameTest = "_".join([sA,sB,str(i),"test","temp"]);
		outfTrain = open(subsetnameTrain,"w");
		outfTest = open(subsetnameTest,"w");
		breakA = random.randint(0,lA);
		breakB = random.randint(0,lB);
		classAL = samp_pos[sA];
		classBL = samp_pos[sB];
		classAL = classAL[breakA:] + classAL[0:breakA];
		classBL = classBL[breakB:] + classBL[0:breakB];
		i = 0;
		highA = highB = subsize;
		lowA  = class_compute[sA] - subsize;
		lowB = class_compute[sB] - subsize;
		if (subsize > lA):
			highA = lA;
			lowA = 0;
		if (subsize > lB):
			highB = lB;
			lowB = 0;
		while(i<highA):
			trainfile.seek(classAL[i]);
			line = trainfile.readline();
			print >> outfTrain,re.sub(LABEL,"1",line),
			i += 1;
		i = 0;
		while(i<highB):
			trainfile.seek(classBL[i]);
			line = trainfile.readline();
			print >> outfTrain,re.sub(LABEL,"-1",line),
			i += 1;
		i = lA - 1;
		while(i >= lowA):
			trainfile.seek(classAL[i]);
			line = trainfile.readline();
			print >> outfTest,re.sub(LABEL,"1",line),
			i -= 1;
		i = lB - 1;
		while(i >= lowB):
			trainfile.seek(classBL[i]);
			line = trainfile.readline();
			print >> outfTest,re.sub(LABEL,"-1",line),
			i -= 1;
		outfTrain.close();
		outfTest.close();
		trainfile.close();


# =======================================================================================
# 									Find best parameter from one subset
# =======================================================================================


def search_simple(filename,core,ranges):
	global trainCMD,testCMD;
	testC = " ".join([testCMD,filename + "_test_temp","model_temp","result_temp > out_temp"]);
	if core == 0:
		range_c = ranges[0];
		low = range_c[0] - range_c[1];
		high = range_c[0] + range_c[1];
		step = range_c[2];
		error = 1;
		best_c = low;
		while(low < high):
			c_val = 2**low;
			trainC = " ".join([trainCMD,"-t 0 -c",str(c_val),filename + "_train_temp","model_temp > out_temp"]);
			os.system(trainC);
			os.system(testC);
			this_error = compare(filename + "_test_temp","result_temp");
			if(error>this_error):
				best_c = low;
				error = this_error;
				if(error<0.00001):
					break;
			low += step;
		outf = open("para_result","a");
		print >> outf,filename;
		print >> outf,"-c " + str(best_c);
		outf.close();
	elif core == 2:
		range_c = ranges[0];
		range_g = ranges[1];
		c_low = range_c[0] - range_c[1];
		c_high = range_c[0] + range_c[1];
		c_step = range_c[2];
		g_low = range_g[0] - range_g[1];
		g_high = range_g[0] + range_g[1];
		g_step = range_g[2];
		error = 1;
		best_c = c_low;
		best_g = g_low;
		while(c_low < c_high):
			c_val = 2**c_low;
			g_low = range_g[0] - range_g[1];
			while(g_low < g_high):
				g_val = 2**g_low;
				trainC = " ".join([trainCMD,"-t 2 -c",str(c_val),"-g",str(g_val),filename +
						"_train_temp","model_temp > out_temp"]);
				os.system(trainC);
				os.system(testC);
				this_error = compare(filename + "_test_temp","result_temp");
#				print c_low,g_low,this_error;
				if(error > this_error):
					best_c = c_low;
					best_g = g_low;
					error = this_error;
					if(error<0.00001):
						outf = open("para_result","a");
						print >> outf,filename;
						print >> outf,"-c " + str(best_c);
						print >> outf,"-g " + str(best_g);
						outf.close();
						return;
				g_low += g_step;
			c_low += c_step;
		outf = open("para_result","a");
		print >> outf,filename;
		print >> outf,"-c " + str(best_c);
		print >> outf,"-g " + str(best_g);
		outf.close();

		
# =======================================================================================
# 								Find best parameter from one-vs-one problem
# =======================================================================================

		
def find_subset_para(core,classA,classB,subnum,subsize,ranges):
	global trainCMD,testCMD;
	predix = "_".join([str(classA),str(classB)]);
	for i in range(subnum):
		subfilename = "_".join([predix,str(i)]);
		search_simple(subfilename,core,ranges);
	

# =======================================================================================
#								Combine and create final result
# =======================================================================================


def result_process(core):
	if core==0:
		c_value = 0;
		c_num = 0;
		inf = open("para_result");
		line = inf.readline();
		while(len(line)!=0):
			if line[0] == '-':
				c_value += float(line.split(" ")[1]);
				c_num += 1;
			line = inf.readline();
		outf = open("para_final","w");
		res = 2**(float(c_value)/c_num);
		print "-c :",res;
		print >> outf,"-c ",res;
		outf.close();
		inf.close();
	elif core == 2:
		c_value = 0;
		c_num = 0;
		g_value = 0;
		g_num = 0;
		inf = open("para_result");
		line = inf.readline();
		while(len(line) != 0):
			if line[0] == '-':
				if line[1] == 'c':
					c_value += float(line.split(" ")[1]);
					c_num += 1;
				elif line[1] == 'g':
					g_value += float(line.split(" ")[1]);
					g_num += 1;
			line = inf.readline();
		outf = open("para_final","w");
		c_res = 2**(float(c_value)/c_num);
		g_res = 2**(float(g_value)/g_num);
		print "-c :",c_res;
		print "-g :",g_res;
		print >> outf,"-c ",c_res;
		print >> outf,"-g ",g_res;
		outf.close();
		inf.close();


# =======================================================================================
#									Frame
# =======================================================================================


def grid_search(filename,core,subNum,subSize,ranges,pare_num = 0):
	global class_pare,class_compute;
	os.system("rm  %s"%("para_result"));
	per_process(filename);
	allpare = len(class_pare);
	if(pare_num == 0 or pare_num > allpare):
		length = allpare;
		for i in range(length):
			classA = int(class_pare[i][0]);
			classB = int(class_pare[i][1]);
			draw_subset(filename,classA,classB,subNum,subSize);
			find_subset_para(core,classA,classB,subNum,subSize,ranges);
			print "have complated :",str(float(i+1)/length * 100).rjust(5)[0:5]," %";
			os.system("rm %s"%("*temp"));
	else:
		length = pare_num;
		for i in range(length):
			index = random.randint(0,allpare - 1);
			classA = int(class_pare[index][0]);
			classB = int(class_pare[index][1]);
			draw_subset(filename,classA,classB,subNum,subSize);
			find_subset_para(core,classA,classB,subNum,subSize,ranges);
			print "have complated :",str(float(i+1)/length * 100).rjust(5)[0:5]," %";
			os.system("rm %s"%("*temp"));
	result_process(core);


# =======================================================================================
# 								main  process
# =======================================================================================


if __name__ == "__main__":
	ranges = [(5,10,1),(-2,4,1)];
	ifAll = 10;
	subsize = 500;
	subnum = 1;
	type = 0;
	argv = sys.argv;
	filename = argv[1];
	args = len(argv);
	i = 2;
	while(i < args):
		para = argv[i];
		if (para == "-t"):
			type = int(argv[i+1]);
			i += 2;
		elif (para == "-n"):
			subnum = int(argv[i+1]);
			i += 2;
		elif (para == "-s"):
			subsize = int(argv[i+1]);
			i += 2;
		elif (para == "-c"):
			ranges[0] = (float(argv[i+1]),float(argv[i+2]),float(argv[i+3]));
			i += 4;
		elif (para == "-g"):
			ranges[1] = (float(argv[i+1]),float(argv[i+2]),float(argv[i+3]));
			i += 4;
		elif (para == "-ia"):
			ifAll = int(argv[i+1]);
			i += 2;
		else:
			print "wrong parameter! some parameter is default!";
			break;
	print filename,type,subnum,subsize,ranges,ifAll;
	grid_search(filename,type,subnum,subsize,ranges,ifAll);
#print compare("594_316_0_test_temp","svm_predictions");
