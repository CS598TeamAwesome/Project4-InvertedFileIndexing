#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <time.h>
#include <vector>
#include <BagOfFeatures/Codewords.hpp>
#include <Quantization/VocabularyTreeQuantization.hpp>
#include <Util/Clustering.hpp>
#include <Util/Distances.hpp>
#include <Util/Types.hpp>
#include "LSH/LSH.hpp"
#include "LSH/RandomProjectionAlgorithm.hpp"

using namespace LocalDescriptorAndBagOfFeature;

#ifndef M_PI
#   define M_PI 3.14159265358979323846
#endif

std::vector<std::string> load_image_paths(std::string imageFileList)
{
    std::ifstream iss(imageFileList);

    std::vector<std::string> images_files;
    while(iss)
    {
        std::string imFile;
        std::getline(iss,imFile);

        if(imFile.length() > 0)
            images_files.push_back(imFile);
    }

    return images_files;
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

Histogram compute_histogram(std::string imFile, cv::Ptr<cv::FeatureDetector> &detector, cv::Ptr<cv::FeatureDetector> &detector2, cv::SiftDescriptorExtractor &extractor, Quantization *quant){
    cv::Mat img = cv::imread(imFile);

    //detect SIFT keypoints
    std::vector<cv::KeyPoint> keypoints;
    detector->detect( img, keypoints );

    //detect MSER keypoints
    std::vector<cv::KeyPoint> keypoints2;
    detector2->detect( img, keypoints2 );

    //group them together
    for(cv::KeyPoint& keypoint : keypoints2){
        keypoints.push_back(keypoint);
    }

    //std::cout << " - keypoint_ct: " << keypoints.size() << std::endl;

    //compute descriptors
    cv::Mat descriptor_uchar;
    extractor.compute(img, keypoints, descriptor_uchar);

    cv::Mat descriptor_double;
    descriptor_uchar.convertTo(descriptor_double, CV_64F);

    //convert from mat to bag of unquantized features
    BagOfFeatures unquantized_features;
    convert_mat_to_vector(descriptor_double, unquantized_features);

    //quantize to form bag of words
    Histogram bag_of_words;
    quant->quantize(unquantized_features, bag_of_words);

    //normalize

    return bag_of_words;
}

double sparse_norm(std::map<int, double> m){
    double norm2 = 0.0;
    for(auto &p : m){
        norm2 += p.second * p.second;
    }
    return std::sqrt(norm2);
}

double sparse_dot_product(std::map<int, double> m1, std::map<int, double> m2){
    //multiply any common indexes and add to total
    //otherwise increment the one that is behind
    double dot_product = 0.0;
    auto it1 = m1.begin();
    auto it2 = m2.begin();

    while(it1 != m1.end() && it2 != m2.end()){
        if(it1->first == it2->first){
            dot_product += it1->second * it2->second;
            ++it1;
            ++it2;
        } else if(it1->first > it2->first){
            ++it2;
        } else if(it2->first > it1->first){
            ++it1;
        }
    }

    return dot_product;
}


int main(int argc, char **argv)
{
    /*
     * SYSTEM STARTUP, LOADING IMAGES INTO LSH
     */

    //load file list for images
    std::vector<std::string> image_files = load_image_paths(argv[1]);
    std::cout << "Image files: " << image_files.size() << std::endl;

    int dataset_size = std::ceil(image_files.size()/1000.0); //expected number of weighted histogram files
    std::cout << "Histogram files: " << dataset_size << std::endl;

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
    std::cout << "Vocabulary Size: " << dim << std::endl;

    //load tf-idf weighted histograms
    std::vector<std::map<int, double>> histograms;
    for(int i = 0; i < dataset_size; i++){
        std::ostringstream convert;
        convert << "weighted_histograms/" << i << ".txt";
        std::string s = convert.str();
        std::cout << s << std::endl;
        load_histograms(s, histograms);
    }

    double total_docs = histograms.size();
    std::cout << "Histograms Loaded: " << total_docs << std::endl;

    //insert histograms into lsh
    const int bit_size = 512;
    InvertedFileIndexing::LSHTable<bit_size, InvertedFileIndexing::RandomProjectionAlgorithm<bit_size>> table(dim);

    std::cout << "inserting items into lsh" << std::endl;
    std::vector<std::bitset<bit_size>> bit_indexes;
    for(int i = 0; i < histograms.size(); i++){
        std::bitset<bit_size> bits;
        bits = table.insert(histograms[i], i);
        //std::cout << bits.to_string() << std::endl;
        bit_indexes.push_back(bits);
        if(i%500 == 0){
            std::cout << "... " << i << std::endl;
        }
    }

    /*
     * SETUP FOR SEARCH
     */

    //compute tf-idf histograms for probe images
    std::string detector_type = "SIFT";
    cv::Ptr<cv::FeatureDetector> detector = cv::FeatureDetector::create(detector_type);
    detector->set("nFeatures", 400);

    cv::Ptr<cv::FeatureDetector> detector2 = cv::FeatureDetector::create("MSER");

    cv::SiftDescriptorExtractor extractor;

    vocabulary_tree tree;
    LoadVocabularyTree("vocabulary10k.out", tree);
    VocabularyTreeQuantization tree_quant(tree);

    Quantization *quant;
    quant = &tree_quant;

    /*
     * SEARCH LOOP, USER PROVIDES QUERY, RETURNS AN ORDERED FILE LIST
     */
    int query_radius = 230;
    int result_ct = 15;
    while(true){
        std::cout << "Waiting for user input: " << std::endl;

        std::string query_file = "C:/datasets/INRIA/jpg_scaled/101400.jpg";

        std::string user_input;
        std::cin >> user_input;
        if(user_input.length() > 0){
            if(user_input.compare("quit") == 0){
                std::cout << "Exiting program" << std::endl;
                break;
            } else if(user_input.compare("radius") == 0){
                std::cin >> query_radius;
                std::cout << "Changing hamming radius to " << query_radius << std::endl;
                continue;
            } else if(user_input.compare("results") == 0){
                std::cin >> result_ct;
                std::cout << "Changing max number of results returned to " << result_ct << std::endl;
                continue;
            } else{
                query_file = user_input;
                std::cout << "Query file:  " << query_file << std::endl;
            }
        } else {
            std::cout << "No input was provided." << std::endl;
            continue;
        }

        //std::cin.clear();
        //std::cin.ignore(std::numeric_limits<std::streamsize>::max());

        std::cout << "USER QUERY" << std::endl;

        //compute histogram for query
        double start = clock();
        Histogram query_vector = compute_histogram(query_file, detector, detector2, extractor, quant);

        std::map<int, double> query_map;
        //convert to tf-idf and push to query map
        for(int j = 0; j < query_vector.size(); j++){
            if(query_vector[j] > 0){
                double tf = 1 + std::log(query_vector[j]);
                double idf = std::log(total_docs/document_frequencies[j]);

                //if using vector
                query_vector[j] = tf*idf;

                //if using sparse map
                std::pair<int, double> p;
                p.first = j;
                p.second = tf*idf;
                p.second = query_vector[j];
                query_map.insert(p);
            }
        }

        //normalize
        double norm = sparse_norm(query_map);
        if(norm != 0){
            for(auto& p : query_map){
                p.second = p.second/norm;
            }
        }

        double duration = double( clock() - start ) / (double)CLOCKS_PER_SEC;
        std::cout << "time to compute histogram: " << duration << std::endl;

        //first round of results
        start = clock();
        std::vector<int> initial_results = table.lookup(query_map, query_radius);
        std::cout << "Results: " << initial_results.size() << std::endl;
        duration = double( clock() - start ) / (double)CLOCKS_PER_SEC;
        std::cout << "time for lookup from lsh: " << duration << std::endl;
/*
        //check results
        std::bitset<bit_size> bits;
        bits = table.insert(query_map, -1);
        //std::vector<std::bitset<bit_size>> bit_indexes;
        for(auto &b : bit_indexes){
            int distance = (b^bits).count();
            if(distance < query_radius)
                std::cout << distance << std::endl;
        }
*/

        start = clock();

        //compute cosine similarity for results
        std::vector<std::pair<std::string, double>> reranked_list;
        for(int &i : initial_results){
            //calculate cosine similarity between query and image at index i
            std::pair<std::string, double> result;
            result.first = image_files[i]; //filename
            result.second = sparse_dot_product(query_map, histograms[i]); //similarity score
            reranked_list.push_back(result);
        }
        //sort to rerank
        std::sort(reranked_list.begin(), reranked_list.end(), [](std::pair<std::string, double> one, std::pair<std::string, double> two){
            return one.second > two.second;
        });

        duration = double( clock() - start ) / (double)CLOCKS_PER_SEC;
        std::cout << "time for reranking: " << duration << std::endl;

        //return top N items
        //for now, write to file -- but would be better to send message out
        //list of files ranked by similarity score

        std::ofstream fileout("query-results");
        //fileout.open("query-results", std::ofstream::out | std::ofstream::app);
        fileout << "RESULTS FOR: " << query_file << std::endl;
        for(int i = 0; i < reranked_list.size(); i++){
            //std::cout << r.first << " " << r.second << std::endl;
            fileout << reranked_list[i].first << " " << reranked_list[i].second << std::endl;
            if(i == result_ct)
                break;
        }
        fileout.close();
    }

    return 0;
}
