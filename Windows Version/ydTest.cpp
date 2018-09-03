#include <iostream>
#include <fstream>
#include <ctime>
#include <stdlib.h> 




using namespace std;

#include "io.h"
#include "algo.h"

int main(int argc, char** argv)
{


	//argv[1] input file name
	//argv[2] method name
	//argv[3] min row
	//argv[4] min col
	//argv[5] optional th prob

	
	cout << "*************************************************************************" << endl;
	cout << "Please refer to the paper/read_me.txt for method names and more info." << endl;
	cout << "Method: Apri_ES  DFS_ES" << endl;
	cout << "opsm.exe input_file_name method_name min_row min_col " << endl << endl;
	cout << "Method: Apri_PF  DFS_PF  Apri_PFA  DFS_PFA" << endl;
	cout << "opsm.exe input_file_name method_name min_row min_col th_prob" << endl;
	cout << "*************************************************************************" << endl;
	

	cout << argv[0] << endl << argv[1] << endl;
	Matrix mat = loadMatrix(argv[1]);
	
	int minrow = atoi(argv[3]);
	int mincol = atoi(argv[4]);
	

	


	//cout << "minrow= " << minrow << "   mincol=" << mincol << "   thprob=" << th_prob << endl;

	string str = argv[2];
	if (str == "Apri_ES") 
	{
		char outputfileTime[1000];
		char outputfile[1000];
		sprintf(outputfile, "opsm_output_%s_r%d_c%d.txt", argv[2], minrow, mincol);
		sprintf(outputfileTime, "runtime_opsm_output_%s_r%d_c%d.txt", argv[2], minrow, mincol);
		ofstream fout(outputfile);
		clock_t start, end;
		start = clock();

		Apriori_ExpSup(mat, minrow, mincol, fout);
		end = clock();

		cout << "Running time: " << (double)(end - start) / CLOCKS_PER_SEC << " s" << endl;

		ofstream foutTime(outputfileTime);
		foutTime << (double)(end - start) / CLOCKS_PER_SEC << " s" << endl;
		foutTime.close();

		fout.close();
		return 0;
	}

	if (str == "DFS_ES")
	{
		char outputfileTime[1000];
		char outputfile[1000];
		sprintf(outputfile, "opsm_output_%s_r%d_c%d.txt", argv[2], minrow, mincol);
		sprintf(outputfileTime, "runtime_opsm_output_%s_r%d_c%d.txt", argv[2], minrow, mincol);
		ofstream fout(outputfile);
		clock_t start, end;
		start = clock();

		DFS_ExpSup(mat, minrow, mincol, fout);
		end = clock();

		cout << "Running time: " << (double)(end - start) / CLOCKS_PER_SEC << " s" << endl;

		ofstream foutTime(outputfileTime);
		foutTime << (double)(end - start) / CLOCKS_PER_SEC << " s" << endl;
		foutTime.close();

		fout.close();
		return 0;
	}
	double th_prob = atof(argv[5]);
	int th_prob_fileName = th_prob * 100;
	if (str == "Apri_PF")
	{
		char outputfileTime[1000];
		char outputfile[1000];

		sprintf(outputfile, "opsm_output_%s_r%d_c%d_p%d.txt", argv[2], minrow, mincol, th_prob_fileName);
		sprintf(outputfileTime, "runtime_opsm_output_%s_r%d_c%d_p%d.txt", argv[2], minrow, mincol, th_prob_fileName);
		ofstream fout(outputfile);
		clock_t start, end;
		start = clock();
		
		Apriori_ProbFreq(mat, minrow, mincol, th_prob, fout);
		cout << "end" << endl;
		end = clock();

		cout << "Running time: " << (double)(end - start) / CLOCKS_PER_SEC << " s" << endl;

		ofstream foutTime(outputfileTime);
		foutTime << (double)(end - start) / CLOCKS_PER_SEC << " s" << endl;
		foutTime.close();

		fout.close();
		return 0;
	}

	if (str == "DFS_PF")
	{
		char outputfileTime[1000];
		char outputfile[1000];

		sprintf(outputfile, "opsm_output_%s_r%d_c%d_p%d.txt", argv[2], minrow, mincol, th_prob_fileName);
		sprintf(outputfileTime, "runtime_opsm_output_%s_r%d_c%d_p%d.txt", argv[2], minrow, mincol, th_prob_fileName);
		ofstream fout(outputfile);
		clock_t start, end;
		start = clock();

		DFS_ProbFreq(mat, minrow, mincol, th_prob, fout);

		end = clock();

		cout << "Running time: " << (double)(end - start) / CLOCKS_PER_SEC << " s" << endl;

		ofstream foutTime(outputfileTime);
		foutTime << (double)(end - start) / CLOCKS_PER_SEC << " s" << endl;
		foutTime.close();

		fout.close();
		return 0;
	}

	if (str == "Apri_PFA")
	{
		char outputfileTime[1000];
		char outputfile[1000];

		sprintf(outputfile, "opsm_output_%s_r%d_c%d_p%d.txt", argv[2], minrow, mincol, th_prob_fileName);
		sprintf(outputfileTime, "runtime_opsm_output_%s_r%d_c%d_p%d.txt", argv[2], minrow, mincol, th_prob_fileName);
		ofstream fout(outputfile);
		clock_t start, end;
		start = clock();

		Apriori_ProbFreqApprox(mat, minrow, mincol, th_prob, fout);

		end = clock();

		cout << "Running time: " << (double)(end - start) / CLOCKS_PER_SEC << " s" << endl;

		ofstream foutTime(outputfileTime);
		foutTime << (double)(end - start) / CLOCKS_PER_SEC << " s" << endl;
		foutTime.close();

		fout.close();
		return 0;
	}

	if (str == "DFS_PFA")
	{
		char outputfileTime[1000];
		char outputfile[1000];

		sprintf(outputfile, "opsm_output_%s_r%d_c%d_p%d.txt", argv[2], minrow, mincol, th_prob_fileName);
		sprintf(outputfileTime, "runtime_opsm_output_%s_r%d_c%d_p%d.txt", argv[2], minrow, mincol, th_prob_fileName);
		ofstream fout(outputfile);
		clock_t start, end;
		start = clock();

		DFS_ProbFreqApprox(mat, minrow, mincol, th_prob, fout);

		end = clock();

		cout << "Running time: " << (double)(end - start) / CLOCKS_PER_SEC << " s" << endl;

		ofstream foutTime(outputfileTime);
		foutTime << (double)(end - start) / CLOCKS_PER_SEC << " s" << endl;
		foutTime.close();

		fout.close();
		return 0;
	}
	return -1;
}

int main1()
{
	//测试loadMatrix函数
	Matrix mat=loadMatrix("real_range.txt");
	//mat.report();

	int minrow=40;
	int mincol=5;
	double th_prob=0.6;
	ofstream fout("output.txt");

	clock_t start, end;
	start=clock();
	//DFS_ExpSup(mat, minrow, mincol, fout);
	//DFS_ProbFreq(mat, minrow, mincol, th_prob, fout);
	//DFS_ProbFreqApprox(mat, minrow, mincol, th_prob, fout);
	//Apriori_ExpSup(mat, minrow, mincol, fout);
	//Apriori_ProbFreq(mat, minrow, mincol, th_prob, fout);
	Apriori_ProbFreqApprox(mat, minrow, mincol, th_prob, fout);
	end=clock();
	cout<<"Running time: "<<(double)(end-start)/CLOCKS_PER_SEC<<" s"<<endl;
	fout.close();

	cin>>minrow;//用于调试时保持cmd窗口
	return 0;
}
