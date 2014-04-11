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
#include <Util/Distances.hpp>
#include "LSH/LSH.hpp"
#include "LSH/RandomProjectionAlgorithm.hpp"

#ifndef M_PI
#   define M_PI 3.14159265358979323846
#endif

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
    //std::cout << dot_product(v1, v2) << ", " << norm(v1) << ", " << norm(v2) << ", " << norm(v1) * norm(v2);

    double norm1 = norm(v1);
    double norm2 = norm(v2);

    if(norm1 == 0 && norm2 == 0){
        return 1;
    } else if (norm1 == 0 || norm2 == 0){
        return 0;
    }

    return dot_product(v1, v2)/(norm1 * norm2);
}

double angular_similarity(double cs){
    return 1.0 - (std::acos(cs)/M_PI);
}

void load_histograms(std::string filename, std::vector<std::vector<double>> &histograms, int size){
    //load histograms from file
    std::ifstream filein (filename);

    for(int i = 0; i < 1000; i++){
        std::string s;
        getline(filein, s);
        std::istringstream sin(s);

        std::vector<double> histogram(size);
        double index;
        double value;
        while(sin >> index){
            sin >> value;
            histogram[index] = value;
        }
        histograms.push_back(histogram);
    }
    filein.close();
}

int main(int argc, char **argv)
{
    std::srand(time(0));

    //load document frequency count
    std::ifstream filein ("document_frequencies.txt");
    std::string s;
    getline(filein, s);
    std::istringstream sin(s);
    std::vector<double> document_frequencies;
    double d;
    while(sin >> d){
        document_frequencies.push_back(d);
    }
    filein.close();

    int dim = document_frequencies.size();
    std::cout << dim << std::endl;

    //load tf-idf weighted histograms
    std::vector<std::vector<double>> histograms;
    for(int i = 0; i < 10; i++){
        std::ostringstream convert;
        convert << "weighted_histograms/" << i << ".txt";
        std::string s = convert.str();
        std::cout << s << std::endl;
        load_histograms(s, histograms, dim);
    }

    //assert(histograms[0].size() == dim);
    double total_docs = histograms.size();

    //insert histograms into lsh
    const int bit_size = 64;
    InvertedFileIndexing::LSHTable<bit_size, InvertedFileIndexing::RandomProjectionAlgorithm<bit_size>> table(dim);

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

    //std::cout << cosine_similarity(histograms[6037], histograms[8007]) << std::endl;

    //plot cosine similarities against hamming distance
    std::ofstream fileout ("tfidf_distance_plot");
    int ct = 0;

    std::vector<double> matches;

    int i = 0;
    for(int j = 0; j < 10000; j++){
        ct++;
        double cs = cosine_similarity(histograms[i], histograms[j]);
        fileout << cs << " " << (bit_indexes[i]^bit_indexes[j]).count() << std::endl;
        std::cout << i << "," << j << ": " << cs << " " << (bit_indexes[i]^bit_indexes[j]).count() << std::endl;

        if(cs > 0.1){
            matches.push_back(j);
        }
    }

    for(double&d : matches){
        std::cout << d << std::endl;
    }

    fileout.close();

    return 0;
}
