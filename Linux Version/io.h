#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
using namespace std;

#include "datStruct.h"

//�����ļ���ʽ��
//��һ��rowNum, colNum
//�ڶ��п�ʼ��ÿ��һ��gene: l1 r1 l2 r2 ...
//��ʾranges: [l1, r1], [l2 ,r2], ...
Matrix loadMatrix(string inFile){
	int row, col;
	ifstream fin(inFile.c_str());
	fin>>row>>col;
	cout<<"num_rows = "<<row<<endl;
	cout<<"num_cols = "<<col<<endl;

	Range** mat = new Range*[row];
	for(int i=0; i<row; i++){
		mat[i]=new Range[col];
		for(int j=0; j<col; j++){
			double l, r;
			fin>>l>>r;
			mat[i][j].set(l, r);
		}
	}
	fin.close();
	cout << "Finish reading input matrix"<< endl;
	return Matrix(mat, row, col);
}

//��һ��seq,expSup
//�ڶ���"rowID,
void outputExpSup(vector<int>& seq, vector<Row>& db, double expSup, ofstream& fout)
{
	//����������ֿ������Ķ�
	for(vector<int>::iterator it=seq.begin(); it!=seq.end(); it++)
	{
		fout<<*it<<" ";
	}
	fout<<","<<expSup;
	fout<<endl;

	//sort(db.begin(), db.end(), rowComp);//############ �������� #############
	for(vector<Row>::iterator it=db.begin(); it!=db.end(); it++)
	{
		fout<<it->rowID<<","<<it->mat->getPr()<<" ";
	}
	fout<<endl;
}

bool rowComp(Row& row1, Row& row2)
{
	return row1.mat->getPr() > row2.mat->getPr();//���ʴӴ�С
}

void outputFreq(vector<int>& seq, vector<Row>& db, double expSup, ofstream& fout)
{
	//����������ֿ������Ķ�
	for(vector<int>::iterator it=seq.begin(); it!=seq.end(); it++)
	{
		fout<<*it<<" ";
	}
	fout<<","<<expSup;
	fout<<endl;

	//sort(db.begin(), db.end(), rowComp);//############ �������� #############
	for(vector<Row>::iterator it=db.begin(); it!=db.end(); it++)
	{
		fout<<it->rowID<<","<<it->mat->getPr()<<" ";
	}
	fout<<endl;
}

/*
void outputExpSup(vector<int>& seq, vector<Row>& db, double expSup, ofstream& fout)
{
	//����������ֿ������Ķ�
	fout<<"seq: ";
	for(vector<int>::iterator it=seq.begin(); it!=seq.end(); it++)
	{
		fout<<*it<<" ";
	}
	fout<<" | expSup = "<<expSup;
	fout<<endl;

	for(vector<Row>::iterator it=db.begin(); it!=db.end(); it++)
	{
		fout<<"("<<it->rowID<<", "<<it->mat->getPr()<<") ";
	}
	fout<<endl;
}

bool rowComp(Row& row1, Row& row2)
{
	return row1.mat->getPr() > row2.mat->getPr();//���ʴӴ�С
}

void outputFreq(vector<int>& seq, vector<Row>& db, double th_prob, ofstream& fout)
{
	//����������ֿ������Ķ�
	fout<<"seq: ";
	for(vector<int>::iterator it=seq.begin(); it!=seq.end(); it++)
	{
		fout<<*it<<" ";
	}
	fout<<endl;

	sort(db.begin(), db.end(), rowComp);//############ �������� #############
	for(vector<Row>::iterator it=db.begin(); it!=db.end(); it++)
	{
		fout<<"("<<it->rowID<<", "<<it->mat->getPr()<<") ";
	}
	fout<<endl;
}
*/