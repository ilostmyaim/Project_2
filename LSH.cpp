#include<iostream>
#include<vector>
#include<string>
#include<cstring>
#include<string.h>
#include <fstream>
#include <sstream>
#include<math.h>
#include <algorithm>
#include <chrono>
#include "LSH.h"
#include "hash.h"



using namespace std;
using namespace std::chrono;

LSH::LSH(int k, int L, string inputFile, string outputFile, string queryFile,Metric metric)
{
	/*initialize parameters*/
	_k = k;
	_L = L;
	_inputFile = inputFile;
	_outputFile = outputFile;
	_queryFile = queryFile;
	_arrayOfHashTables = new Hash*[_L]; //create array of pointers to hashtables
	ifstream inFile(_inputFile);
	int count = 0;
	string line;
	if(metric == euclidean) { //calculate number of input vectors to define table size
		if(inFile.is_open()) {
			while(getline(inFile, line))
				++count;
		}
		_hashTableSize = count / 8;

	}
	else if(metric == cosine){ //for cosine ,table size is 2^k
		_hashTableSize = pow(2,_k);
	}
	for(int i = 0;i<_L;i++){ 
		_arrayOfHashTables[i] = new Hash(_hashTableSize,_k); //create hash tables	
	}
}

void LSH::setQueryFileName(string newName)
{
	this-> _queryFile = newName;
}

LSH::~LSH()
{
	for(int i = 0;i<_L;i++){
		delete _arrayOfHashTables[i];
	}
	delete[] _arrayOfHashTables;
}

//executes LSH 
void LSH::executeLSH(Metric metric)
{
	//do stuff
	int i_l=0; //counter for L and k respectively
	double R=0; //radius
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
	
	for(i_l=0;i_l<_L;i_l++){
		//read vectors from input file and fill hashtables
		if (inputFile.is_open()) {
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
				if(metric == euclidean){
					hash_value = _arrayOfHashTables[i_l]->hash(vec);
					actualHashValue = ((hash_value % M) + M) % _hashTableSize;
				}
				else{
					hash_value = _arrayOfHashTables[i_l]->cosineHash(vec);
					actualHashValue = hash_value;
				}
				///insert item to LSH structure
				_arrayOfHashTables[i_l]->insertItem(item,actualHashValue);
				//clear vec in order to read the next one
				vec.clear();
				
			
			}	

		}
		//reinitialize vec_id to zero after whole input file is read
		vec_id = 0;
		//clear inputFile and start reading again from the beginning for next table
		inputFile.clear();
		inputFile.seekg(0,ios::beg);
		if(inputFile.bad()){
			perror("error");
		}		
	
	}// after filling the hashtables start reading the query file
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
			//whole LSH process time
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
			cout << "Time = " << _duration.count() << std::fixed <<"seconds" << endl;
		}
	}
	
	cout << "Max_ratio: " << max_ratio << endl;
	cout << "finished" << endl;

}

//displays LSH structure
void LSH::displayLSH()
{
	cout << "display lsh" << endl;
	for(int i = 0;i<this->_L;i++){	
		cout << "Printing hashtable["<<i<<"]"<<endl;
		_arrayOfHashTables[i]->displayHash();

	}
}

//Range search in given Radius
void LSH::rangeSearch(vector_t q, double R, double C=1, Metric metric = euclidean)
{
	int i_l;
	string hash_string;
	long int hash_value;
	long double actualHashValue=0; 
	if(metric == euclidean){ 
		for(i_l = 0; i_l < _L; i_l++){ //for each array hash q vector and execute range search
			hash_value = _arrayOfHashTables[i_l]->hash(q);
			actualHashValue = ((hash_value % M) + M) % _hashTableSize;
			_arrayOfHashTables[i_l]->traverseBucket(q, actualHashValue, R, C , metric);
		}
	}
	else{
		for(i_l = 0; i_l < _L; i_l++){
			hash_value = _arrayOfHashTables[i_l]->cosineHash(q);
			actualHashValue = hash_value;
			_arrayOfHashTables[i_l]->traverseBucket(q, actualHashValue, R, C , metric);
		}
	}
}

//NN and approximate NN search
double LSH::nearestNeighbor(vector_t q,Metric metric)
{
	int i_l;
	string hash_string;
	long int hash_value;
	long double actualHashValue=0;
	double max_ratio = -5;
	double ratio = 0;
	if(metric == euclidean){ 
		for(i_l = 0; i_l < _L; i_l++){
			hash_value = _arrayOfHashTables[i_l]->hash(q);
			actualHashValue = ((hash_value % M) + M) % _hashTableSize;
			ratio = (double)_arrayOfHashTables[i_l]->nearestNeighborTraverse(q, actualHashValue, this->_L,metric);
			if(ratio != -1){ 
				if(ratio > max_ratio){
					max_ratio = ratio;
				}
			}
		}
	}
	else{
		for(i_l = 0; i_l < _L; i_l++){
			hash_value = _arrayOfHashTables[i_l]->cosineHash(q);
			actualHashValue = hash_value;
			ratio = (double)_arrayOfHashTables[i_l]->nearestNeighborTraverse(q, actualHashValue, this->_L,metric);
			if(ratio != -1){ 
				if(ratio > max_ratio){
					max_ratio = ratio;
				}
			}
		}
	}
	cout << "nearestNeighbor max_ratio: " << max_ratio << endl;
	return max_ratio;
}

int LSH::sizeofLSH()
{
	int memorySum = 0;
	for(int i = 0;i<_L;i++){
		memorySum += sizeof(_arrayOfHashTables[i]);
	}
	return memorySum;
}


//initializes command line parameters
void initParameters(int* k, int* L, std::string &input_file, std::string & output_file, std::string & query_file,string &met,int argc, char** argv)
{
	int i;
	//int k_flag = 0, L_flag = 1, input_flag = 1, output_flag = 1, query_flag = 1;
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
			*L=stoi(argv[i]);
			//L_flag = 1;
		}
		else if(strcmp(argv[i],"-metric") == 0){
			i++;
			met = argv[i];
		}
	}
}


