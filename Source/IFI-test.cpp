#include <array>
#include <algorithm>
#include <assert.h>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>
#include "InvertedFileIndex/InvertedFileIndex.hpp"

void load_histograms(std::string filename, std::vector<std::vector<double>> &histograms){
    //load histograms from file
    std::ifstream filein (filename);

    for(int i = 0; i < 1000; i++){
        std::string s;
        getline(filein, s);
        std::istringstream sin(s);

        std::vector<double> histogram;
        double d;
        while(sin >> d){
            histogram.push_back(d);
        }
        histograms.push_back(histogram);
    }
    filein.close();
}

void test_insert_and_lookup(){
    std::vector<std::vector<double>> histograms;
    load_histograms("wang_bow256_histograms", histograms);

    InvertedFileIndexing::InvertedFileIndex ifi(256); //vocabulary size is 256


    for(int i = 0; i < histograms.size(); i++){
        ifi.insert(histograms[i], i);
    }

    for(int i = 0; i < 1000; i++){
        std::vector<int> neighbors = ifi.lookup(histograms[i], 5);
        std::cout << "neighbors for " << i << ": ";
        for(int& neighbor : neighbors){
            std::cout << neighbor << " ";
        }
        std::cout << std::endl;
    }    
}

int main(int argc, char **argv)
{
    test_insert_and_lookup();
    return 0;
}
