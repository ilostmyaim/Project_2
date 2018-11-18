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
	this->_centroid_id = item.id;
	for(int i=0; i<(int)item.vec.size(); i++){
		_centroid.push_back(item.vec[i]);	
	}

}

/*
void Cluster::addItem(item_t item)
{
	_items.push_back(item);
}
*/


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


vector_t Cluster::getCentroid()
{
	return _centroid;
}

void Cluster::setCentroidValue(int index, double value)
{
	_centroid[index] = value;
}

/*
item_t Cluster::getItem(int index)
{
	return _items[index];
}
*/


int Cluster::getTotalItems()
{
	return (int)_items.size();
}


int Cluster::getID()
{
	return this->_cluster_id;
}

int Cluster::getCentroidSize()
{
	return (int)this->_centroid.size();
}

unsigned int Cluster::getCentroidID()
{
	return this->_centroid_id;
}

void Cluster::add(const item_t &item)
{
	int i = 0;
	for(i=0;i<_centroid.size();i++){
		_centroid[i] += item.vec[i];
	}
}

void Cluster::calculateFinal(int totalItems)
{
	unsigned int i;
	for(i=0;i<(int)_centroid.size();i++){
		_centroid[i] /= double(totalItems);
	}
}

/*insert item to cluster*/
void Cluster::insertItem(item_t item)
{
	this->_items.push_back(item);
}
