//ʹ�� http://www.codeproject.com/Articles/9388/How-to-implement-the-FFT-algorithm
#pragma once

class CFourier
{
public:
	double pi;
	double *vector;
	CFourier(void);
	~CFourier(void);
	// FFT 1D
	//number_of_samples ������2^n
	void FFT(double data[], unsigned long number_of_samples);//data��re,re,re...
	void IFFT(double vec[], unsigned long number_of_samples);//vec��re,im,re,im...
	//�������vector��
};
