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
#include "hash.h"
#include "KMeans.h"

KMeans::KMeans(int K, int max_iter, long int total_items)
{
	this->_K = K;
	this->_max_iter = max_iter;
	this->_total_items = total_items;	
}