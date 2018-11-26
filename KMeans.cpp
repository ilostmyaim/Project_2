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
	this->_outputFile = init_params.output_file;
	this->_initChoice = init_params.init_choice;
	this->_assignChoice = init_params.assign_choice;
	this->_updateChoice = init_params.update_choice;
	this->_met = init_params.met;	
	this->_num_hash_functions = init_params.number_of_hash_functions;
	this-> _num_hashtables = init_params.number_of_hash_tables;
	this->_MC = init_params.MC;
	this->_probes = init_params.probes;

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
				item.cluster_id = -1;
				item.distance = -1;
				_items.push_back(item);
				/*clear vector for next iteration*/
				item.vec.clear();
				this->_totalItems++;
			}
		}
	}
	Metric metric;
	if(_met.compare("euclidean") == 0){ 
		metric = euclidean;
		if(_assignChoice == 2){ //if lsh update is selected insert points to hash table
			cout << "Yasss" << endl;
			_LSHObject = new LSH(init_params.number_of_hash_functions, init_params.number_of_hash_tables, init_params.input_file, init_params.output_file, init_params.output_file, metric);
			_LSHObject->insertAllItems(_items,metric);
		}
		else if(_assignChoice == 3) {//cube assignment
			_CUBEObject = new CUBE(init_params.number_of_hash_functions,_MC,_probes,init_params.input_file, init_params.output_file, init_params.output_file, metric);
			_CUBEObject->insertAllItems(_items,metric);
		}
	}
	cout << "Total items: " << this->_totalItems << endl;
}

//Random selection of k points(simplest)
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
						//print_vector(item.vec);
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
	bool changed;
	Metric metric;
	if(_met.compare("euclidean") == 0){
		metric = euclidean;
	}
	else 
		metric = cosine;
	for(iter=0; iter<_max_iterations; iter++) {
		cout << "Iteration " << iter << endl;
		//assignment
		//cout << "11111111111111111" << endl;
		if(_assignChoice == 1){
			changed = lloydsAssignment();
			//printClusters();
			/*
			for(auto &item : _items) {
				cout << "item cluster: " << item.cluster_id << " ";
				cout << "item id: " << item.id  << " ";
			}
			*/
		}
		else if(_assignChoice == 2){//lsh assignment
			//cout << "11111111111111111" << endl;
			
			LSHAssignment();
			//printClusters();
			//_LSHObject->displayLSH();
			//lloydsAssignment();
		}
		else if(_assignChoice == 3){//cube assignment
			CUBEAssignment();
		}
		//update
		//cout << "22222222222222222" << endl;
		if(_updateChoice == 1){
			//cout << "22222222222222222" << endl;
			updateMeans();
		}
		//cout << "33333333333333333" << endl;
		/*
		if(!changed) {
			cout << "Convergence after " << iter << " iterations" << endl;
			return true;
		}
		*/
		
	}

	if(_assignChoice == 2 || _assignChoice == 3){ 
		double mindist=99999999;
		double dist =0;
		/*assign every unassigned point*/
		if(metric == euclidean){ 
			for(auto &item_ : _items) {
				if(item_.cluster_id == -1){ 
					for(int i=0;i<_K;i++){
						dist=euclideanNorm(item_.vec,_clusters[i].getCentroid());
						if(dist < mindist){
							mindist = dist;
							item_.distance = mindist;
							item_.cluster_id = _clusters[i].getID();
							_clusters[item_.cluster_id - 1].insertItem(item_);
						}
					}
				}
			}
		}
	}
	cout << "finished " << endl;
	return false;
}

//Lloyd's assignement
bool KMeans::lloydsAssignment()
{
	bool changed = false;
	int i;
	Metric metric;
	if(_met.compare("euclidean") == 0){
		metric = euclidean;
	}
	else 
		metric = cosine;
	/*iterate over all points,
	find the nearest cluster and assign*/
	for(auto &item : _items) {
		int newCluster = getNearestCluster(item,metric);

		bool val = item.cluster_id != newCluster;
		//if new cluster is different from the old one ,then remove item from old cluster
		if(val == true ){
			if(item.cluster_id == -1) { 
				item.cluster_id = newCluster;
				_clusters[item.cluster_id - 1].insertItem(item);
			}
			else {
				_clusters[item.cluster_id - 1].removeItem(item.id);
				item.cluster_id = newCluster;
				_clusters[item.cluster_id - 1].insertItem(item);
			}

		}
		//else do nothing
		
		changed = changed || val;

		//cout <<"Item: " << item.id << " assigned to cluster: " << item.cluster_id << endl;
	}
	return changed;
}

int KMeans::getNearestCluster(item_t item,Metric metric)
{
	double sum = 0.0, min_dist=100000000;
	int min_cluster = -1;
	int i;
	//print_vector(item.vec);
	if(metric == euclidean){ 
		for(i = 0;i<_K;i++){
			double dist=euclideanNorm(item.vec,_clusters[i].getCentroid());
			//cout << "Distance is " << dist << endl;
			if(dist < min_dist) {
				min_dist = dist;
				min_cluster = _clusters[i].getID();
				//cout << "min cluster: " << min_cluster << endl; 
			}
		}
	}
	else{
		for(i = 0;i<_K;i++){
			double dist=cosineSimilarity(item.vec,_clusters[i].getCentroid());
			//cout << "Distance is " << dist << endl;
			if(dist < min_dist) {
				min_dist = dist;
				min_cluster = _clusters[i].getID();
				//cout << "min cluster: " << min_cluster << endl; 
			}
		}
	}

	return min_cluster;
}	

bool KMeans::LSHAssignment()
{
	//first insert items into the data structure
	bool changed = false;
	Metric metric;
	double initialRange = 0;
	vector< vector<item_t> > items;
	
	if(this->_met.compare("euclidean") == 0){
		metric = euclidean;
	}
	else{
		metric = cosine;
	}

	initialRange = initialRangeLSH(metric);
	
	for(int counter=0;counter<5;counter++){ 
		for (int i = 0; i < _clusters.size(); i++){
			items = _LSHObject->rangeSearch(_clusters[i].getCentroid(),_clusters[i].getID(),initialRange,1,metric);
			for(int j=0;j<items.size();j++){
				for(int j_j=0;j_j<items[j].size();j_j++) {
					//cout << "items["<<j<<"]"<<"["<<j_j<<"]"<<".cluster_id: " << items[j][j_j].cluster_id << endl;;
					for(auto &item_ : _items) {

						bool val = item_.cluster_id != items[j][j_j].cluster_id;
						//
						//if new cluster is different from the old one ,then remove item from old cluster
						if(val == true ){
							if(item_.id == items[j][j_j].id) { 
								if(item_.cluster_id == -1){ 
									item_.cluster_id = items[j][j_j].cluster_id;
									item_.distance = items[j][j_j].distance;
									_clusters[item_.cluster_id - 1].insertItem(item_);
								}
								else {
									if(items[j][j_j].distance < item_.distance) {
										_clusters[item_.cluster_id - 1].removeItem(item_.id);
										item_.cluster_id = items[j][j_j].cluster_id;
										item_.distance = items[j][j_j].distance;
										_clusters[item_.cluster_id - 1].insertItem(item_);
									}
								}
							}
						}
						//else do nothing
						changed = changed || val;

						//cout <<"Item: " << item_.id << " assigned to cluster: " << item_.cluster_id << " ";
					}
				}
			}
			//cout << endl;

			items.clear();
		}
		
		initialRange = initialRange*2;
	}


	return true;
}


double KMeans::initialRangeLSH(Metric metric)
{
	double minDistance = 900000000;
	double distance =0;
	if(metric == euclidean){ 
		for (int i = 0; i < _clusters.size()-1;i++){
			for (int j = i+1; j < _clusters.size(); j++){
				distance = euclideanNorm(_clusters[i].getCentroid(),_clusters[j].getCentroid());
				if(distance < minDistance){
					minDistance = distance;
				}
			}
		}
	}

	return minDistance/double(2.0);
}

bool KMeans::CUBEAssignment()
{
	//first insert items into the data structure
	bool changed = false;
	Metric metric;
	double initialRange = 0;
	vector< vector<item_t> > items;
	
	if(this->_met.compare("euclidean") == 0){
		metric = euclidean;
	}
	else{
		metric = cosine;
	}

	initialRange = initialRangeLSH(metric);
	for(int counter=0;counter<5;counter++) {
		for(int i=0; i<_clusters.size();i++){
			items = _CUBEObject->rangeSearch(_clusters[i].getCentroid(),_clusters[i].getID(),initialRange,1,metric);
			for(int j=0;j<items.size();j++) {
				for(int j_j=0;j_j<items[j].size();j_j++){
					for(auto &item_ : _items) {
						bool val = item_.cluster_id != items[j][j_j].cluster_id;
						//if new cluster is different from the old one ,then remove item from old cluster
						if(val == true ){
							if(item_.id == items[j][j_j].id) { 
								if(item_.cluster_id == -1){ 
									item_.cluster_id = items[j][j_j].cluster_id;
									item_.distance = items[j][j_j].distance;
									_clusters[item_.cluster_id - 1].insertItem(item_);
								}
								else {
									if(items[j][j_j].distance < item_.distance) {
										_clusters[item_.cluster_id - 1].removeItem(item_.id);
										item_.cluster_id = items[j][j_j].cluster_id;
										item_.distance = items[j][j_j].distance;
										_clusters[item_.cluster_id - 1].insertItem(item_);
									}
								}
							}
						}
						changed = changed || val;
					}
				}
			}
			items.clear();
		}
		initialRange = initialRange*2;
	}

	return true;
}
//K-means update
bool KMeans::updateMeans() 
{
	/*create a map of cluster.id, item*/
	multimap<int, const item_t*> itemClusterMap;
	for(const auto &item: _items) {
		auto newPair = std::pair<int, const item_t*>(item.cluster_id, &item);
		itemClusterMap.insert(newPair);
	}	
	/*compute the mean for every cluster*/

	int i;
	for(i=0;i<_K;i++){
		//cout << "Edw " << i << " ";
		computeMean(itemClusterMap, _clusters[i].getID(), &_clusters[i]);
	}
	//cout << "Mphka 2" << endl;
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

	//cout << "before calculate mean" << endl;
	/*calculate mean*/
	for(auto iter=currentCluster.first; iter != currentCluster.second; iter++) {
		cluster->add(*(iter->second));
		totalItems++;
	}
	//cout << "before final " << endl;
	cluster->calculateFinal(totalItems);

}

void KMeans::printClusters()
{
	int i;
	item_t item;
	vector_t vec;
	int sum = 0;
	for(i=0;i<this->_K;i++) {
		vec = _clusters[i].getCentroid();
		cout << "Cluster: " << _clusters[i].getID() << endl;
		sum += _clusters[i].getTotalItems();
		cout << "Number of items in cluster: " << _clusters[i].getTotalItems() << endl;
		//cout << "Centroid: " << _clusters[i].getCentroidID() << endl;
		//print_vector(vec);
	}
	cout << "Total Items in clusters: " << sum << endl;
	cout << "ai: " <<  _ai.size() << endl;
	cout << "bi: " << _bi.size() << endl;
	cout << "Silhouette: " << this->_avgSilhouette << endl;;
}

double KMeans::computeSilhouette()
{
	double avgSil = 0,sum=0;
	int i,j,ix,jx;
	double avgDistSecond = 0;
	double avgDist_b = 0,totalAvg_b = 0, bi = 0;
	double minDist = 900000000000;
	double minID ;
	double dist = 0;
	vector_t centroidNeighbor;
	double clusterTotalItems;
	double clusterTotalItemsNeighbor;
	vector<double> bi_values;
	item_t item;
	for(i=0;i < _K ; i++) { //for each cluster
		//cout << "outer _K: " << i << " ";
		//cout << "Cluster " << i << endl;
		clusterTotalItems = _clusters[i].getTotalItems();
		//cout << "clusterTotalItems " <<  clusterTotalItems << endl;
		if(clusterTotalItems > 0){ 
			for(ix =  0;ix<clusterTotalItems;ix++){ //for every item in a cluster,calculate b(i)
				avgDist_b = 0;
				item = _clusters[i].getItem(ix);
				for(j=0;j<_K;j++) { //find second nearest cluster
					//cout << "inner_K: " << j << " ";
					if(i != j){
						//cout <<"i: " << i << " j: " << j << " ";
						centroidNeighbor = _clusters[j].getCentroid();
						if(_clusters[j].getTotalItems() > 0) { //if cluster is not empty
							dist = euclideanNorm(item.vec,centroidNeighbor);
							if(dist < minDist){
								minDist = dist;
								minID = j;
							}
						}
					}
				}
				//after finding second nearest cluster ,calculate b(i)
				//get second nearest centroid
				centroidNeighbor = _clusters[minID].getCentroid();
				//cout << "Neighbor: " << minID << " " ;
				clusterTotalItemsNeighbor = _clusters[minID].getTotalItems();
				//cout << "clusterTotalItemsNeighbor " << clusterTotalItemsNeighbor << endl;
				for(jx=0;jx<clusterTotalItemsNeighbor;jx++) {
					avgDist_b += euclideanNorm(item.vec, _clusters[minID].getItem(jx).vec);
				}
				totalAvg_b = avgDist_b / clusterTotalItemsNeighbor;
				bi_values.push_back(totalAvg_b);
			}
			//calculate average bi for cluster
			for(j=0;j<bi_values.size();j++) {
				bi += bi_values[j];
			}
	
			bi = bi / double(bi_values.size());
			//cout << "bi: " << bi << " " ;
			//average bi for each cluster
			_bi.push_back(bi);
			_ai.push_back(_clusters[i].computeAvgDistance());

		}
		else{
			cout << "empty cluster: " << i << endl;
		}
		
	}
	
	//cout << "eftasa" << endl;
	for(i=0;i<this->_bi.size();i++) {
		if(_ai[i] >= _bi[i]) {
			sum += ((_bi[i] - _ai[i]) / _ai[i]);
		}
		else {
			sum += ((_bi[i] - _ai[i]) / _bi[i]);
		}
	}
	this->_avgSilhouette = sum / this->_ai.size();
	return 0;
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
					else if(name.compare("init") == 0) {
						init_params->init_choice = stoi(value);
					}
					else if(name.compare("assign") == 0) {
						init_params->assign_choice = stoi(value);
					}
					else if(name.compare("update") == 0) {
						init_params->update_choice = stoi(value);
					}
					else if(name.compare("MC") == 0) {
						init_params->MC = stoi(value);
					}
					else if(name.compare("probes") == 0) {
						init_params->probes = stoi(value);
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