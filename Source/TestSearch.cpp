#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/nonfree/features2d.hpp>
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
#include <unordered_map>
#include <vector>
#include <Feature/ColorHistogram.hpp>
#include <BagOfFeatures/Codewords.hpp>
#include <Util/Distances.hpp>
#include <Quantization/VocabularyTreeQuantization.hpp>
#include <Util/Clustering.hpp>
#include <Util/Types.hpp>
#include "LSH/LSH.hpp"
#include "LSH/RandomProjectionAlgorithm.hpp"
#include <opencv2/opencv.hpp>

using namespace ColorTextureShape;
using namespace LocalDescriptorAndBagOfFeature;

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
        if(!filein){
            histograms.pop_back();
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
/*
        //normalize
        double n = norm(histogram);
        for(int j = 0; j < histogram.size(); j++){
            if(histogram[j] > 0){
                histogram[j] = histogram[j]/n;
            }
        }
*/
        histograms.push_back(histogram);
    }
    filein.close();
}

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

     std::cout << " - keypoint_ct: " << keypoints.size() << std::endl;

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
    for(int i = 0; i < 11; i++){
        std::ostringstream convert;
        convert << "weighted_histograms/" << i << ".txt";
        std::string s = convert.str();
        std::cout << s << std::endl;
        load_histograms(s, histograms, dim);
    }
/*
    //load color histograms
    std::vector<std::vector<double>> color_histograms;
    for(int i = 0; i < 11; i++){
        std::ostringstream convert;
        convert << "color_histograms/" << i << ".txt";
        std::string s = convert.str();
        std::cout << s << std::endl;
        load_histograms(s, color_histograms, 64);
    }

    //concatenate
    for(int i = 0; i < histograms.size(); i++){
        //double downweight = 4.0;
        for(int j = 0; j < color_histograms[i].size(); j++){
            histograms[i].push_back(color_histograms[i][j]);
        }
        std::cout << i << " " << histograms[i].size() << std::endl;
    }
*/
    //assert(histograms[0].size() == dim);
    double total_docs = histograms.size();

    //insert histograms into lsh
    const int bit_size = 32;
    InvertedFileIndexing::LSHTable<bit_size, InvertedFileIndexing::RandomProjectionAlgorithm<bit_size>> table(dim);

    std::vector<std::bitset<bit_size>> bit_indexes;
    for(int i = 0; i < histograms.size(); i++){
        std::bitset<bit_size> bits;
        bits = table.insert(histograms[i], i);
        std::cout << bits.to_string() << std::endl;
        bit_indexes.push_back(bits);
    }
/*
  //TEST 1 - 6 probes with expected matches placed into the image set

    //compute tf-idf histograms for probe images
    std::string detector_type = "SIFT";
    cv::Ptr<cv::FeatureDetector> detector = cv::FeatureDetector::create(detector_type);
    detector->set("nFeatures", 200);

    cv::Ptr<cv::FeatureDetector> detector2 = cv::FeatureDetector::create("MSER");

    cv::SiftDescriptorExtractor extractor;

    vocabulary_tree tree;
    LoadVocabularyTree("labelme_vocabulary10k.out", tree);
    VocabularyTreeQuantization tree_quant(tree);

    Quantization *quant;
    quant = &tree_quant;

    std::unordered_map<int, std::string> index_to_filename;

    std::vector<std::string> probes;
    probes.push_back("probes/101000.jpg");
    probes.push_back("probes/101100.jpg");
    probes.push_back("probes/101200.jpg");
    probes.push_back("probes/101500.jpg");
    probes.push_back("probes/102100.jpg");
    probes.push_back("probes/102200.jpg");

    ColorHistogram ch;

    std::ofstream fileout ("tfidf_distance_plot_probes");
    for(std::string &query : probes){
        std::cout << "computing histogram for query: " << query;

        Histogram query_vector = compute_histogram(query, detector, detector2, extractor, quant);

        //convert to tf-idf
        for(int j = 0; j < query_vector.size(); j++){
            if(query_vector[j] > 0){
                double tf = 1 + std::log(query_vector[j]);
                double idf = std::log(total_docs/document_frequencies[j]);
                query_vector[j] = tf*idf;
            }
        }

        std::bitset<bit_size> bits;
        bits = table.insert(query_vector, -1);

        //exhaustive search for best cosine similarity
        //exhaustive search for best hamming distance
        std::cout << "VALUES FOR " << query << std::endl;
        for(int j = 0; j < histograms.size(); j++){
            double cs = cosine_similarity(query_vector, histograms[j]);
            double hd = (bits^bit_indexes[j]).count();
            fileout << cs << " " << hd << std::endl;

            if(j > 9999){
                //auto it = index_to_filename.find(j);
                std::cout << j << " " << cs << " " << hd << std::endl;
            } else if(cs > .1){
                std::cout << j << " " << cs << " " << hd << std::endl;
            }
        }
    }
    fileout.close();
*/

/*  //TEST 2 - lookup neighbors a fixed hamming distance away from query
    std::vector<double> good_items;
    //testing conditions??
    for(int i = 0; i < 10000; i++){
        //std::vector<int> zero_distance_neighbors = table.lookup(histograms[i]);
        //std::vector<int> d_distance_neighbors = table.lookup(histograms[i], 0);
        //assert(zero_distance_neighbors.size() == d_distance_neighbors.size());

        std::vector<int> neighbors = table.lookup(histograms[i], 15);
        //std::cout << one_distance_neighbors.size() << " ";

        if(neighbors.size() > 1){
            good_items.push_back(i);
            std::cout << "good item: " << i << ", " << neighbors.size() << "- ";

            for(int&i : neighbors){
                std::cout << i << " ";
            }
            std::cout << std::endl;
        }
    }
*/
    //std::cout << cosine_similarity(histograms[6037], histograms[8007]) << std::endl;
/*
  //TEST 3 - plot cosine similarities against hamming distance for selected image pairs
    //plot cosine similarities against hamming distance
    std::ofstream fileout ("tfidf_distance_plot_MSER_color");
    int ct = 0;

    int has_matches [] = {3, 14, 15, 17, 21, 22, 23, 27, 29, 35, 43, 49, 50, 72, 77, 80, 86, 88, 100, 101, 107, 111, 117, 137, 157, 163, 171, 187, 195, 198, 207, 208, 217, 238, 245, 247, 258, 261, 263, 266, 274, 280, 281, 283, 286, 289, 291};

    for(int k = 0; k < 47; k++){
        int i = has_matches[k];

        std::cout << "VALUES FOR " << has_matches[k] << std::endl;
        for(int j = 0; j < 10000; j++){
            ct++;
            double cs = cosine_similarity(histograms[i], histograms[j]);
            double hd = (bit_indexes[i]^bit_indexes[j]).count();
            fileout << cs << " " << hd << std::endl;

            if(cs > 0.3){
                std::cout << j << ": " << cs << " " << hd << std::endl;
            }          
        }
    }

    fileout.close();
*/

    //TEST 4 - lookup time for:
    // 1. exhaustive cosine similarity search
    // 2. linear scan (also exhaustive) for hamming distance
    // 3. inverted file index lookup
      int has_matches [] = {3, 14, 15, 17, 21, 22, 23, 27, 29, 35, 43, 49, 50, 72, 77, 80, 86, 88, 100, 101, 107, 111, 117, 137, 157, 163, 171, 187, 195, 198, 207, 208, 217, 238, 245, 247, 258, 261, 263, 266, 274, 280, 281, 283, 286, 289, 291};

      double start = clock();
      //cosine similarity
      for(int i = 0; i < 1000; i++){
          for(int j = 0; j < 100; j++){
              double cs = cosine_similarity(histograms[i], histograms[j]);
          }
      }
      std::cout << double( clock() - start ) / (double)CLOCKS_PER_SEC<< " seconds for exhaustive cosine similarity." << std::endl;

      start = clock();
      //hamming distance
      for(int i = 0; i < 1000; i++){
          std::vector<int> neighbors;
          for(int j = 0; j < 10000; j++){
              double hd = (bit_indexes[i]^bit_indexes[j]).count();
              if(hd <= 40){
                  neighbors.push_back(j);
              }
          }
      }
      std::cout << double( clock() - start ) / (double)CLOCKS_PER_SEC<< " seconds for hamming distance linear scan." << std::endl;

      start = clock();
      //hamming distance lsh
      for(int i = 0; i < 1000; i++){
          table.lookup(histograms[i], 5);
      }
      std::cout << double( clock() - start ) / (double)CLOCKS_PER_SEC<< " seconds for lsh lookup (hamming distance linear scan)." << std::endl;

      //inverted file index

    return 0;
}
