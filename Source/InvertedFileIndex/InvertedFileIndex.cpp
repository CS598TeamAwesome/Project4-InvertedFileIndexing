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

InvertedFileIndex::InvertedFileIndex(int d){
    inverted_index.resize(d);
}

/*
 * takes the histogram for an image, and for each word in the histogram,
 * adds the image_location to the inverted index entry for that word
 */
void InvertedFileIndex::insert(const std::vector<double> &image, int image_location){
    //CHENG, IMPLEMENT THIS
	for(int i = 0; i < image.size(); i++){
		if(image[i] > 0){
			//int arr[2] = {image_location, image[i]};
			array<int,2> arr;
			arr[0] = image_location;
			arr[1] = image[i];
			//cout << arr[0] << ": " << arr[1] << endl;
			inverted_index[i].push_back(arr);
		}
	}
}

/*
 * takes the histogram for an image, and for each word in the histogram,
 * casts a vote for each image_index that has that word,
 * return a list of the n image_indexes with the most votes sorted by vote count
 */
std::vector<int> InvertedFileIndex::lookup(const std::vector<double> &query, int n){
	std::vector<int> results;
	std::map<int,int> map;
	//initialize inverted_index
	for(int i = 0; i < query.size(); i++){
		for(int j = 0; j < inverted_index[i].size(); j++){
			inverted_index[i][j][1] *= query[i];
		}
	}
	
	//calculate vote
	for(int i = 0; i < inverted_index.size(); i++){
		for(int j = 0; j < inverted_index[i].size(); j++){
			if(map.count(inverted_index[i][j][0]) > 0){
				cout << "in" << endl;
				std::map<int, int>::iterator it = map.find(inverted_index[i][j][0]);
				it->second += inverted_index[i][j][1];
			}
			else{
				map[inverted_index[i][j][0]] = inverted_index[i][j][1];
			}
		}
	}
	for(auto p : map){
		cout << p.first << ": " << p.second << endl;
	}
	//pass
	vector<pair<int, int>> count(map.size());
    std::copy(map.begin(), map.end(), count.begin());
    
	
	//sort by vote
	
	for(int i = 1; i < count.size(); i++){
		int j = i;
		while(j > 0 && count[j-1].second < count[j].second){
			//swap
			pair<int,int> temp_count;
			temp_count.first = count[j].first;
			temp_count.second = count[j].second;

			count[j].first = count[j-1].first;
			count[j].second = count[j-1].second;

			count[j-1].first = temp_count.first;
			count[j-1].second = temp_count.second;

			j--;
		}
	}
	
	for(pair<int,int> ele : count){
		results.push_back(ele.first);
	}
    //CHENG, IMPLEMENT THIS

    return results;
}
