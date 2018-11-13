#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>
#include <cmath>
#include <chrono>
#include "LSH.h"
#include "hash.h"


using namespace std;


int main(int argc, char **argv) {
	vector_t vec;
	string line;

	int k,L;
	string input_file, output_file, query_file, met;
	Metric metric;
	string input;
	/*initialize parameters*/
	initParameters(&k, &L, input_file, output_file, query_file, met, argc, argv);

	/*decide which metric to use*/
	if(met.compare("cosine") == 0)
		metric = cosine;
	else
		metric = euclidean;
	/*create an LSH object*/
	LSH lshObject(k, L, input_file, output_file, query_file,metric);

	ofstream outputFile(output_file);
	streambuf *coutbuf = cout.rdbuf();
	cout.rdbuf(outputFile.rdbuf());

	/*execute LSH for givern input and query files*/
	lshObject.executeLSH(metric);

	while(true) {

		cout.rdbuf(coutbuf);

		cout << "Do you want to use a different query file?(y/n): ";
		getline(cin, input);
		if(input.compare("y") == 0){
			cout << "Enter name/path of query file: ";
			getline(cin, input);

			ofstream outputFile(output_file);
			streambuf *coutbuf = cout.rdbuf();
			cout.rdbuf(outputFile.rdbuf());

			lshObject.setQueryFileName(input);
			lshObject.executeLSH(metric);
			continue;
		}
		else{
			break;
		}
	}
	
	//lshObject.displayLSH();
	
	
	





	return 0;

}
