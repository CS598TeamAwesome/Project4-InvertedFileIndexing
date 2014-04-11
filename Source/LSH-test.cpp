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
#include "LSH/LSH.hpp"
#include "LSH/RandomProjectionAlgorithm.hpp"

double dot_product(std::vector<double> v1, std::vector<double> v2){
    //std::inner_product(image.begin(), image.end(), hyperplane.begin(), dot_product);
    double dot_product = 0.0;
    for(int i = 0; i < v1.size(); i++)
    {
        dot_product += v1[i] * v2[i];
    }
    return dot_product;
}

double norm(std::vector<double> v){
    double norm2 = std::accumulate(v.begin(), v.end(), 0.0, [](double accum, double elem) { return accum + elem * elem; });
    return std::sqrt(norm2);
}

double cosine_similarity(std::vector<double> v1, std::vector<double> v2){
    return dot_product(v1, v2)/(norm(v1) * norm(v2));
}

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
    const int bit_size = 50;

    std::vector<std::vector<double>> histograms;
    load_histograms("wang_bow256_histograms", histograms);
    InvertedFileIndexing::LSHTable<bit_size, InvertedFileIndexing::RandomProjectionAlgorithm<bit_size>> table(256);

    std::vector<std::bitset<bit_size>> bit_indexes;
    for(int i = 0; i < histograms.size(); i++){
        std::bitset<bit_size> bits;
        bits = table.insert(histograms[i], i);
        std::cout << bits.to_string() << std::endl;
        bit_indexes.push_back(bits);
    }

    //testing conditions??
    for(int i = 0; i < 1000; i++){
        std::vector<int> zero_distance_neighbors = table.lookup(histograms[i]);
        std::vector<int> d_distance_neighbors = table.lookup(histograms[i], 0);
        assert(zero_distance_neighbors.size() == d_distance_neighbors.size());

        std::vector<int> one_distance_neighbors = table.lookup(histograms[i], 1);
        //std::cout << one_distance_neighbors.size() << " ";

        std::vector<int> two_distance_neighbors = table.lookup(histograms[i], 2);
        //std::cout << two_distance_neighbors.size() << std::endl;
    }    

    //plot cosine similarities against hamming distance
    std::ofstream fileout ("distance_plot");
    for(int i = 0; i < 1000; i++){
        for(int j = i; j < 1000; j++){
            fileout << cosine_similarity(histograms[i], histograms[j]) << " " << (bit_indexes[i]^bit_indexes[j]).count() << std::endl;
        }
    }
    fileout.close();
}

int main(int argc, char **argv)
{
    test_random_projection();
    test_bithash();
    test_insert_and_lookup();
    return 0;
}
