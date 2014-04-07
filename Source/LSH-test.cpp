#include <array>
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
#include "LSH/LSH.hpp"
#include "LSH/RandomProjectionAlgorithm.hpp"

void test_random_projection(){
    InvertedFileIndexing::RandomProjectionAlgorithm<10> lsh(256);

    std::random_device rd;
    std::default_random_engine generator( rd() );

    std::vector<double> rp = lsh.generateRandomProjection(256, generator);
    for(double &d : rp){
        std::cout << d << ", ";
    }
    std::cout << std::endl;
}

void test_bithash(){
    InvertedFileIndexing::RandomProjectionAlgorithm<1> lsh(10);

    std::random_device rd;
    std::default_random_engine generator( rd() );

    std::vector<double> rp = lsh.generateRandomProjection(10, generator);

    //random image vector -- should use actual images
    for(int i = 0; i < 100; i++){
        std::cout << i << std::endl;
        std::vector<double> image_histogram = lsh.generateRandomProjection(10, generator);
        std::cout << lsh.bithash(image_histogram, rp) << std::endl;

        for(double &d : image_histogram){
            std::cout << d << ", ";
        }
        std::cout << std::endl;
    }
}

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
    InvertedFileIndexing::LSHTable<10, InvertedFileIndexing::RandomProjectionAlgorithm<10>> table(256);

    std::bitset<10> bits;
    for(int i = 0; i < histograms.size(); i++){
        bits = table.insert(histograms[i], i);
        std::cout << bits.to_string() << std::endl;
    }

    //testing conditions??

    for(int i = 0; i < histograms.size(); i++){
        std::vector<int> value = table.lookup(histograms[i]);
        std::cout << value.size() << std::endl;
    }
}

int main(int argc, char **argv)
{
    test_random_projection();
    test_bithash();
    test_insert_and_lookup();
    return 0;
}
