#pragma once

#include <iostream>
using namespace std;

#include "Fourier.h"

//========================== ���¼����� ==========================

//convol�Ӻ����� ���鳤��
int getSampleRate(int num)
{
	int sample_rate=2;
	while(sample_rate<num) sample_rate*=2;
	return sample_rate;
}

//convol�Ӻ���: �����˷�
void multiply(double* re, double a, double b, double c, double d)
{//(a+bi)(c+di)
	re[0]=a*c-b*d;
	re[1]=a*d+b*c;
}

//�����Ӻ������������
void printArray(double* a, int num)
{
	for(int i=0; i<num; i++) cout<<a[i]<<" ";
	cout<<endl;
}

double* convNaive(double* v1, int n1, double* v2, int n2)
{
	int n=n1+n2-1;
	double* r=new double[n];
	for(int i=0; i<n; i++) r[i]=0.0;
	for(int i=0; i<n1; i++)
	{
		for(int j=0; j<n2; j++)
		{
			r[i+j]+=v1[i]*v2[j];
		}
	}
	return r;
}

double* convFFT(double* d1, int n1, double* d2, int n2)
{//ʹ�� http://www.codeproject.com/Articles/9388/How-to-implement-the-FFT-algorithm
	int n0=n1+n2-1;
	int n=getSampleRate(n0);

	double* dat1=new double[n];
	for(int i=0; i<n1; i++) dat1[i]=d1[i];
	for(int i=n1; i<n; i++) dat1[i]=0;

	double* dat2=new double[n];
	for(int i=0; i<n2; i++) dat2[i]=d2[i];
	for(int i=n2; i<n; i++) dat2[i]=0;

	CFourier fft1, fft2, ifft;
	fft1.FFT(dat1, n);
	double* v1=fft1.vector;
	fft2.FFT(dat2, n);
	double* v2=fft2.vector;

	double* tmp=new double[2*n];
	double cur[2];
	for(int i=0; i<n; i++)
	{
		multiply(cur, v1[2*i], v1[2*i+1], v2[2*i], v2[2*i+1]);
		tmp[2*i]=cur[0];
		tmp[2*i+1]=cur[1];
	}

	ifft.IFFT(tmp, n);

	//normalize: ./n
	int id=0;
	double* re=new double[2*n0];
	for(int i=0; i<2*n0; i+=2)
	{
		re[id]=ifft.vector[i]/n;
		id++;
	}

	delete[] tmp;
	delete[] dat1;
	delete[] dat2;

	return re;
}

//========================== ���¼���PMF ==========================
double* DP_PMF(double* pr, int num)
{
	//������ʴ�pr,
	//���pmf[num+1] (+1����Ϊ��pmf[0])
	//pr��Ҫ�ں�����delete[]

	double *fx, *f1x;
	fx=new double[1];
	fx[0]=1;
	for(int i=1; i<=num; i++)
	{
		f1x=new double[i+1];
		double cur=pr[i-1];

		f1x[0]=(1-cur)*fx[0];
		for(int j=1; j<i; j++)
		{
			f1x[j]=cur*fx[j-1] + (1-cur)*fx[j];
		}
		f1x[i]=cur*fx[i-1];//fx[i]=0, ����ʵ����û�д洢
		delete[] fx;
		fx=f1x;
	}
	return fx;
}

double cdf(double* pmf, int num, int minsup)//pmfʵ�ʴ�СΪnum+1
{
	double re=0;
	//�Ƚ�1...minsup��minsup+1...num�ĸ���
	if(num-minsup>minsup)
	{
		for(int i=0; i<=minsup; i++) re+=pmf[i];
		return re;
	}
	else
	{
		for(int i=minsup+1; i<=num; i++) re+=pmf[i];
		return 1-re;
	}
}

int size_th = 500; //====== �ɵ�������pr[]����С�ڵ���size_th����DP_PMF����

double* DC_PMF(double* pr, int num)
{
	//������ʴ�pr,
	//���pmf[num+1] (+1����Ϊ��pmf[0])
	//pr��Ҫ�ں�����delete[]
	double* re;

	if(num<=size_th)
	{
		return DP_PMF(pr, num);
	}

	int mid=num/2;
	double* ch1=DC_PMF(pr, mid);
	double* ch2=DC_PMF(pr+mid, num-mid);
	re=convFFT(ch1, mid+1, ch2, num-mid+1);//+1����Ϊ����pmf[0]����˶�һ��
	delete[] ch1;
	delete[] ch2;
	return re;
}

bool PMFCheck(double* &buf, double* pr, int num, int minsup, double minprob)//buf�Ǵ���buffer
{
	if(num<=size_th)
	{
		buf=DP_PMF(pr, num);
		return 1-cdf(buf, num, minsup-1)>=minprob;
	}

	int mid=num/2;
	double *ch1, *ch2;
	if(PMFCheck(ch1, pr, mid, minsup, minprob)) return true;
	if(PMFCheck(ch2, pr+mid, num-mid, minsup, minprob)) return true;
	buf=convFFT(ch1, mid+1, ch2, num-mid+1);//+1����Ϊ����pmf[0]����˶�һ��
	delete[] ch1;
	delete[] ch2;
	return 1-cdf(buf, num, minsup-1)>=minprob;
}


//�ݹ鵽base case
double* DC_PMF0(double* pr, int num)
{
	//������ʴ�pr,
	//���pmf[num+1] (+1����Ϊ��pmf[0])
	//pr��Ҫ�ں�����delete[]
	double* re;

	if(num==1)
	{
		re=new double[2];
		re[0]=1-pr[0];
		re[1]=pr[0];
		return re;
	}

	int mid=num/2;
	double* ch1=DC_PMF0(pr, mid);
	double* ch2=DC_PMF0(pr+mid, num-mid);
	re=convFFT(ch1, mid+1, ch2, num-mid+1);//+1����Ϊ����pmf[0]����˶�һ��
	delete[] ch1;
	delete[] ch2;
	return re;
}

//�ݹ鵽base case
bool PMFCheck0(double* &buf, double* pr, int num, int minsup, double minprob)//buf�Ǵ���buffer
{
	if(num==1)
	{
		buf=new double[2];
		buf[0]=1-pr[0];
		buf[1]=pr[0];
		return 1-cdf(buf, 1, minsup-1)>=minprob;
	}

	int mid=num/2;
	double *ch1, *ch2;
	if(PMFCheck0(ch1, pr, mid, minsup, minprob)) return true;
	if(PMFCheck0(ch2, pr+mid, num-mid, minsup, minprob)) return true;
	buf=convFFT(ch1, mid+1, ch2, num-mid+1);//+1����Ϊ����pmf[0]����˶�һ��
	delete[] ch1;
	delete[] ch2;
	return 1-cdf(buf, num, minsup-1)>=minprob;
}
