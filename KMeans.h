#pragma once


#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <list>
#include "hash.h"
#include "Cluster.h"

typedef struct { 
	int K; //number of clusters
	int number_of_hash_functions; //default: L=4
	int number_of_hash_tables; //default: L=5
	std::string input_file;
	std::string output_file;
	std::string conf_file;
	std::string met; //metric
}init_params_t;



class KMeans {
private: 
	int _K; // number of cluster(subsets) for K-Means
	int _max_iter;
	long int _total_items; //number of items read from dataset
	int _num_hash_functions;
	int _num_hashtables;
	std::string _input_file;
	std::string _output_file;
	std::string _conf_file;
	std::string _met; //metric


public:
	KMeans(init_params_t init_params);
	void executeKMeans();
};

void initParametersKMeans(init_params_t *init_params, int argc, char **argv);