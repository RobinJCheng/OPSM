#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <stack>
#include <algorithm>

#include <sstream>
#include <iomanip>
#include <stdexcept>
using namespace std;

#include "datStruct.h"
#include "io.h"
#include "convol.h"

#define EXP 2.71828182845904523536
#define PI 3.14159265397



double RationalApproximation(double t)
{
	// Abramowitz and Stegun formula 26.2.23.
	// The absolute value of the error should be less than 4.5 e-4.
	double c[] = { 2.515517, 0.802853, 0.010328 };
	double d[] = { 1.432788, 0.189269, 0.001308 };
	return t - ((c[2] * t + c[1])*t + c[0]) /
		(((d[2] * t + d[1])*t + d[0])*t + 1.0);
}

double NormalCDFInverse(double p)
{
	if (p <= 0.0 || p >= 1.0)
	{
		std::stringstream os;
		os << "Invalid input argument (" << p
			<< "); must be larger than 0 but less than 1.";
		throw std::invalid_argument(os.str());
	}

	// See article above for explanation of this section.
	if (p < 0.5)
	{
		// F^-1(p) = - G^-1(p)
		return -RationalApproximation(sqrt(-2.0*log(p)));
	}
	else
	{
		// F^-1(p) = G^-1(1-p)
		return RationalApproximation(sqrt(-2.0*log(1 - p)));
	}
}

//=============== ����ΪDP����ʵ��㷨 ===============

//����t1<...<ti��Ӧ��DPMatrix (in), ��t1<...<ti<t(i+1)��Ӧ��DPMatrix (out)
void appendRng(DPMatrix &out, DPMatrix &in, Range& rng)//rng=t(i+1)
{
	out.seqLen=in.seqLen+1;
	out.seq=new Range[out.seqLen];
	for(int i=0; i<in.seqLen; i++) out.seq[i]=in.seq[i];
	out.seq[in.seqLen]=rng;
	//��������Ϣ������+splits vec
	vector<double> newSplits;
	newSplits.reserve(in.rngNum+2);
	int pos1=0;//in.splits���±�
	int pos2=0;//0��ʾrng.l, 1��ʾrng.r
	//merge "in.splits"��"{rng.l, rng.r}" (ȥ�ظ�)
	while(pos1<=in.rngNum && pos2<2)
	{
		double v1=in.splits[pos1];
		double v2=((pos2 == 0)?rng.l:rng.r);
		if(v1<v2)
		{
			newSplits.push_back(v1);
			pos1++;
		}
		else//v1>=v2
		{
			newSplits.push_back(v2);
			pos2++;
			if(v1==v2) pos1++;//ȥ�ظ�
		}
	}
	while(pos1<=in.rngNum)
	{
		double v1=in.splits[pos1];
		newSplits.push_back(v1);
		pos1++;
	}
	while(pos2<2)
	{
		double v2=((pos2 == 0)?rng.l:rng.r);
		newSplits.push_back(v2);
		pos2++;
	}
	//��������
	out.rngNum=newSplits.size()-1;
	//����newSplits -> out.splits
	out.splits=new double[newSplits.size()];
	vector<double>::iterator it;
	pos1=0;//�鲢��pos1û���ˣ��������øñ���
	for(it=newSplits.begin(); it!=newSplits.end(); it++)
	{
		out.splits[pos1]=*it;
		pos1++;
	}
	//��������DP����
	out.DP=new double*[out.seqLen];
	for(int i=0; i<out.seqLen; i++) out.DP[i]=new double[out.rngNum];
	//���»�ȡ��������(����ÿ��tiһ��entry)
	double* pr=new double[out.seqLen];
	for(int i=0; i<out.seqLen; i++) pr[i]=out.seq[i].pdf();
	//��������DPԪ��ֵ
	for(int i=0; i<out.seqLen-1; i++)//-1��Ϊ���һ��û�ж�����������
	{
		int pos=0;//��in.DP���е�λ��, ��ʱ��Ӧ��<=in.splits[pos+1]�ķ�Χ��
		for(int j=0; j<out.rngNum; j++)
		{
			Range cur(out.splits[j], out.splits[j+1]);
			//range�����<=cur.r�ķ�Χ��
			if(cur.r==in.splits[pos+1])
			{//��������
				out.DP[i][j]=in.DP[i][pos];
				pos++;
			}
			else
			{//�����entry����ʱǰ����Ҫ�õ���entryӦ���Ѿ��������
				double gap=cur.gap();
				if(j==0)//DP��Ӧ��y=0��base case
				{
					double term=1;
					int k=i;//��i��ǰɨ
					while(out.seq[k].contains(cur) && pr[k]>0)//��cur������pdf��Ϊ0
					{
						term = term*pr[k]*gap/(i-k+1);
						k--;
						if(k<0) break;
					}
					if(k<0) out.DP[i][j]=term;
					else out.DP[i][j]=0;
				}
				else
				{
					//���¼�����ʲ���
					double sum=out.DP[i][j-1];
					double term=1;//�����subrange�ϵĸ���
					int k=i;//��i��ǰɨ
					while(out.seq[k].contains(cur) && pr[k]>0)//��cur������pdf��Ϊ0
					{
						term = term*pr[k]*gap/(i-k+1);
						sum += k>0?term*out.DP[k-1][j-1]:term;
						k--;
						if(k<0) break;
					}
					out.DP[i][j]=sum;
				}
			}
		}
	}
	//�������һ��
	for(int j=0; j<out.rngNum; j++)
	{
		int i=out.seqLen-1;
		Range cur(out.splits[j], out.splits[j+1]);
		double gap=cur.gap();
		if(j==0)//DP��Ӧ��y=0��base case
		{
			double term=1;
			int k=i;//��i��ǰɨ
			while(out.seq[k].contains(cur) && pr[k]>0)//��cur������pdf��Ϊ0
			{
				term = term*pr[k]*gap/(i-k+1);
				k--;
				if(k<0) break;
			}
			if(k<0) out.DP[i][j]=term;
			else out.DP[i][j]=0;
		}
		else
		{
			//���¼�����ʲ���
			double sum=out.DP[i][j-1];
			double term=1;//�����subrange�ϵĸ���
			int k=i;//��i��ǰɨ
			while(out.seq[k].contains(cur) && pr[k]>0)//��cur������pdf��Ϊ0
			{
				term = term*pr[k]*gap/(i-k+1);
				sum += k>0?term*out.DP[k-1][j-1]:term;
				k--;
				if(k<0) break;
			}
			out.DP[i][j]=sum;
		}
	}
	delete[] pr;
}

/*//���º���������ʹ�ð��е�������ķ�������̬����subrange
//������Щ����û��ȥ���ظ���split��
void setSubRanges(vector<double> &output, vector<Range> &input)
{
	//inputΪһ��range
	//outputΪ��range��������range��split points
	output.clear();
	output.reserve(2*input.size());
	vector<Range>::iterator it;
	for(it=input.begin(); it!=input.end(); it++)
	{
		output.push_back(it->l);
		output.push_back(it->r);
	}
	sort(output.begin(), output.end());
}

void setSubRanges(vector<double> &output, Range* row, int col)
{
	//inputΪ�����һ��, colΪ����
	//outputΪ��range��������range��split points
	output.clear();
	output.reserve(2*col);
	for(int i=0; i<col; i++)
	{
		output.push_back(row[i].l);
		output.push_back(row[i].r);
	}
	sort(output.begin(), output.end());
}

//���º���ֱ�Ӽ���DP (naive)
void DP(DPMatrix& re, vector<Range>& ranges)
{
	vector<double> splits;
	setSubRanges(splits, ranges);
	//��������re.splits
	re.rngNum=splits.size()-1;
	re.splits=new double[re.rngNum+1];
	vector<double>::iterator it;
	int pos=0;
	for(it=splits.begin(); it!=splits.end(); it++)
	{
		re.splits[pos]=*it;
		pos++;
	}
	//��������re.seq
	re.seqLen=ranges.size();
	re.seq=new Range[re.seqLen];
	for(int i=0; i<re.seqLen; i++) re.seq[i]=ranges[i];
	//���»�ȡ��������(����ÿ��tiһ��entry)
	double pr[re.seqLen];
	for(int i=0; i<re.seqLen; i++) pr[i]=re.seq[i].pdf();
	//��������re.DP
	re.DP=new double*[re.seqLen];
	for(int i=0; i<re.seqLen; i++) re.DP[i]=new double[re.rngNum];
	//����
	for(int i=0; i<re.seqLen; i++)
	{
		for(int j=0; j<re.rngNum; j++)
		{
			Range cur(re.splits[j], re.splits[j+1]);
			double gap=cur.gap();
			if(j==0)//base case
			{
				double term=1;
				int k=i;//��i��ǰɨ
				while(re.seq[k].contains(cur) && pr[k]>0)//��cur������pdf��Ϊ0
				{
					term = term*pr[k]*gap/(i-k+1);
					k--;
					if(k<0) break;
				}
				if(k<0) re.DP[i][j]=term;
				else re.DP[i][j]=0;
			}
			else
			{
				//���¼�����ʲ���
				double sum=re.DP[i][j-1];
				double term=1;//�����subrange�ϵĸ���
				int k=i;//��i��ǰɨ
				while(re.seq[k].contains(cur) && pr[k]>0)//��cur������pdf��Ϊ0
				{
					term = term*pr[k]*gap/(i-k+1);
					sum += k>0?term*re.DP[k-1][j-1]:term;
					k--;
					if(k<0) break;
				}
				re.DP[i][j]=sum;
			}
		}
	}
}
//*/

//=============== ����ΪDFS Expected Support ===============
Range rngOP(Range& lastRng, Range& nextRng)
{
	double l1=lastRng.l;
	double l2=nextRng.l;
	return Range(l1>l2?l1:l2, nextRng.r);
}

void DFS_ExpSup(vector<int>& seq, vector<Row>& db, vector<int>& dict, Matrix& mat, int minrow, int mincol, ofstream& fout)
{//��鵱ǰseq�Ƿ�frequent, Ȼ��ݹ�
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	/*
	cout<<"Checking seq = ";
	for(vector<int>::iterator it=seq.begin(); it!=seq.end(); it++)
	{
		cout<<*it<<" ";
	}
	cout<<"..."<<endl;
	*/
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//matΪ���ݾ���
	vector<Row> db1;
	vector<Row> db1_pt;//��¼db1�м�¼��Ӧ��db�м�¼
	double sum0=0;
	int col=seq.back();
	for(vector<Row>::iterator it=db.begin(); it!=db.end(); it++)
	{
		int row=it->rowID;
		Range lastRng=mat.M[row][col];
		//��lastRng����pruning
		Range curRng=rngOP(it->lastRng, lastRng);
		if(curRng.r>curRng.l)
		{
			//range��Ϊ��(û�еȺ�, ��ʹ����, ����ҲΪ0)
			Row row1;
			row1.rowID=row;
			row1.lastRng=curRng;
			db1.push_back(row1);
			db1_pt.push_back(*it);
			sum0+=it->mat->getPr();
		}
	}
	if(sum0<minrow) return;//CntPrune
	double sum=0;//����refinement
	int cnt=db1.size();
	for(int i=0; i<cnt; i++)
	{
		db1[i].mat=new DPMatrix();
		appendRng(*db1[i].mat, *db1_pt[i].mat, mat.M[db1[i].rowID][col]);//����seq����>1
		sum+=db1[i].mat->getPr();//�����ۼ�
		sum0-=db1_pt[i].mat->getPr();
		//��Ȼ���Լ���Ƿ�sum>=minrow������Ϊ�˵ݹ�ʱdb1��ȷ�趨���������ⲽpruning
		if(sum0+sum<minrow)//Partial CntPrune, ����ǿ��Եģ���Ϊ����db1�ò�����
		{
			//�ͷ��Ѿ������mat
			for(int j=0; j<=i; j++)
			{
				delete db1[j].mat;
			}
			return;
		}
	}
	if(sum>=minrow)//expSup>=minrow?
	{
		if(seq.size()>=mincol) outputExpSup(seq, db1, sum, fout);//���pattern
		vector<int> dict1;
		//�������ֵ�: ȥ����ǰ����ַ�
		for(vector<int>::iterator it=dict.begin(); it!=dict.end(); it++)
		{
			int cur=*it;
			if(cur!=col) dict1.push_back(cur);
		}
		for(vector<int>::iterator it=dict1.begin(); it!=dict1.end(); it++)
		{
			vector<int> seq1=seq;
			seq1.push_back(*it);
			DFS_ExpSup(seq1, db1, dict1, mat, minrow, mincol, fout);//�ݹ�
		}
	}
	//�ֶ��ͷ�db1��DP�������Ϣ
	for(vector<Row>::iterator it=db1.begin(); it!=db1.end(); it++)
	{
		delete (*it).mat;
	}
}

void DFS_ExpSup(Matrix &mat, int minrow, int mincol, ofstream& fout)
{//�ⲿ���ýӿ�
	//����matrix��ʽ��û��missing data����˳���Ϊ1�Ķ�frequent
	//������� (1) minrow <= row; (2) mincol > 1
	int row=mat.row;
	int col=mat.col;

	for(int i=0; i<col; i++)
	{
		//cout<<"Processing "<<i<<" ..."<<endl;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		vector<int> seq;
		seq.push_back(i);

		vector<Row> db;
		db.reserve(row);
		for(int j=0; j<row; j++)
		{
			Row cur;
			cur.rowID=j;
			cur.lastRng=mat.M[j][i];
			cur.mat=new DPMatrix(cur.lastRng);
			db.push_back(cur);
		}

		vector<int> dict;
		//�������ֵ�: ȥ����ǰ����ַ�
		for(int j=0; j<col; j++)
		{
			if(j!=i) dict.push_back(j);
		}
		//�ݹ�
		for(vector<int>::iterator it=dict.begin(); it!=dict.end(); it++)
		{
			vector<int> seq1=seq;
			seq1.push_back(*it);
			DFS_ExpSup(seq1, db, dict, mat, minrow, mincol, fout);
		}
		//�ͷ�db
		for(vector<Row>::iterator it=db.begin(); it!=db.end(); it++)
		{
			delete (*it).mat;
		}
	}
}

//=============== ����ΪDFS Probablisticaly Frequent ===============

void DFS_ProbFreq(vector<int>& seq, vector<Row>& db, vector<int>& dict, Matrix& mat, int minrow, int mincol, double th_prob, ofstream& fout)
{//��鵱ǰseq�Ƿ�frequent, Ȼ��ݹ�
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	/*
	cout<<"Checking seq = ";
	for(vector<int>::iterator it=seq.begin(); it!=seq.end(); it++)
	{
		cout<<*it<<" ";
	}
	cout<<"..."<<endl;
	//*/
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	double prod = minrow*th_prob;
	//matΪ���ݾ���
	vector<Row> db1;
	vector<Row> db1_pt;//��¼db1�м�¼��Ӧ��db�м�¼
	double sum0=0;
	int col=seq.back();
	for(vector<Row>::iterator it=db.begin(); it!=db.end(); it++)
	{
		int row=it->rowID;
		Range lastRng=mat.M[row][col];
		//��lastRng����pruning
		Range curRng=rngOP(it->lastRng, lastRng);
		if(curRng.r>curRng.l)
		{
			//range��Ϊ��(û�еȺ�, ��ʹ����, ����ҲΪ0)
			Row row1;
			row1.rowID=row;
			row1.lastRng=curRng;
			db1.push_back(row1);
			db1_pt.push_back(*it);
			sum0+=it->mat->getPr();
		}
	}
	double sum=0;//����refinement
	int cnt=db1.size();
	if(cnt<minrow) return;//CntPrune
	if(sum0<prod) return;//MarkovPrune
	double* vec=new double[cnt];//������convolution������
	for(int i=0; i<cnt; i++)
	{
		db1[i].mat=new DPMatrix();
		appendRng(*db1[i].mat, *db1_pt[i].mat, mat.M[db1[i].rowID][col]);//����seq����>1
		vec[i]=db1[i].mat->getPr();
		sum+=vec[i];//�����ۼ�
		sum0-=db1_pt[i].mat->getPr();
		//��Ȼ���Լ���Ƿ�sum>=minrow������Ϊ�˵ݹ�ʱdb1��ȷ�趨���������ⲽpruning
		if(sum0+sum<prod)//MarkovPrune, ����ǿ��Եģ���Ϊ����db1�ò�����
		{
			//�ͷ�convolution�����õ�����
			delete[] vec;
			//�ͷ��Ѿ������mat
			for(int j=0; j<=i; j++)
			{
				delete db1[j].mat;
			}
			return;
		}
	}

	//BEGIN: ExpPrune
	double delta=(minrow-sum-1)/sum;
	if(delta>0)
	{
		bool expPrune=false;
		if(delta>=2*EXP-1)
		{
			if(pow(2, -sum*delta)<th_prob) expPrune=true;
		}
		else
		{
			if(exp(-sum*delta*delta/4)<th_prob) expPrune=true;
		}
		if(expPrune)
		{
			//cout<<"ExpPruned"<<endl;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			//�ͷ�convolution�����õ�����
			delete[] vec;
			//�ֶ��ͷ�db1��DP�������Ϣ
			for(vector<Row>::iterator it=db1.begin(); it!=db1.end(); it++)
			{
				delete (*it).mat;
			}
			return;
		}
	}
	//END: ExpPrune

	//�����Ƿ�probFreq
	double *buf;
	bool freq=PMFCheck(buf, vec, cnt, minrow, th_prob);
	//delete[] buf;

	if(freq)
	{
		if(seq.size()>=mincol) outputFreq(seq, db1, th_prob, fout);//���pattern
		vector<int> dict1;
		//�������ֵ�: ȥ����ǰ����ַ�
		for(vector<int>::iterator it=dict.begin(); it!=dict.end(); it++)
		{
			int cur=*it;
			if(cur!=col) dict1.push_back(cur);
		}
		for(vector<int>::iterator it=dict1.begin(); it!=dict1.end(); it++)
		{
			vector<int> seq1=seq;
			seq1.push_back(*it);
			DFS_ProbFreq(seq1, db1, dict1, mat, minrow, mincol, th_prob, fout);//�ݹ�
		}
	}
	//�ͷ�convolution�����õ�����
	delete[] vec;
	//�ֶ��ͷ�db1��DP�������Ϣ
	for(vector<Row>::iterator it=db1.begin(); it!=db1.end(); it++)
	{
		delete (*it).mat;
	}
}

void DFS_ProbFreq(Matrix &mat, int minrow, int mincol, double th_prob, ofstream& fout)
{//�ⲿ���ýӿ�
	//����matrix��ʽ��û��missing data����˳���Ϊ1�Ķ�frequent
	//������� (1) minrow <= row; (2) mincol > 1
	int row=mat.row;
	int col=mat.col;

	for(int i=0; i<col; i++)
	{
		//cout<<"Processing "<<i<<" ..."<<endl;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		vector<int> seq;
		seq.push_back(i);

		vector<Row> db;
		db.reserve(row);
		for(int j=0; j<row; j++)
		{
			Row cur;
			cur.rowID=j;
			cur.lastRng=mat.M[j][i];
			cur.mat=new DPMatrix(cur.lastRng);
			db.push_back(cur);
		}

		vector<int> dict;
		//�������ֵ�: ȥ����ǰ����ַ�
		for(int j=0; j<col; j++)
		{
			if(j!=i) dict.push_back(j);
		}
		//�ݹ�
		for(vector<int>::iterator it=dict.begin(); it!=dict.end(); it++)
		{
			vector<int> seq1=seq;
			seq1.push_back(*it);
			DFS_ProbFreq(seq1, db, dict, mat, minrow, mincol, th_prob, fout);
		}
		//�ͷ�db
		for(vector<Row>::iterator it=db.begin(); it!=db.end(); it++)
		{
			delete (*it).mat;
		}
	}
}

//=============== ����ΪBFS Expected Support ===============

void bottomCheck_ExpSup(vector<int> seq, Node *curRoot, Node *nextRoot, stack<vector<Row>*>& dbStack, Matrix& mat, int minrow, int mincol, ofstream& fout)
{//����ǰpattern�����frequent��pattern��������pattern����nextRoot
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	/*
	cout<<"Checking seq = ";
	for(vector<int>::iterator it=seq.begin(); it!=seq.end(); it++)
	{
		cout<<*it<<" ";
	}
	cout<<"..."<<endl;
	//*/
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//�������µ�check����Ҫ����pruning
	if(subseqExists(curRoot, seq)==false) return;//���Ƚ���subsequence pruning
	///////////////////////////////
	vector<Row>* db=dbStack.top();
	vector<Row>* db1=new vector<Row>();
	vector<Row> db1_pt;//��¼db1�м�¼��Ӧ��db�м�¼
	double sum0=0;
	int col=seq.back();
	for(vector<Row>::iterator it=(*db).begin(); it!=(*db).end(); it++)
	{
		int row=it->rowID;
		Range lastRng=mat.M[row][col];
		//��lastRng����pruning
		Range curRng=rngOP(it->lastRng, lastRng);
		if(curRng.r>curRng.l)
		{
			//range��Ϊ��(û�еȺ�, ��ʹ����, ����ҲΪ0)
			Row row1;
			row1.rowID=row;
			row1.lastRng=curRng;
			db1->push_back(row1);
			db1_pt.push_back(*it);
			sum0+=it->mat->getPr();//�����ϴε�pr��Ϊ�Ͻ�
		}
	}
	if(sum0<minrow)//CntPrune (�����ϴε�pr��Ϊ�Ͻ�)
	{
		delete db1;
		return;
	}

	int cnt=db1->size();
	double sum=0;
	for(int i=0; i<cnt; i++)
	{
		(*db1)[i].mat=new DPMatrix();
		appendRng(*(*db1)[i].mat, *db1_pt[i].mat, mat.M[(*db1)[i].rowID][col]);//����seq����>1
		sum+=(*db1)[i].mat->getPr();//�����ۼ�
		sum0-=db1_pt[i].mat->getPr();
		//��Ȼ���Լ���Ƿ�sum>=minrow������Ϊ�˵ݹ�ʱdb1��ȷ�趨���������ⲽpruning
		if(sum0+sum<minrow)//Partial CntPrune, ����ǿ��Եģ���Ϊ����db1�ò�����
		{
			//�ͷ��Ѿ������mat
			for(int j=0; j<=i; j++)
			{
				delete (*db1)[j].mat;
			}
			delete db1;
			return;
		}
	}
	if(sum>=minrow)//expSup>=minrow?
	{
		if(seq.size()>=mincol) outputExpSup(seq, *db1, sum, fout);//���pattern
		addSeq(nextRoot, seq);
	}
	//�ֶ��ͷ�db1��DP�������Ϣ
	for(vector<Row>::iterator it=db1->begin(); it!=db1->end(); it++)
	{
		delete (*it).mat;
	}
	delete db1;
}

void recursiveCheck_ExpSup(int k, int level, vector<int> seq, Node *curNode, Node *curRoot, Node *nextRoot, stack<vector<Row>*>& dbStack, Matrix& mat, int minrow, int mincol, vector<int>* dict, ofstream& fout)
{//����ǰ�ڵ㲢�ݹ�
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	/*
	cout<<"Checking seq = ";
	for(vector<int>::iterator it=seq.begin(); it!=seq.end(); it++)
	{
		cout<<*it<<" ";
	}
	cout<<"..."<<endl;
	//*/
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//recursiveCheck�ߵ�����ǰ����·(����curNode->ch)������Ҫpruning(Ҳpruning����)
	//ͬ���ģ�����Ҫ���pattern
	vector<Row>* db=dbStack.top();
	vector<Row>* db1=new vector<Row>();
	int col=seq.back();
	for(vector<Row>::iterator it=(*db).begin(); it!=(*db).end(); it++)
	{
		int row=it->rowID;
		Range lastRng=mat.M[row][col];
		//��lastRng����pruning
		Range curRng=rngOP(it->lastRng, lastRng);
		if(curRng.r>curRng.l)
		{
			//range��Ϊ��(û�еȺ�, ��ʹ����, ����ҲΪ0)
			Row row1;
			row1.rowID=row;
			row1.lastRng=curRng;
			row1.mat=new DPMatrix();
			appendRng(*(row1.mat), *(it->mat), mat.M[row][col]);
			db1->push_back(row1);
		}
	}
	vector<Node*> chList=curNode->chList;
	//�ݹ�
	if(level<k-2)
	{
		for(vector<Node*>::iterator it=chList.begin(); it!=chList.end(); it++)
		{
			Node* child=*it;
			vector<int> seq1=seq;
			seq1.push_back(child->last);
			dbStack.push(db1);
			recursiveCheck_ExpSup(k, level+1, seq1, child, curRoot, nextRoot, dbStack, mat, minrow, mincol, NULL, fout);
			dbStack.pop();
		}
	}
	else if(level==k-2)
	{
		//�ռ�dict
		vector<int> *dict=new vector<int>;
		for(vector<Node*>::iterator it=chList.begin(); it!=chList.end(); it++)
		{
			Node* child=*it;
			dict->push_back(child->last);
		}
		//�ݹ�
		for(vector<Node*>::iterator it=chList.begin(); it!=chList.end(); it++)
		{
			Node* child=*it;
			vector<int> seq1=seq;
			seq1.push_back(child->last);
			dbStack.push(db1);
			recursiveCheck_ExpSup(k, level+1, seq1, child, curRoot, nextRoot, dbStack, mat, minrow, mincol, dict, fout);
			dbStack.pop();
		}
		//k-2���ͷ�dict������Ĳ��ù�
		delete dict;
	}
	else if(level==k-1)
	{
		//�ݹ�
		for(vector<int>::iterator it=dict->begin(); it!=dict->end(); it++)
		{
			if((*it) != seq.back())//ȥ���Ѿ����ֵ���
			{
				vector<int> seq1=seq;
				seq1.push_back(*it);
				dbStack.push(db1);
				bottomCheck_ExpSup(seq1, curRoot, nextRoot, dbStack, mat, minrow, mincol, fout);
				dbStack.pop();
			}
		}
	}
	//�ֶ��ͷ�db1��DP�������Ϣ
	for(vector<Row>::iterator it=db1->begin(); it!=db1->end(); it++)
	{
		delete (*it).mat;
	}
	delete db1;
}

void Apriori_ExpSup(Matrix &mat, int minrow, int mincol, ofstream& fout)
{
	int row=mat.row;
	int col=mat.col;

	//Length 1 tree: ���ڼ���û��missing value����minrow<=row�����ü���Ƿ�frequent
	Node *curRoot=initTree(), *nextRoot;
	for(int i=0; i<col; i++)
	{
		curRoot->appendChild(new Node(i));
	}
	//pattern growth
	for(int k=2; !emptyTree(curRoot); k++)
	{
		//printTree(curRoot);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//cout<<"Generating Length "<<k<<" Patterns ..."<<endl;
		//��ʼ��nextRoot
		nextRoot=initTree();
		//��curRoot����nextRoot
		stack<vector<Row>*> dbStack;
		//�����һ��
		for(vector<Node*>::iterator it=curRoot->chList.begin(); it!=curRoot->chList.end(); it++)
		{
			int i=(*it)->last;
			vector<int> seq;
			seq.push_back(i);

			vector<Row>* db=new vector<Row>;
			db->reserve(row);
			for(int j=0; j<row; j++)
			{
				Row cur;
				cur.rowID=j;
				cur.lastRng=mat.M[j][i];
				cur.mat=new DPMatrix(cur.lastRng);
				db->push_back(cur);
			}
			vector<int> *dict=new vector<int>;
			if(k==2)
			{
				for(int j=0; j<col; j++)
				{
					if(j!=i)
					{
						vector<int> seq1;
						seq1.push_back(i);
						seq1.push_back(j);
						dbStack.push(db);
						bottomCheck_ExpSup(seq1, curRoot, nextRoot, dbStack, mat, minrow, mincol, fout);
						dbStack.pop();
					}
				}
			}
			else if(k==3)
			{
				//�����ֵ�
				vector<int> *dict=new vector<int>;
				for(vector<Node*>::iterator dit=(*it)->chList.begin(); dit!=(*it)->chList.end(); dit++)
				{
					Node* child=*dit;
					dict->push_back(child->last);
				}
				//����dict�ݹ�
				for(vector<Node*>::iterator it1=(*it)->chList.begin(); it1!=(*it)->chList.end(); it1++)
				{
					int j=(*it1)->last;
					vector<int> seq1;
					seq1.push_back(i);
					seq1.push_back(j);
					dbStack.push(db);
					recursiveCheck_ExpSup(k, 2, seq1, *it1, curRoot, nextRoot, dbStack, mat, minrow, mincol, dict, fout);
					dbStack.pop();
				}
				delete dict;
			}
			else
			{
				for(vector<Node*>::iterator it1=(*it)->chList.begin(); it1!=(*it)->chList.end(); it1++)
				{
					int j=(*it1)->last;
					vector<int> seq1;
					seq1.push_back(i);
					seq1.push_back(j);
					dbStack.push(db);
					recursiveCheck_ExpSup(k, 2, seq1, *it1, curRoot, nextRoot, dbStack, mat, minrow, mincol, NULL, fout);
					dbStack.pop();
				}
			}
			//�ͷ�db
			for(vector<Row>::iterator it1=(*db).begin(); it1!=(*db).end(); it1++)
			{
				delete (*it1).mat;
			}
			delete db;
		}
		//���µ�ǰtree
		delete curRoot;
		curRoot=nextRoot;
	}
	delete curRoot;
}

//=============== ����ΪBFS Probablistically Frequent ===============

void bottomCheck_ProbFreq(vector<int> seq, Node *curRoot, Node *nextRoot, stack<vector<Row>*>& dbStack, Matrix& mat, int minrow, int mincol, double th_prob, ofstream& fout)
{//����ǰpattern�����frequent��pattern��������pattern����nextRoot
	//�������µ�check����Ҫ����pruning
	if(subseqExists(curRoot, seq)==false) return;//���Ƚ���subsequence pruning
	///////////////////////////////
	double prod = minrow*th_prob;
	vector<Row>* db=dbStack.top();
	vector<Row>* db1=new vector<Row>();
	vector<Row> db1_pt;//��¼db1�м�¼��Ӧ��db�м�¼
	double sum0=0;
	int col=seq.back();
	for(vector<Row>::iterator it=(*db).begin(); it!=(*db).end(); it++)
	{
		int row=it->rowID;
		Range lastRng=mat.M[row][col];
		//��lastRng����pruning
		Range curRng=rngOP(it->lastRng, lastRng);
		if(curRng.r>curRng.l)
		{
			//range��Ϊ��(û�еȺ�, ��ʹ����, ����ҲΪ0)
			Row row1;
			row1.rowID=row;
			row1.lastRng=curRng;
			db1->push_back(row1);
			db1_pt.push_back(*it);
			sum0+=it->mat->getPr();//�����ϴε�pr��Ϊ�Ͻ�
		}
	}
	double sum=0;//����refinement
	int cnt=db1->size();
	if(cnt<minrow)//CntPrune
	{
		delete db1;
		return;
	}
	if(sum0<prod)//MarkovPrune
	{
		delete db1;
		return;
	}
	double* vec=new double[cnt];//������convolution������
	for(int i=0; i<cnt; i++)
	{
		(*db1)[i].mat=new DPMatrix();
		appendRng(*(*db1)[i].mat, *db1_pt[i].mat, mat.M[(*db1)[i].rowID][col]);//����seq����>1
		vec[i]=(*db1)[i].mat->getPr();
		sum+=vec[i];//�����ۼ�
		sum0-=db1_pt[i].mat->getPr();
		//��Ȼ���Լ���Ƿ�sum>=minrow������Ϊ�˵ݹ�ʱdb1��ȷ�趨���������ⲽpruning
		if(sum0+sum<prod)//MarkovPrune, ����ǿ��Եģ���Ϊ����db1�ò�����
		{
			//�ͷ�convolution�����õ�����
			delete[] vec;
			//�ͷ��Ѿ������mat
			for(int j=0; j<=i; j++)
			{
				delete (*db1)[j].mat;
			}
			delete db1;
			return;
		}
	}

	//BEGIN: ExpPrune
	double delta=(minrow-sum-1)/sum;
	if(delta>0)
	{
		bool expPrune=false;
		if(delta>=2*EXP-1)
		{
			if(pow(2, -sum*delta)<th_prob) expPrune=true;
		}
		else
		{
			if(exp(-sum*delta*delta/4)<th_prob) expPrune=true;
		}
		if(expPrune)
		{
			//�ͷ�convolution�����õ�����
			delete[] vec;
			//�ֶ��ͷ�db1��DP�������Ϣ
			for(vector<Row>::iterator it=db1->begin(); it!=db1->end(); it++)
			{
				delete (*it).mat;
			}
			delete db1;
			return;
		}
	}
	//END: ExpPrune

	//�����Ƿ�probFreq
	double *buf;
	bool freq=PMFCheck(buf, vec, cnt, minrow, th_prob);
	//	delete[] buf;

	if(freq)
	{
		if(seq.size()>=mincol) outputFreq(seq, *db1, th_prob, fout);//���pattern
		addSeq(nextRoot, seq);
	}

	//�ͷ�convolution�����õ�����
	delete[] vec;
	//�ֶ��ͷ�db1��DP�������Ϣ
	for(vector<Row>::iterator it=db1->begin(); it!=db1->end(); it++)
	{
		delete (*it).mat;
	}
	delete db1;
}

void recursiveCheck_ProbFreq(int k, int level, vector<int> seq, Node *curNode, Node *curRoot, Node *nextRoot, stack<vector<Row>*>& dbStack, Matrix& mat, int minrow, int mincol, vector<int>* dict, double th_prob, ofstream& fout)
{//����ǰ�ڵ㲢�ݹ�
	//recursiveCheck�ߵ�����ǰ����·(����curNode->ch)������Ҫpruning(Ҳpruning����)
	//ͬ���ģ�����Ҫ���pattern
	vector<Row>* db=dbStack.top();
	vector<Row>* db1=new vector<Row>();
	int col=seq.back();
	for(vector<Row>::iterator it=(*db).begin(); it!=(*db).end(); it++)
	{
		int row=it->rowID;
		Range lastRng=mat.M[row][col];
		//��lastRng����pruning
		Range curRng=rngOP(it->lastRng, lastRng);
		if(curRng.r>curRng.l)
		{
			//range��Ϊ��(û�еȺ�, ��ʹ����, ����ҲΪ0)
			Row row1;
			row1.rowID=row;
			row1.lastRng=curRng;
			row1.mat=new DPMatrix();
			appendRng(*(row1.mat), *(it->mat), mat.M[row][col]);
			db1->push_back(row1);
		}
	}
	vector<Node*> chList=curNode->chList;
	//�ݹ�
	if(level<k-2)
	{
		for(vector<Node*>::iterator it=chList.begin(); it!=chList.end(); it++)
		{
			Node* child=*it;
			vector<int> seq1=seq;
			seq1.push_back(child->last);
			dbStack.push(db1);
			recursiveCheck_ProbFreq(k, level+1, seq1, child, curRoot, nextRoot, dbStack, mat, minrow, mincol, NULL, th_prob, fout);
			dbStack.pop();
		}
	}
	else if(level==k-2)
	{
		//�ռ�dict
		vector<int> *dict=new vector<int>;
		for(vector<Node*>::iterator it=chList.begin(); it!=chList.end(); it++)
		{
			Node* child=*it;
			dict->push_back(child->last);
		}
		//�ݹ�
		for(vector<Node*>::iterator it=chList.begin(); it!=chList.end(); it++)
		{
			Node* child=*it;
			vector<int> seq1=seq;
			seq1.push_back(child->last);
			dbStack.push(db1);
			recursiveCheck_ProbFreq(k, level+1, seq1, child, curRoot, nextRoot, dbStack, mat, minrow, mincol, dict, th_prob, fout);
			dbStack.pop();
		}
		//k-2���ͷ�dict������Ĳ��ù�
		delete dict;
	}
	else if(level==k-1)
	{
		//�ݹ�
		for(vector<int>::iterator it=dict->begin(); it!=dict->end(); it++)
		{
			if((*it) != seq.back())//ȥ���Ѿ����ֵ���
			{
				vector<int> seq1=seq;
				seq1.push_back(*it);
				dbStack.push(db1);
				bottomCheck_ProbFreq(seq1, curRoot, nextRoot, dbStack, mat, minrow, mincol, th_prob, fout);
				dbStack.pop();
			}
		}
	}
	//�ֶ��ͷ�db1��DP�������Ϣ
	for(vector<Row>::iterator it=db1->begin(); it!=db1->end(); it++)
	{
		delete (*it).mat;
	}
	delete db1;
}

void Apriori_ProbFreq(Matrix &mat, int minrow, int mincol, double th_prob, ofstream& fout)
{
	int row=mat.row;
	int col=mat.col;

	//Length 1 tree: ���ڼ���û��missing value����minrow<=row�����ü���Ƿ�frequent
	Node *curRoot=initTree(), *nextRoot;
	for(int i=0; i<col; i++)
	{
		curRoot->appendChild(new Node(i));
	}
	//pattern growth
	for(int k=2; !emptyTree(curRoot); k++)
	{
		//cout<<"Generating Length "<<k<<" Patterns ..."<<endl;
		//��ʼ��nextRoot
		nextRoot=initTree();
		//��curRoot����nextRoot
		stack<vector<Row>*> dbStack;
		//�����һ��
		for(vector<Node*>::iterator it=curRoot->chList.begin(); it!=curRoot->chList.end(); it++)
		{
			int i=(*it)->last;
			vector<int> seq;
			seq.push_back(i);

			vector<Row>* db=new vector<Row>;
			db->reserve(row);
			for(int j=0; j<row; j++)
			{
				Row cur;
				cur.rowID=j;
				cur.lastRng=mat.M[j][i];
				cur.mat=new DPMatrix(cur.lastRng);
				db->push_back(cur);
			}
			if(k==2)
			{
				for(int j=0; j<col; j++)
				{
					if(j!=i)
					{
						vector<int> seq1;
						seq1.push_back(i);
						seq1.push_back(j);
						dbStack.push(db);
						bottomCheck_ProbFreq(seq1, curRoot, nextRoot, dbStack, mat, minrow, mincol, th_prob, fout);
						dbStack.pop();
					}
				}
			}
			else if(k==3)
			{
				//�����ֵ�
				vector<int> *dict=new vector<int>;
				for(vector<Node*>::iterator dit=(*it)->chList.begin(); dit!=(*it)->chList.end(); dit++)
				{
					Node* child=*dit;
					dict->push_back(child->last);
				}
				//����dict�ݹ�
				for(vector<Node*>::iterator it1=(*it)->chList.begin(); it1!=(*it)->chList.end(); it1++)
				{
					int j=(*it1)->last;
					vector<int> seq1;
					seq1.push_back(i);
					seq1.push_back(j);
					dbStack.push(db);
					recursiveCheck_ProbFreq(k, 2, seq1, *it1, curRoot, nextRoot, dbStack, mat, minrow, mincol, dict, th_prob, fout);
					dbStack.pop();
				}
				delete dict;
			}
			else
			{
				for(vector<Node*>::iterator it1=(*it)->chList.begin(); it1!=(*it)->chList.end(); it1++)
				{
					int j=(*it1)->last;
					vector<int> seq1;
					seq1.push_back(i);
					seq1.push_back(j);
					dbStack.push(db);
					recursiveCheck_ProbFreq(k, 2, seq1, *it1, curRoot, nextRoot, dbStack, mat, minrow, mincol, NULL, th_prob, fout);
					dbStack.pop();
				}
			}
			//�ͷ�db
			for(vector<Row>::iterator it1=(*db).begin(); it1!=(*db).end(); it1++)
			{
				delete (*it1).mat;
			}
			delete db;
		}
		//���µ�ǰtree
		delete curRoot;
		curRoot=nextRoot;
	}
	delete curRoot;
}

//comment: ֻ��bottomCheck�����ǲ�ͬ��
//db����ֻ��pruning+������ã�Ŀǰֻ��dfs��������������


//------------------------------------------approximate PMF

void bottomCheck_ProbFreqApprox(vector<int> seq, Node *curRoot, Node *nextRoot, stack<vector<Row>*>& dbStack, Matrix& mat, int minrow, int mincol, double th_prob, double tm, ofstream& fout)
{//����ǰpattern�����frequent��pattern��������pattern����nextRoot
 //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 /*
 cout<<"Checking seq = ";
 for(vector<int>::iterator it=seq.begin(); it!=seq.end(); it++)
 {
 cout<<*it<<" ";
 }
 cout<<"..."<<endl;
 //*/
 //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 //�������µ�check����Ҫ����pruning
	if (subseqExists(curRoot, seq) == false) return;//���Ƚ���subsequence pruning
													///////////////////////////////
	double prod = minrow * th_prob;

	vector<Row>* db = dbStack.top();
	vector<Row>* db1 = new vector<Row>();
	vector<Row> db1_pt;//��¼db1�м�¼��Ӧ��db�м�¼
	double sum0 = 0;
	int col = seq.back();
	for (vector<Row>::iterator it = (*db).begin(); it != (*db).end(); it++)
	{
		int row = it->rowID;
		Range lastRng = mat.M[row][col];
		//��lastRng����pruning
		Range curRng = rngOP(it->lastRng, lastRng);
		if (curRng.r>curRng.l)
		{
			//range��Ϊ��(û�еȺ�, ��ʹ����, ����ҲΪ0)
			Row row1;
			row1.rowID = row;
			row1.lastRng = curRng;
			db1->push_back(row1);
			db1_pt.push_back(*it);
			sum0 += it->mat->getPr();//�����ϴε�pr��Ϊ�Ͻ�
		}
	}
	if (sum0<minrow)//CntPrune (�����ϴε�pr��Ϊ�Ͻ�)
	{
		delete db1;
		return;
	}
	if (sum0 < prod)
	{
		delete db1;
		return;
	}

	int cnt = db1->size();
	double mu = 0;
	double segmaSquare = 0;
	double tmpProb;
	for (int i = 0; i<cnt; i++)
	{
		(*db1)[i].mat = new DPMatrix();
		appendRng(*(*db1)[i].mat, *db1_pt[i].mat, mat.M[(*db1)[i].rowID][col]);//����seq����>1
		tmpProb = (*db1)[i].mat->getPr();
		mu += tmpProb;//�����ۼ�
		segmaSquare = segmaSquare + tmpProb * (1 - tmpProb);
		sum0 -= db1_pt[i].mat->getPr();
		//��Ȼ���Լ���Ƿ�sum>=minrow������Ϊ�˵ݹ�ʱdb1��ȷ�趨���������ⲽpruning
		if (sum0 + mu<prod)//MarkovPrune, ����ǿ��Եģ���Ϊ����db1�ò�����
		{
			//�ͷ��Ѿ������mat
			for (int j = 0; j <= i; j++)
			{
				delete (*db1)[j].mat;
			}
			delete db1;
			return;
		}
	}
	double t = (minrow - 0.5 - mu) / sqrt(segmaSquare);
	if (t <= tm)//expSup>=minrow?
	{
		if (seq.size() >= mincol) outputFreq(seq, *db1, th_prob, fout);//���pattern
		addSeq(nextRoot, seq);
	}
	//�ֶ��ͷ�db1��DP�������Ϣ
	for (vector<Row>::iterator it = db1->begin(); it != db1->end(); it++)
	{
		delete (*it).mat;
	}
	delete db1;
}

void recursiveCheck_ProbFreqApprox(int k, int level, vector<int> seq, Node *curNode, Node *curRoot, Node *nextRoot, stack<vector<Row>*>& dbStack, Matrix& mat, int minrow, int mincol, vector<int>* dict, double th_prob, double tm, ofstream& fout)
{//����ǰ�ڵ㲢�ݹ�
 //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 /*
 cout<<"Checking seq = ";
 for(vector<int>::iterator it=seq.begin(); it!=seq.end(); it++)
 {
 cout<<*it<<" ";
 }
 cout<<"..."<<endl;
 //*/
 //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 //recursiveCheck�ߵ�����ǰ����·(����curNode->ch)������Ҫpruning(Ҳpruning����)
 //ͬ���ģ�����Ҫ���pattern
	vector<Row>* db = dbStack.top();
	vector<Row>* db1 = new vector<Row>();
	int col = seq.back();
	for (vector<Row>::iterator it = (*db).begin(); it != (*db).end(); it++)
	{
		int row = it->rowID;
		Range lastRng = mat.M[row][col];
		//��lastRng����pruning
		Range curRng = rngOP(it->lastRng, lastRng);
		if (curRng.r>curRng.l)
		{
			//range��Ϊ��(û�еȺ�, ��ʹ����, ����ҲΪ0)
			Row row1;
			row1.rowID = row;
			row1.lastRng = curRng;
			row1.mat = new DPMatrix();
			appendRng(*(row1.mat), *(it->mat), mat.M[row][col]);
			db1->push_back(row1);
		}
	}
	vector<Node*> chList = curNode->chList;
	//�ݹ�
	if (level<k - 2)
	{
		for (vector<Node*>::iterator it = chList.begin(); it != chList.end(); it++)
		{
			Node* child = *it;
			vector<int> seq1 = seq;
			seq1.push_back(child->last);
			dbStack.push(db1);
			recursiveCheck_ProbFreqApprox(k, level + 1, seq1, child, curRoot, nextRoot, dbStack, mat, minrow, mincol, NULL, th_prob, tm, fout);
			dbStack.pop();
		}
	}
	else if (level == k - 2)
	{
		//�ռ�dict
		vector<int> *dict = new vector<int>;
		for (vector<Node*>::iterator it = chList.begin(); it != chList.end(); it++)
		{
			Node* child = *it;
			dict->push_back(child->last);
		}
		//�ݹ�
		for (vector<Node*>::iterator it = chList.begin(); it != chList.end(); it++)
		{
			Node* child = *it;
			vector<int> seq1 = seq;
			seq1.push_back(child->last);
			dbStack.push(db1);
			recursiveCheck_ProbFreqApprox(k, level + 1, seq1, child, curRoot, nextRoot, dbStack, mat, minrow, mincol, dict, th_prob, tm, fout);
			dbStack.pop();
		}
		//k-2���ͷ�dict������Ĳ��ù�
		delete dict;
	}
	else if (level == k - 1)
	{
		//�ݹ�
		for (vector<int>::iterator it = dict->begin(); it != dict->end(); it++)
		{
			if ((*it) != seq.back())//ȥ���Ѿ����ֵ���
			{
				vector<int> seq1 = seq;
				seq1.push_back(*it);
				dbStack.push(db1);
				bottomCheck_ProbFreqApprox(seq1, curRoot, nextRoot, dbStack, mat, minrow, mincol, th_prob, tm, fout);
				dbStack.pop();
			}
		}
	}
	//�ֶ��ͷ�db1��DP�������Ϣ
	for (vector<Row>::iterator it = db1->begin(); it != db1->end(); it++)
	{
		delete (*it).mat;
	}
	delete db1;
}



void Apriori_ProbFreqApprox(Matrix &mat, int minrow, int mincol, double th_prob, ofstream& fout)
{
	int row = mat.row;
	int col = mat.col;

	double tm = NormalCDFInverse((1 - th_prob) / sqrt(2 * PI));
	//cout << tm << endl;

	//Length 1 tree: ���ڼ���û��missing value����minrow<=row�����ü���Ƿ�frequent
	Node *curRoot = initTree(), *nextRoot;
	for (int i = 0; i<col; i++)
	{
		curRoot->appendChild(new Node(i));
	}
	//pattern growth
	for (int k = 2; !emptyTree(curRoot); k++)
	{
		//printTree(curRoot);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		//cout << "Generating Length " << k << " Patterns ..." << endl;
		//��ʼ��nextRoot
		nextRoot = initTree();
		//��curRoot����nextRoot
		stack<vector<Row>*> dbStack;
		//�����һ��
		for (vector<Node*>::iterator it = curRoot->chList.begin(); it != curRoot->chList.end(); it++)
		{
			int i = (*it)->last;
			vector<int> seq;
			seq.push_back(i);

			vector<Row>* db = new vector<Row>;
			db->reserve(row);
			for (int j = 0; j<row; j++)
			{
				Row cur;
				cur.rowID = j;
				cur.lastRng = mat.M[j][i];
				cur.mat = new DPMatrix(cur.lastRng);
				db->push_back(cur);
			}
			vector<int> *dict = new vector<int>;
			if (k == 2)
			{
				for (int j = 0; j<col; j++)
				{
					if (j != i)
					{
						vector<int> seq1;
						seq1.push_back(i);
						seq1.push_back(j);
						dbStack.push(db);
						bottomCheck_ProbFreqApprox(seq1, curRoot, nextRoot, dbStack, mat, minrow, mincol, th_prob, tm, fout);
						dbStack.pop();
					}
				}
			}
			else if (k == 3)
			{
				//�����ֵ�
				vector<int> *dict = new vector<int>;
				for (vector<Node*>::iterator dit = (*it)->chList.begin(); dit != (*it)->chList.end(); dit++)
				{
					Node* child = *dit;
					dict->push_back(child->last);
				}
				//����dict�ݹ�
				for (vector<Node*>::iterator it1 = (*it)->chList.begin(); it1 != (*it)->chList.end(); it1++)
				{
					int j = (*it1)->last;
					vector<int> seq1;
					seq1.push_back(i);
					seq1.push_back(j);
					dbStack.push(db);
					recursiveCheck_ProbFreqApprox(k, 2, seq1, *it1, curRoot, nextRoot, dbStack, mat, minrow, mincol, dict, th_prob, tm, fout);
					dbStack.pop();
				}
				delete dict;
			}
			else
			{
				for (vector<Node*>::iterator it1 = (*it)->chList.begin(); it1 != (*it)->chList.end(); it1++)
				{
					int j = (*it1)->last;
					vector<int> seq1;
					seq1.push_back(i);
					seq1.push_back(j);
					dbStack.push(db);
					recursiveCheck_ProbFreqApprox(k, 2, seq1, *it1, curRoot, nextRoot, dbStack, mat, minrow, mincol, NULL, th_prob, tm, fout);
					dbStack.pop();
				}
			}
			//�ͷ�db
			for (vector<Row>::iterator it1 = (*db).begin(); it1 != (*db).end(); it1++)
			{
				delete (*it1).mat;
			}
			delete db;
		}
		//���µ�ǰtree
		delete curRoot;
		curRoot = nextRoot;
	}
	delete curRoot;
}


//--------------------------------------------DFS_Prob_Freq_Approximation

void DFS_ProbFreqApprox(vector<int>& seq, vector<Row>& db, vector<int>& dict, Matrix& mat, int minrow, int mincol, double th_prob, double tm, ofstream& fout)
{//��鵱ǰseq�Ƿ�frequent, Ȼ��ݹ�
 //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 /*
 cout<<"Checking seq = ";
 for(vector<int>::iterator it=seq.begin(); it!=seq.end(); it++)
 {
 cout<<*it<<" ";
 }
 cout<<"..."<<endl;
 //*/
 //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	double prod = minrow*th_prob;
	//matΪ���ݾ���
	vector<Row> db1;
	vector<Row> db1_pt;//��¼db1�м�¼��Ӧ��db�м�¼
	double sum0 = 0;
	int col = seq.back();
	for (vector<Row>::iterator it = db.begin(); it != db.end(); it++)
	{
		int row = it->rowID;
		Range lastRng = mat.M[row][col];
		//��lastRng����pruning
		Range curRng = rngOP(it->lastRng, lastRng);
		if (curRng.r>curRng.l)
		{
			//range��Ϊ��(û�еȺ�, ��ʹ����, ����ҲΪ0)
			Row row1;
			row1.rowID = row;
			row1.lastRng = curRng;
			db1.push_back(row1);
			db1_pt.push_back(*it);
			sum0 += it->mat->getPr();
		}
	}
	//double sum = 0;//����refinement
	double mu = 0;
	double segmaSquare = 0;
	double tmpProb;
	int cnt = db1.size();
	if (cnt<minrow) return;//CntPrune
	if (sum0<prod) return;//MarkovPrune
	double* vec = new double[cnt];//������convolution������
	for (int i = 0; i<cnt; i++)
	{
		db1[i].mat = new DPMatrix();
		appendRng(*db1[i].mat, *db1_pt[i].mat, mat.M[db1[i].rowID][col]);//����seq����>1


		tmpProb = (db1)[i].mat->getPr();
		mu += tmpProb;//�����ۼ�
		segmaSquare = segmaSquare + tmpProb * (1 - tmpProb);
		sum0 -= db1_pt[i].mat->getPr();
		//��Ȼ���Լ���Ƿ�sum>=minrow������Ϊ�˵ݹ�ʱdb1��ȷ�趨���������ⲽpruning
		if (sum0 + mu<prod)//MarkovPrune, ����ǿ��Եģ���Ϊ����db1�ò�����
		{
			//�ͷ�convolution�����õ�����
			delete[] vec;
			//�ͷ��Ѿ������mat
			for (int j = 0; j <= i; j++)
			{
				delete db1[j].mat;
			}
			return;
		}
	}
	double t = (minrow - 0.5 - mu) / sqrt(segmaSquare);
	if (t <= tm)//expSup>=minrow?
	{
		if (seq.size() >= mincol) outputFreq(seq, db1, th_prob, fout);//���pattern
		vector<int> dict1;
		//�������ֵ�: ȥ����ǰ����ַ�
		for (vector<int>::iterator it = dict.begin(); it != dict.end(); it++)
		{
			int cur = *it;
			if (cur != col) dict1.push_back(cur);
		}
		for (vector<int>::iterator it = dict1.begin(); it != dict1.end(); it++)
		{
			vector<int> seq1 = seq;
			seq1.push_back(*it);
			DFS_ProbFreqApprox(seq1, db1, dict1, mat, minrow, mincol, th_prob, tm, fout);//�ݹ�
		}
	}

	//�ֶ��ͷ�db1��DP�������Ϣ
	for (vector<Row>::iterator it = db1.begin(); it != db1.end(); it++)
	{
		delete (*it).mat;
	}
}

void DFS_ProbFreqApprox(Matrix &mat, int minrow, int mincol, double th_prob, ofstream& fout)
{//�ⲿ���ýӿ�
 //����matrix��ʽ��û��missing data����˳���Ϊ1�Ķ�frequent
 //������� (1) minrow <= row; (2) mincol > 1
	int row = mat.row;
	int col = mat.col;
	double tm = NormalCDFInverse((1 - th_prob) / sqrt(2 * PI));
	for (int i = 0; i<col; i++)
	{
		//cout << "Processing " << i << " ..." << endl;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		vector<int> seq;
		seq.push_back(i);

		vector<Row> db;
		db.reserve(row);
		for (int j = 0; j<row; j++)
		{
			Row cur;
			cur.rowID = j;
			cur.lastRng = mat.M[j][i];
			cur.mat = new DPMatrix(cur.lastRng);
			db.push_back(cur);
		}

		vector<int> dict;
		//�������ֵ�: ȥ����ǰ����ַ�
		for (int j = 0; j<col; j++)
		{
			if (j != i) dict.push_back(j);
		}
		//�ݹ�
		for (vector<int>::iterator it = dict.begin(); it != dict.end(); it++)
		{
			vector<int> seq1 = seq;
			seq1.push_back(*it);
			DFS_ProbFreqApprox(seq1, db, dict, mat, minrow, mincol, th_prob, tm, fout);
		}
		//�ͷ�db
		for (vector<Row>::iterator it = db.begin(); it != db.end(); it++)
		{
			delete (*it).mat;
		}
	}
}