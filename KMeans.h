#pragma once


#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <list>
#include "hash.h"

class KMeans {
private: 
	int _K; // number of cluster(subsets) for K-Means
	int _max_iter;
	long int _total_items; //number of items read from dataset
	vector<Cluster> _clusters;


public:
	KMeans(int K, int max_iter, long int total_items);
	executeKMeans()
};