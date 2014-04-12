#include <algorithm>
#include <cstdlib>
#include <utility>
#include "InvertedFileIndex.hpp"

using namespace InvertedFileIndexing;

InvertedFileIndex::InvertedFileIndex(int d){
    inverted_index.resize(d);
}

/*
 * takes the histogram for an image, and for each word in the histogram,
 * adds the image_location to the inverted index entry for that word
 */
void InvertedFileIndex::insert(const std::vector<double> &image, int image_location){
}

/*
 * takes the histogram for an image, and for each word in the histogram,
 * casts a vote for each image_index that has that word,
 * return a list of the n image_indexes with the most votes sorted by vote count
 */
std::vector<int> InvertedFileIndex::lookup(const std::vector<double> &query, int n){
    std::vector<int> results;

    return results;
}
