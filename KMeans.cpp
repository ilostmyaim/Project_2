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
using namespace std::chrono;
using std::default_random_engine;


KMeans::KMeans(init_params_t init_params)
{
	this->_K = init_params.K;
	this->_inputFile = init_params.input_file;

	/*raed  total number of vectors from input file*/
	this->_totalItems = 0;
	string line;
	ifstream inFile(this->_inputFile);
	if(inFile.is_open()) {
		while(getline(inFile, line))
			if(!line.empty())
				this->_totalItems++;
	}
	cout << "Total items: " << this->_totalItems << endl;
	
}

void KMeans::randomInitialization()
{
	int i;
	string line;
	uniform_int_distribution<int> distribution(1,this->_totalItems);
	ifstream inFile(_inputFile);
	std::cout.precision(20);
	item_t item;
	if(inFile.is_open()) {
		for(i=1; i<=this->_K; i++) {
			inFile.seekg(0,ios::beg);
			item.id = (distribution(generator));
			
			while(getline(inFile, line)){
				line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
				if(!line.empty()){
					auto delimiterPos = line.find(",");
					/*read vector id*/
					string vector_id = line.substr(0,delimiterPos);
					item.id = stoul(vector_id,nullptr,0);
					/*read actual vector as a string from input file*/
					string actualvector = line.substr(delimiterPos + 1);
					string vector_value;
					/*if id from input file matches generated id then make vector centroid*/
					if(item.id == stoi(vector_id)){
						stringstream stream(actualvector);
						while(getline(stream,vector_value,',')){
							/*convert values of vector from string to double*/
							item.vec.push_back(stod(vector_value));
						}
						/*create cluster*/
						Cluster clusterObject(this->_K,item );
						this->_clusters.push_back(clusterObject);
						/*clear vector for next iteration*/
						item.vec.clear();
					}
				}
			}
			inFile.clear();
		}

	}
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