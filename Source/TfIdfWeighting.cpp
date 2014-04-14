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

void load_histograms(std::string filename, std::vector<std::vector<double>> &histograms, int size){
    //load histograms from file
    std::ifstream filein (filename);

    for(int i = 0; i < 1000; i++){
        if(!filein){
            break;
        }
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
    int hist_size = 10000;

    std::vector<std::vector<double>> histograms;
    for(int i = 0; i < 11; i++){
        std::ostringstream convert;
        convert << "histograms/" << i << ".txt";
        std::string s = convert.str();
        std::cout << s << std::endl;
        load_histograms(s, histograms, hist_size);
    }

    //do a document frequency count
    int vocabulary_size = histograms[0].size();
    std::vector<double> document_frequency(vocabulary_size);
    for(int i = 0; i < histograms.size(); i++){
        for(int j = 0; j < histograms[i].size(); j++){
            if(histograms[i][j] > 0){
                document_frequency[j]++;
            }
        }
    }

    std::ofstream out("document_frequencies.txt");
    for(int i = 0; i < document_frequency.size(); i++){
        out << document_frequency[i] << " ";
    }
    out << std::endl;

    double total_docs = histograms.size();

    //weight each histogram using tfidf
    for(int i = 0; i < histograms.size(); i++){
        for(int j = 0; j < histograms[i].size(); j++){
            if(histograms[i][j] > 0){
                double tf = 1 + std::log(histograms[i][j]);
                double idf = std::log(total_docs/document_frequency[j]);
                histograms[i][j] = tf*idf;
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
        for(int j = 0; j < histograms[i].size(); j++){
            if(histograms[i][j] != 0)
            outputFile << j << " " << histograms[i][j] << " ";
        }
        outputFile << std::endl;

    }

    std::cout << histograms.size() << std::endl;
    return 0;
}
