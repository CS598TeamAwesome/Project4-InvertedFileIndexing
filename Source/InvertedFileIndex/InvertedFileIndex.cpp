#include <algorithm>
#include <cstdlib>
#include <utility>
#include "InvertedFileIndex.hpp"
#include <iostream>
#include <array>
#include <map>
#include <iterator>

using namespace InvertedFileIndexing;
using namespace std;

InvertedFileIndex::InvertedFileIndex(double d){
    inverted_index.resize(d);
}

/*
 * takes the histogram for an image, and for each word in the histogram,
 * adds the image_location to the inverted index entry for that word
 */
vector<pair<double,double>> InvertedFileIndex::calNorm(std::vector<std::vector<array<double,2>>> v){
	std::map<double,double> map;
	for(int i = 0; i < v.size(); i++){
		for(int j = 0; j < v[i].size(); j++){
			//cout << v[i][j][0] << ": " << v[i][j][1] << endl;
			if(map.count(v[i][j][0]) > 0){
				//cout << "in" << endl;
				std::map<double, double>::iterator it = map.find(v[i][j][0]);
				it->second += pow(v[i][j][1],2);
			}
			else{
				
				map[v[i][j][0]] = pow(v[i][j][1],2);
			}
		}
	}
	vector<pair<double, double>> norm(map.size());
    std::copy(map.begin(), map.end(), norm.begin());

	for(int i = 0; i < norm.size(); i++){
		norm[i].second = pow(norm[i].second,0.5);
	}
	for(auto p : norm){
		cout << p.first << ": " << p.second << endl;
	}
	return norm;
}

void InvertedFileIndex::insert(const std::vector<double> &image, int image_location){
    //CHENG, IMPLEMENT THIS
	for(int i = 0; i < image.size(); i++){
		if(image[i] > 0){
			//int arr[2] = {image_location, image[i]};
			array<double,2> arr;
			arr[0] = image_location;
			arr[1] = image[i];
			//cout << arr[0] << ": " << arr[1] << endl;
			inverted_index[i].push_back(arr);
		}
	}
}

void InvertedFileIndex::insert(const std::map<double,double> &image, int image_location){
    //CHENG, IMPLEMENT THIS
	for(auto p : image){
		array<double,2> arr;
		arr[0] = image_location;
		arr[1] = p.second;
		//cout << p.first << ": " << p.second << endl;
		inverted_index[p.first].push_back(arr);
	}
}
/*
 * takes the histogram for an image, and for each word in the histogram,
 * casts a vote for each image_index that has that word,
 * return a list of the n image_indexes with the most votes sorted by vote count
 */
std::vector<double> InvertedFileIndex::lookup(const std::vector<double> &query, int n){
	std::vector<double> results;
	std::map<double,double> map;
	std::vector<std::vector<array<double,2>>> copy_index = inverted_index;
	//initialize inverted_index
	for(int i = 0; i < query.size(); i++){
		for(int j = 0; j < copy_index[i].size(); j++){
			copy_index[i][j][1] *= query[i];
		}
	}
	
	//calculate vote
	for(int i = 0; i < copy_index.size(); i++){
		for(int j = 0; j < copy_index[i].size(); j++){
			if(map.count(copy_index[i][j][0]) > 0){
				//cout << "in" << endl;
				std::map<double, double>::iterator it = map.find(copy_index[i][j][0]);
				it->second += copy_index[i][j][1];
			}
			else{
				map[copy_index[i][j][0]] = copy_index[i][j][1];
			}
		}
	}
	for(auto p : map){
		cout << p.first << ": " << p.second << endl;
	}
	//pass
	vector<pair<double, double>> count(map.size());
    std::copy(map.begin(), map.end(), count.begin());

	//calculate cosine distance
	double imgNorm = 0;
	for(auto p : query){
		imgNorm += pow(p,2);
	}
	//cout << "Norm: " << imgNorm << endl;
	imgNorm = pow(imgNorm,0.5);
	cout << "Norm: " << imgNorm << endl;
	vector<pair<double, double>> norm = calNorm(inverted_index);
	for(auto p : norm){
		cout << p.first << ": " << p.second << endl;
	}
	//cout << "size: " << norm.size() << endl;
	//cout << "size: " << count.size() << endl;
	vector<pair<double, double>> cosDistance(map.size());
	for(int i = 0; i < cosDistance.size(); i++){
		cosDistance[i].first = count[i].first;
		
		cosDistance[i].second = count[i].second/norm[i].second;
		cout << count[i].first << ": " << count[i].second << endl;
		cout << norm[i].first << ": " << norm[i].second << endl;
	}
	
	//sort by vote	
	for(int i = 1; i < cosDistance.size(); i++){
		int j = i;
		while(j > 0 && cosDistance[j-1].second < cosDistance[j].second){
			//swap
			pair<double,double> temp_count;
			temp_count.first = cosDistance[j].first;
			temp_count.second = cosDistance[j].second;

			cosDistance[j].first = cosDistance[j-1].first;
			cosDistance[j].second = cosDistance[j-1].second;

			cosDistance[j-1].first = temp_count.first;
			cosDistance[j-1].second = temp_count.second;

			j--;
		}
	}
	
	for(pair<double,double> ele : cosDistance){
		cout << ele.first << ": " << ele.second << endl;
		results.push_back(ele.first);
	}
    //CHENG, IMPLEMENT THIS

    return results;
}

std::vector<double> InvertedFileIndex::lookup(const std::map<double,double> &query, int n){
    std::vector<double> results;
	std::map<double,double> map;
	std::vector<std::vector<array<double,2>>> copy_index = inverted_index;
	//initialize inverted_index
	for(auto p : query){
		for(int j = 0; j < copy_index[p.first].size(); j++){
			copy_index[p.first][j][1] *= p.second;
		}
	}
	
	//calculate vote
	for(int i = 0; i < copy_index.size(); i++){
		for(int j = 0; j < copy_index[i].size(); j++){
			if(map.count(copy_index[i][j][0]) > 0){
				cout << "in" << endl;
				std::map<double, double>::iterator it = map.find(copy_index[i][j][0]);
				it->second += copy_index[i][j][1];
			}
			else{
				map[copy_index[i][j][0]] = copy_index[i][j][1];
			}
		}
	}
	//for(auto p : map){
	//	cout << p.first << ": " << p.second << endl;
	//}
	//pass
	vector<pair<double, double>> count(map.size());
    std::copy(map.begin(), map.end(), count.begin());
    
	//calculate cosine distance
	double imgNorm = 0;
	for(auto p : query){
		imgNorm += pow(p.second,2);
	}
	//cout << "Norm: " << imgNorm << endl;
	imgNorm = pow(imgNorm,0.5);
	cout << "Norm: " << imgNorm << endl;
	vector<pair<double, double>> norm = calNorm(inverted_index);
	for(auto p : norm){
		cout << p.first << ": " << p.second << endl;
	}
	//cout << "size: " << norm.size() << endl;
	//cout << "size: " << count.size() << endl;
	vector<pair<double, double>> cosDistance(map.size());
	for(int i = 0; i < cosDistance.size(); i++){
		cosDistance[i].first = count[i].first;
		
		cosDistance[i].second = count[i].second/norm[i].second;
		cout << count[i].first << ": " << count[i].second << endl;
		cout << norm[i].first << ": " << norm[i].second << endl;
	}
	
	//sort by vote	
	for(int i = 1; i < cosDistance.size(); i++){
		int j = i;
		while(j > 0 && cosDistance[j-1].second < cosDistance[j].second){
			//swap
			pair<double,double> temp_count;
			temp_count.first = cosDistance[j].first;
			temp_count.second = cosDistance[j].second;

			cosDistance[j].first = cosDistance[j-1].first;
			cosDistance[j].second = cosDistance[j-1].second;

			cosDistance[j-1].first = temp_count.first;
			cosDistance[j-1].second = temp_count.second;

			j--;
		}
	}
	
	for(pair<double,double> ele : cosDistance){
		cout << ele.first << ": " << ele.second << endl;
		results.push_back(ele.first);
	}
    //CHENG, IMPLEMENT THIS

    return results;
}