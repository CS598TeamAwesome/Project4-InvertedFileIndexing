#include <array>
#include <algorithm>
#include <assert.h>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>
#include <Util/Distances.hpp>
#include "LSH/LSH.hpp"
#include "LSH/RandomProjectionAlgorithm.hpp"

double sparse_norm(std::map<int, double> m){
    double norm2 = 0.0;
    for(auto &p : m){
        norm2 += p.second * p.second;
    }
    return std::sqrt(norm2);
}

void load_histograms(std::string filename, std::vector<std::map<int, double>> &histograms){
    //load histograms from file
    std::ifstream filein (filename);

    for(int i = 0; i < 1000; i++){
        if(!filein){
            break;
        }
        std::string s;
        getline(filein, s);
        std::istringstream sin(s);

        std::map<int, double> histogram_map;
        double index;
        double value;
        while(sin >> index){
            sin >> value;
            std::pair<int, double> key_value_pair;
            key_value_pair.first = index;
            key_value_pair.second = value;
            histogram_map.insert(key_value_pair);
        }

        if(histogram_map.size() > 0)
            histograms.push_back(histogram_map);
    }
    filein.close();
}

int main(int argc, char **argv)
{
    int dataset_size = std::atoi(argv[1]); //number of histogram files to convert to tf-idf weighting
    int hist_size = 10000; //number of codewords in vocabulary

    std::vector<std::map<int, double>> histograms;
    for(int i = 0; i < dataset_size; i++){
        std::ostringstream convert;
        convert << "histograms/" << i << ".txt";
        std::string s = convert.str();
        std::cout << s << std::endl;
        load_histograms(s, histograms);
    }

    //do a document frequency count
    int vocabulary_size = hist_size;
    std::vector<double> document_frequency(vocabulary_size);
    for(int i = 0; i < histograms.size(); i++){
        //iterate over each map
        for(auto p : histograms[i]){
            document_frequency[p.first]++;
        }
    }

    std::cout << document_frequency.size() << std::endl;
    std::ofstream out("document_frequencies.txt");
    for(int i = 0; i < document_frequency.size(); i++){
        out << document_frequency[i] << " ";
    }
    out << std::endl;

    double total_docs = histograms.size();

    //weight each histogram using tfidf
    for(int i = 0; i < histograms.size(); i++){
        for(auto& p : histograms[i]){
            double tf = 1 + std::log(p.second);
            double idf = std::log(total_docs/document_frequency[p.first]);
            p.second = tf*idf;
        }
    }

    //normalize each histogram
    for(int i = 0; i < histograms.size(); i++){
        double norm = sparse_norm(histograms[i]);
        if(norm != 0){
            for(auto& p : histograms[i]){
                    p.second = p.second/norm;
            }
        }
    }

    //write back out to files
    std::ofstream outputFile;
    for(int i = 0; i < histograms.size(); i++){
        if(i%1000 == 0){
            //open new outputFile
            if(outputFile.is_open()){
                outputFile.close();
            }

            std::ostringstream convert;
            convert << "weighted_histograms/" << i/1000 << ".txt";
            std::string s = convert.str();

            outputFile.open(s);
        }
        for(auto p : histograms[i]){
            outputFile << p.first << " " << p.second << " ";
        }
        outputFile << std::endl;
    }

    std::cout << histograms.size() << std::endl;
    return 0;
}
