# Mining Order-Preserving Sub-matrices Under Data Uncertainty: A Possible-World Approach



## Compiling the source code.

We provided a Windows and Linux version for compilation.

### Compile and run on Windows

To compile the source code on Windows please use Visual Studio 2015 (or newer). Go to "Windows Version" sub-folder and load the solution to VS then build the solution. A pre-compiled opsm.exe is also provided.
We provided the preprocessed GAL and GDS data set in "testData" folder.
To run the program, go to the folder that contains the opsm.exe on windows command line, e.g. 


```
opsm.exe /testData/GAL_range.txt Apri_PF 200 4 0.6
```

### Compile and run on Linux

To compile the source code on Linux using g++, go to "Linux Version" sub-folder from terminal and use the following command to compile:

```
g++ -O2 convTest.cpp Fourier.cpp  ydTest.cpp -o opsm
```

we provided the preprocessed GAL and GDS data set in "testData" folder, to run the program, e.g.



```
./opsm /testData/GAL_range.txt Apri_PF 200 4 0.6
```


## Parameter List

Here is the detailed meaning of the parameters:

```
./opsm input_file method_name min_row min_col threshold_probabilty (optional for PF and PFA methods only)
```

* [input_file] - the location of the input file
* [method_name] - which method do you want to run {"Apri_ES", "Apri_PF", "Apri_PFA", "DFS_ES", "DFS_PF", "DFS_PFA"} 
			      "Apri" means Apriori based mining, "DFS" means depth first search based mining, "ES" denotes Expected Support, "PF" denotes Probabilistic Frequentness, and "PFA" denotes Probabilistic Frequentness Approximation.
* [min_row] - the minimum number of rows for OPSMs
* [min_col] - the minimum number of columns for OPSMs
* [threshold_probabilty] - a parameter for PF and PFA based methods only, please refer to the paper for detailed meaning, generally you can set any value from [0,1], the bigger the value the less amount of OPSMs will be returned.



## Using the source code
The main() function is in ydTest.cpp. The current main() function is to collect argv parameters as input then run corresponds methods.
The main1() function in ydTest.cpp is the most directly way of calling our methods provided for users' reference. The main1() function servers as example for users trying to call our methods.

