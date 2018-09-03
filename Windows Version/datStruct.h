#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <vector>
using namespace std;

double MISSING_VAL = -999999;

struct Range {
	double l, r;//�����missing value, ��涨l=r=MISSING_VAL

	Range(){}

	Range(double l, double r)
	{
		this->l=l;
		this->r=r;
	}

	void set(double l, double r)
	{
		this->l=l;
		this->r=r;
	}

	bool isMissing()
	{
		return (l == MISSING_VAL);
	}

	string toString()
	{
		if(isMissing()) return "?";
		stringstream ss;
		ss << "[" << l << ", " << r << "]";
		return ss.str();
	}

	bool contains(Range sub)
	{
		return l<=sub.l && r>= sub.r;
	}

	double gap()
	{
		return (r-l);
	}

	double pdf()
	{
		return 1/(r-l);
	}
};

struct Matrix
{
	Range** M;
	int row;
	int col;

	Matrix(Range** M, int row, int col)
	{
		this->M=M;
		this->row=row;
		this->col=col;
	}

	~Matrix()
	{
		for(int i=0; i<row; i++) delete[] M[i];
		delete[] M;
	}

	void report()
	{
		for(int i=0; i<row; i++)
		{
			for(int j=0; j<col; j++)
			{
				cout<<M[i][j].toString()<<"\t";
			}
			cout<<endl;
		}
	}
};

struct DPMatrix
{
	int seqLen;//order�ĳ���
	Range* seq;//order sequence
	int rngNum;//subrange����
	double* splits;//subrange�ָ���
	double** DP;//DP����

	DPMatrix()
	{
		//������Ĭ�ϵģ���Ȼ���������ᱨ��
		seqLen=0;
		seq=NULL;
		rngNum=0;
		splits=NULL;
		DP=NULL;
	}

	DPMatrix(Range& rng)//DP��Ӧ��x=0��base case
	{
		//�����һ��ʱ����
		seqLen=1;
		rngNum=1;

		seq=new Range[1];
		seq[0]=rng;

		splits=new double[2];
		splits[0]=rng.l;
		splits[1]=rng.r;

		DP=new double*[1];
		DP[0]=new double[1];
		DP[0][0]=1;
	}

	double getPr()
	{
		return DP[seqLen-1][rngNum-1];
	}

	void report()
	{
		for(int i=0; i<seqLen; i++)
		{
			for(int j=0; j<rngNum; j++)
			{
				cout<<DP[i][j]<<"\t";
			}
			cout<<endl;
		}
	}

	~DPMatrix()
	{
		for(int i=0; i<seqLen; i++) delete[] DP[i];
		if(DP!=NULL) delete[] DP;
		if(seq!=NULL) delete[] seq;
		if(splits!=NULL) delete[] splits;
		//cout<<seqLen<<" x "<<rngNum<<" freed !"<<endl;//###################
	}
};

struct Row
{//�����һ�У�ά���㷨��Ҫ�ĸ�����Ϣ
	int rowID;//����
	Range lastRng;//���ڴʵ�pruning
	DPMatrix* mat; //���ʼ����õ�DP����, Ϊ�˷�ֹ����block����ͷţ���Ҫ��̬����+�ֶ��ͷ�
};

//================= ���½ṹ����prefixTree =================
struct Node
{
	int last;//���е����һ��Ԫ�أ�rootΪ-1 (level 0)
	vector<Node*> chList;//�ӽڵ�

	Node(int last)
	{
		this->last=last;
	}

	~Node()
	{
		for(vector<Node*>::iterator it=chList.begin(); it!=chList.end(); it++)
		{
			delete *it;
		}
	}

	void appendChild(Node* child)
	{
		chList.push_back(child);
	}

	Node* goToChild(int last)
	{
		for(vector<Node*>::iterator it=chList.begin(); it!=chList.end(); it++)
		{
			if((*it)->last==last) return *it;
		}
		return NULL;
	}
};

Node* initTree()
{
	//return root
	return new Node(-1);
}

bool emptyTree(Node* root)
{
	return (root->chList.size())==0;
}

bool exists(Node* root, vector<int> seq)
{
	Node* cur=root;
	for(vector<int>::iterator it=seq.begin(); it!=seq.end(); it++)
	{
		int last=*it;
		Node* next=cur->goToChild(last);
		if(next==NULL) return false;
		cur=next;
	}
	return true;
}

bool subseqExists(Node* root, vector<int> seq)
{//anti-monoticity pruning
	int size=seq.size();
	for(int i=0; i<size; i++)
	{
		vector<int> subseq;
		for(int j=0; j<size; j++)
		{
			if(j!=i) subseq.push_back(seq[j]);
		}
		if(!exists(root, subseq)) return false;
	}
	return true;
}

//ǰ��ĺ�������curRoot
//����ĺ�������nextRoot
void addSeq(Node* root, vector<int> seq)
{
	Node* cur=root;
	for(vector<int>::iterator it=seq.begin(); it!=seq.end(); it++)
	{
		int last=*it;
		Node* next=cur->goToChild(last);
		if(next==NULL)
		{
			next=new Node(last);
			cur->appendChild(next);
		}
		cur=next;
	}
}

//�������ڵ���
void printTree(int level, Node* cur)
{
	for(int i=0; i<level; i++) cout<<"=== ";
	cout<<cur->last<<endl;
	vector<Node*> list=cur->chList;
	for(vector<Node*>::iterator it=list.begin(); it!=list.end(); it++)
	{
		printTree(level+1, *it);
	}
}

void printTree(Node* root)
{
	vector<Node*> list=root->chList;
	for(vector<Node*>::iterator it=list.begin(); it!=list.end(); it++)
	{
		printTree(1, *it);
	}
}
