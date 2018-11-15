#pragma once

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <list>
#include "hash.h"

class Cluster {
private:
	int _cluster_id; //id of a cluster
	vector_t _centroid;
	std::vector<item_t> _items;

public:
	Cluster(int cluster_id, item_t item);
	void addItem(item_t item);
	bool removeItem(unsigned int item_id);
	double getCentroidValue(int index);
	void setCentroidValue(int index,double value);
	item_t getItem(int index);
	int getTotalItems();
	int getID();


};