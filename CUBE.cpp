#include<iostream>
#include<vector>
#include<string>
#include<string.h>
#include <fstream>
#include <sstream>
#include<math.h>
#include <algorithm>
#include <chrono>
#include "CUBE.h"
#include "hash.h"
#include "LSH.h"



using namespace std;
using namespace std::chrono;


CUBE::CUBE(int k, int m, int probes, string inputFile, string outputFile, string queryFile,Metric metric)
{
	_k = k;
	_MC = m;
	_probes = probes;
	_inputFile = inputFile;
	_queryFile = queryFile;
	_outputFile = outputFile;
	
	/*read number of vectors from input file*/
	ifstream inFile(_inputFile);
	int count = 0;
	string line;
	if(metric == euclidean) {
		if(inFile.is_open()) {
			while(getline(inFile, line))
				++count;
		}
		_hashTableSize = log2(count);

	}
	else if(metric == cosine){ 
		_hashTableSize = pow(2,_k);
	}



	_hashTable = new Hash(_hashTableSize,_k);

}

void CUBE::setQueryFileName(string newName)
{
	this-> _queryFile = newName;
}

int CUBE::get_k()
{
	return this->_k;
}

int CUBE::get_M()
{
	return this->_MC;
}

int CUBE::get_probes()
{
	return this->_probes;
}

string CUBE::get_inputFile()
{
	return this->_inputFile;
}

string CUBE::get_outputFile()
{
	return this->_outputFile;
}

int CUBE::executeCUBE(Metric metric)
{
	double R=0;//radius
	string tmp; //for query file
	string line; //used in getline() for file parsing
	double C=1;
	double pValue=0,qValue=0; //read values from input file and query file here
	vector_t vec; //store p_values to vec while reading
	ifstream inputFile(_inputFile);// input file stream
	ifstream queryFile(_queryFile);// query file stream
	ofstream outputFile(_outputFile);
	long int hash_value;
	long double actualHashValue=0; 
	static unsigned int vec_id = 0; // id for each vector readen
	static unsigned int q_id = 0;
	item_t item;
	double max_ratio = -1.0; //max ratio
	double ratio_item = 0.0; //ratio for each query
	cout.precision(9);

	if (inputFile.is_open()) {
		//read vectors from input file and fill hashtables
		while(getline(inputFile, line)) {
			stringstream stream(line);
			while(1){
				stream >> pValue;
				if(!stream)
					break;
				vec.push_back(pValue);
			}
			
			
			item.vec = vec;
			vec_id++;
			item.id = vec_id;
			//generate hash values for given metric
			if(metric == euclidean) {
				hash_value = _hashTable->hashCUBE(vec);
				actualHashValue = ((hash_value % M) + M) % _hashTableSize;
			}
			else{
				hash_value = _hashTable->cosineHash(vec);
				actualHashValue = hash_value;
			}
			///insert item to CUBE structure
			_hashTable->insertItem(item, actualHashValue);
			//clear vec in order to read the next one
			vec.clear();
		}

		//reinitialize vec_id to zero
		vec_id = 0;
		//clear inputFile and start reading again from the beginning for next table
		inputFile.clear();
		inputFile.seekg(0,ios::beg);
		if(inputFile.bad()){
			perror("error");
		}		

	}

	
	// after filling the hashtables start reading the query file
	cout.precision(9);
	if(queryFile.is_open()){ 
		vec.clear();
		//read Radius
		getline(queryFile, line);
		stringstream stream(line);
		stream >> tmp >> R;
		cout << "Radius is: " << R << endl;
		if(R == 0){ // find nearest neighbor
			/*********** Nearest neighbor*********/
			auto start = high_resolution_clock::now();
			while(getline(queryFile, line)) {
				while(line.length() < DIMENSION){
					getline(queryFile, line);
				}
				stringstream stream(line);
				while(1){
					stream >> pValue;
					if(!stream)
						break;
					vec.push_back(pValue);
				}
				
				cout << endl;
				cout << "Query: " << q_id << endl;
				cout << "Nearest neighbor method" << endl;
				q_id++;
				//get max ratio
				ratio_item = nearestNeighbor(vec,metric);
				if(ratio_item > max_ratio){
					cout << "ratio_item: " << ratio_item << endl;
					max_ratio = ratio_item;
				}
				vec.clear();
				
			}
			auto stop = high_resolution_clock::now();
			duration<double> _duration= (stop-start);
			//whole CUBE process time
			cout << "Time = " << _duration.count() << std::fixed << "seconds" << endl;
		}
		else {
			/*********Range Search***********/
			auto start = high_resolution_clock::now();
			while(getline(queryFile, line)) {
				while(line.length() < DIMENSION){
					getline(queryFile, line);
				}
				stringstream stream(line);
				while(1){
					stream >> pValue;
					if(!stream)
						break;
					vec.push_back(pValue);
				}
				
				cout << endl;
				cout << "Query: " << q_id << endl;
				cout << "R-near neighbors method "<< endl;
				q_id++;
				rangeSearch(vec,R,C,metric);
				vec.clear();
			}
			auto stop = high_resolution_clock::now();
			duration<double> _duration= (stop-start);
			//whole LSH process time
			cout << "Time = " << _duration.count() << std::fixed << "seconds" << endl;
		}
	}
		

	cout << "Max_ratio: " << max_ratio << endl;
	cout << "finished" << endl;

}

//displays CUBE structure
void CUBE::displayCUBE()
{
	cout << "display CUBE" << endl;
	_hashTable->displayHash();

}

//Range search in given Radius
void CUBE::rangeSearch(vector_t q, double R, double C=1, Metric metric = euclidean)
{
	int i_l;
	string hash_string;
	long int hash_value;
	long double actualHashValue=0;
	long int tmp_hash;
	int tmp_probes;
	tmp_probes = this->_probes; 
	if(metric == euclidean){ 
		hash_value = _hashTable->hashCUBE(q);
		actualHashValue = ((hash_value % M) + M) % _hashTableSize;
		_hashTable->traverseBucket(q, actualHashValue, R, C , metric);
		
		tmp_probes--;
		tmp_hash = actualHashValue - 1;
		while(tmp_probes > 0){ //start checking left neighbor buckets first
			if(tmp_hash >= 0){
				_hashTable->traverseBucket(q,tmp_hash, R,C,metric);
				tmp_probes--;
				tmp_hash--;
			}
			else{
				break;
			}
		}
		tmp_hash = actualHashValue + 1;
		while(tmp_probes > 0){ //start checking right neighbor buckets,if there are no more left neighbor buckets
			if(tmp_hash < this->_hashTableSize){
				_hashTable->traverseBucket(q,tmp_hash, R,C,metric);
				tmp_probes--;
				tmp_hash++;
			}
			else{
				break;
			}
		}
	}
	else{//cosine
		hash_value = _hashTable->cosineHash(q);
		actualHashValue = hash_value;
		_hashTable->traverseBucket(q, actualHashValue, R, C , metric);
		tmp_probes--;
		tmp_hash = actualHashValue - 1;
		while(tmp_probes > 0){ //start checking left neighbor buckets
			if(tmp_hash >= 0){
				_hashTable->traverseBucket(q, actualHashValue, R, C , metric);
				tmp_probes--;
				tmp_hash--;
			}
			else{
				break;
			}
		}
		tmp_hash = actualHashValue + 1;
		while(tmp_probes > 0){ //start checking righ neighbor buckets
			if(tmp_hash < this->_hashTableSize){
				_hashTable->traverseBucket(q, actualHashValue, R, C , metric);
				tmp_probes--;
				tmp_hash++;
			}
			else{
				break;
			}
		}
	}
}


//NN and approximate NN search
double CUBE::nearestNeighbor(vector_t q,Metric metric)
{
	int i_l;
	string hash_string;
	long int hash_value;
	long double actualHashValue=0; 
	long int tmp_hash;
	int tmp_probes = this->_probes;
	double max_ratio = -5;
	double ratio = 0;
	if(metric == euclidean){ 
		hash_value = _hashTable->hashCUBE(q);// get hash value
		actualHashValue = ((hash_value % M) + M) % _hashTableSize;
		//search NN in bucket
		ratio = (double)_hashTable->nearestNeighborTraverse(q, actualHashValue, this->_MC/3,metric);
		if(ratio > max_ratio){
			max_ratio = ratio;
		}
		tmp_probes--;
		tmp_hash = actualHashValue - 1;
		while(tmp_probes > 0){ //start checking left neighbor buckets
			if(tmp_hash >= 0){
				ratio = _hashTable->nearestNeighborTraverse(q,tmp_hash, this->_MC/3,metric);
				if(ratio != -1){ 
					if(ratio > max_ratio){
						max_ratio = ratio;
					}
				}
				tmp_probes--;
				tmp_hash--;
			}	
			else{
				break;
			}
		}
		tmp_hash = actualHashValue + 1;
		while(tmp_probes > 0){ //start checking right neighbor buckets
			if(tmp_hash < this->_hashTableSize){
				ratio = _hashTable->nearestNeighborTraverse(q,tmp_hash, this->_MC/3,metric);
				if(ratio != -1){ 
					if(ratio > max_ratio){
						max_ratio = ratio;
					}
				}
				tmp_probes--;
				tmp_hash++;
			}
			else{
				break;
			}
		}

	
	}
	else{//cosine
		hash_value = _hashTable->cosineHash(q);
		actualHashValue = hash_value;
		ratio = _hashTable->nearestNeighborTraverse(q, actualHashValue, this->_MC/3,metric);
		if(ratio > max_ratio){
			max_ratio = ratio;
		}
		tmp_probes--;
		tmp_hash = actualHashValue - 1;
		while(tmp_probes > 0){ //start checking left neighbor buckets
			if(tmp_hash >= 0){
				ratio = _hashTable->nearestNeighborTraverse(q,tmp_hash, this->_MC/3,metric);
				if(ratio != -1){ 
					if(ratio > max_ratio){
						max_ratio = ratio;
					}
				}
				tmp_probes--;
				tmp_hash--;
			}
			else{
				break;
			}
		}
		tmp_hash = actualHashValue + 1;
		while(tmp_probes > 0){ //start checking righ neighbor buckets
			if(tmp_hash < this->_hashTableSize){
				ratio = _hashTable->nearestNeighborTraverse(q,tmp_hash, this->_MC/3,metric);
				if(ratio != -1){ 
					if(ratio > max_ratio){
						max_ratio = ratio;
					}
				}
				tmp_probes--;
				tmp_hash++;
			}
			else{
				break;
			}
		}
	}
	//output max ratio
	cout << "nearestNeighbor max_ratio: " << max_ratio << endl;
	return max_ratio;
}



//initialize command line parameters
void initParametersCube(int* k, int* m, int* probes, std::string &input_file, std::string & output_file, std::string & query_file,string &met,int argc, char** argv)
{
	int i;
	//int k_flag = 0, m_flag = 1, input_flag = 1, output_flag = 1, query_flag = 1;
	for(i = 1; i<argc; i++){
		if(strcmp(argv[i], "-o") == 0){
			i++;
			output_file = argv[i];
			//output_flag = 1;
		}
		else if(strcmp(argv[i],"-q") == 0){
			i++;
			query_file = argv[i];
			//query_flag = 1;
		}
		else if(strcmp(argv[i],"-d") == 0){
			i++;
			input_file = argv[i];
			//input_flag = 1;
		}
		else if(strcmp(argv[i],"-k") == 0){
			i++;
			*k=stoi(argv[i]);
			//k_flag = 1;
		}
		else if(strcmp(argv[i],"-L") == 0){
			i++;
			*m=stoi(argv[i]);
			//m_flag = 1;
		}
		else if(strcmp(argv[i],"-metric") == 0){
			i++;
			met = argv[i];
		}
		else if(strcmp(argv[i],"-probes") == 0){
			i++;
			*probes=stoi(argv[i]);
		}
	}
		
}