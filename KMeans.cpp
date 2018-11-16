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
#include<map>
#include "hash.h"
#include "KMeans.h"

using namespace std;
using namespace std::chrono;
using std::default_random_engine;


KMeans::KMeans(init_params_t init_params)
{
	int i;
	string line;
	item_t item;
	this->_K = init_params.K;
	this->_inputFile = init_params.input_file;

	/*raed  total number of vectors from input file*/
	this->_totalItems = 0;
	this->_max_iterations = init_params.max_iterations;
	ifstream inFile(this->_inputFile);

	if(inFile.is_open()) {
		while(getline(inFile, line)){ 
			line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
			if(!line.empty()) {
				auto delimiterPos = line.find(",");
				/*read vector id*/
				string vector_id = line.substr(0,delimiterPos);
				item.id = stoul(vector_id,nullptr,0);
				/*read actual vector as a string from input file*/
				string actualvector = line.substr(delimiterPos + 1);
				string vector_value;
				
				stringstream stream(actualvector);
				while(getline(stream,vector_value,',')){
					/*convert values of vector from string to double*/
					item.vec.push_back(stod(vector_value));
				}
				/*add item to items collection of KMeans*/
				_items.push_back(item);
				/*clear vector for next iteration*/
				item.vec.clear();
				this->_totalItems++;
			}
		}
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
	
	for(i=1; i<=this->_K; i++) {
		if(inFile.is_open()) {
			item.id = (distribution(generator));
			while(getline(inFile, line)){
				line.erase(std::remove_if(line.begin(), line.end(), ::isspace), line.end());
				if(!line.empty()){
					auto delimiterPos = line.find(",");
					/*read vector id*/
					string vector_id = line.substr(0,delimiterPos);
					/*read actual vector as a string from input file*/
					string actualvector = line.substr(delimiterPos + 1);
					string vector_value;
					/*if id from input file matches generated id then make vector centroid*/
					if(item.id == stoul(vector_id,nullptr,0)){
						stringstream stream(actualvector);
						while(getline(stream,vector_value,',')){
							/*convert values of vector from string to double*/
							item.vec.push_back(stod(vector_value));
						}
						cout << "item id: " << item.id << endl;
						print_vector(item.vec);
						/*create cluster*/
						Cluster clusterObject(i,item );
						this->_clusters.push_back(clusterObject);
						/*clear vector for next iteration*/
						item.vec.clear();
						break;
					}
				}
			}
		}
		inFile.clear();
		inFile.seekg(0,ios::beg);

	}
}

bool KMeans::executeKMeans()
{
	int iter;
	cout << "KMeans" << endl;
	for(iter=0; iter<_max_iterations; iter++) {
		cout << "Iteration " << iter << endl;
		//lloyds assignment
		cout << "11111111111111111" << endl;
		bool changed = lloydsAssignment();
		//kMeans update
		cout << "22222222222222222" << endl;
		updateMeans();
		cout << "33333333333333333" << endl;
		if(!changed) {
			cout << "Convergence after " << iter << " iterations" << endl;
			return true;
		}
	}
	return false;
}

bool KMeans::lloydsAssignment()
{
	bool changed = false;
	int i;
	/*iterate over all points,
	find the nearest cluster and assign*/
	for(auto &item : _items) {
		int newCluster = getNearestCluster(item);

		bool val = item.cluster_id != newCluster;
		item.cluster_id = newCluster;
		changed = changed || val;

		cout <<"Item: " << item.id << " assigned to cluster: " << item.cluster_id << endl;
	}
	return changed;
}

int KMeans::getNearestCluster(item_t item)
{
	double sum = 0.0, min_dist=100000000;
	int min_cluster = -1;
	int i;
	//print_vector(item.vec);
	for(i = 0;i<_K;i++){
		double dist=euclideanNorm(item.vec,_clusters[i].getCentroid());
		//cout << "Distance is " << dist << endl;
		if(dist < min_dist) {
			min_dist = dist;
			min_cluster = _clusters[i].getID();
			//cout << "min cluster: " << min_cluster << endl; 
		}
	}

	return min_cluster;
}	

bool KMeans::updateMeans() 
{
	/*create a map of cluster.id, item*/
	multimap<int, const item_t*> itemClusterMap;
	for(const auto &item: _items) {
		auto newPair = std::pair<int, const item_t*>(item.cluster_id, &item);
		itemClusterMap.insert(newPair);
	}	
	cout << "Mphka " << endl;
	/*compute the mean for every cluster*/
	int i;
	for(i=0;i<_K;i++){
		computeMean(itemClusterMap, _clusters[i].getID(), &_clusters[i]);
	}

	return true;
}

void KMeans::computeMean(const multimap<int, const item_t*> &multimap, int clusterID, Cluster *cluster)
{
	int i;
	for(i=0; i<cluster->getCentroidSize();i++){
		cluster->setCentroidValue(i,0.0);
	}
	/*search for all the points in a cluster*/
	auto currentCluster = multimap.equal_range(clusterID);
	int totalItems = 0;

	cout << "before calculate mean" << endl;
	/*calculate mean*/
	for(auto iter=currentCluster.first; iter != currentCluster.second; iter++) {
		cluster->add(*(iter->second));
		totalItems++;
	}
	cout << "before final " << endl;
	cluster->calculateFinal(totalItems);

}

void KMeans::printClusters()
{
	int i;
	item_t item;
	vector_t vec;
	for(i=0;i<this->_K;i++) {
		vec = _clusters[i].getCentroid();
		cout << "Cluster: " << _clusters[i].getID() << endl;
		cout << "Centroid: " << _clusters[i].getCentroidID() << endl;
		print_vector(vec);
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
					else if(name.compare("max_iterations") == 0) {
						init_params->max_iterations = stoi(value);
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