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
#include "Cluster.h"
#include "hash.h"

using namespace std;

Cluster::Cluster(int cluster_id, item_t item)
{
	this->_cluster_id = cluster_id;
	for(int i=0; i<(int)item.vec.size(); i++){
		_centroid.push_back(item.vec[i]);	
	}
	_items.push_back(item);

}

void Cluster::addItem(item_t item)
{
	_items.push_back(item);
}

bool Cluster::removeItem(unsigned int item_id)
{
	for(int i=0;i<(int)_items.size();i++){
		if(_items[i].id == item_id){
			_items.erase(_items.begin() + i);
			return true;
		}
	}
	return false;
}

double Cluster::getCentroidValue(int index)
{
	return _centroid[index];
}

void Cluster::setCentroidValue(int index, double value)
{
	_centroid[index] = value;
}

item_t Cluster::getItem(int index)
{
	return _items[index];
}

int Cluster::getTotalItems()
{
	return (int)_items.size();
}

int Cluster::getID()
{
	return this->_cluster_id;
}

