#include<iostream>
#include<vector>
#include<string>
#include<cstring>
#include<string.h>
#include<fstream>
#include<sstream>
#include<math.h>
#include<algorithm>
#include<chrono>
#include "hash.h"
#include "KMeans.h"

using namespace std;

KMeans::KMeans(init_params_t init_params)
{
	this->_K = init_params.K;
	this->_input_file = init_params.input_file;

	/*raed  total number of vectors from input file*/
	this->_total_items = 0;
	string line;
	ifstream inFile(this->_input_file);
	if(inFile.is_open()) {
		while(getline(inFile, line))
			if(!line.empty())
				this->_total_items++;
	}
	cout << "Total items: " << this->_total_items << endl;
	
}



void initParametersKMeans(init_params_t *init_params, int argc, char** argv)
{
	int i;
	string line;
	for(i = 1; i<argc; i++){
		if(strcmp(argv[i], "-o") == 0){
			i++;
			init_params->output_file = argv[i];
		}
		else if(strcmp(argv[i],"-c") == 0){
			i++;
			init_params->conf_file = argv[i];
			ifstream confFile(init_params->conf_file);// input file stream
			if(confFile.is_open()) {
				while(getline(confFile ,line)) {
					line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
					if(line.empty())
						continue;
					auto delimiterPos = line.find(":");
					string name = line.substr(0,delimiterPos);
					string value = line.substr(delimiterPos + 1);
					if(name.compare("number_of_clusters") == 0){
						cout << "Value: " << value << endl;
						init_params->K = stoi(value);
					}
					else if(name.compare("number_of_hash_functions") == 0){
						init_params->number_of_hash_functions = stoi(value);
					}
					else if(name.compare("number_of_hash_tables") == 0){
						init_params->number_of_hash_tables = stoi(value);
					}
				}
			}

		}
		else if(strcmp(argv[i],"-i") == 0){
			i++;
			init_params->input_file = argv[i];
		}
		else if(strcmp(argv[i],"-d") == 0){
			i++;
			init_params->met = argv[i];
		}
	}
}