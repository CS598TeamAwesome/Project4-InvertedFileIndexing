#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <Feature/ColorHistogram.hpp>
#include <Feature/HistogramOfOrientedGradients.hpp>
#include <BagOfFeatures/Codewords.hpp>
#include <Quantization/HardAssignment.hpp>
#include <Quantization/CodewordUncertainty.hpp>
#include <Quantization/VocabularyTreeQuantization.hpp>
#include <Util/Clustering.hpp>
#include <Util/Types.hpp>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <map>
#include <time.h>
#include <opencv2/opencv.hpp>

using namespace ColorTextureShape;
using namespace LocalDescriptorAndBagOfFeature;

double norm(std::vector<double> v){
    double norm2 = std::accumulate(v.begin(), v.end(), 0.0, [](double accum, double elem) { return accum + elem * elem; });
    return std::sqrt(norm2);
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

int main(int argc, char **argv)
{
    std::srand(time(0));
    // Load file list for images
    std::vector<std::string> image_files = load_image_paths(argv[1]);
    std::cout << image_files.size() << std::endl;

    std::string detector_type = "SIFT";
    cv::Ptr<cv::FeatureDetector> detector = cv::FeatureDetector::create(detector_type);
    detector->set("nFeatures", 400);

    cv::Ptr<cv::FeatureDetector> detector2 = cv::FeatureDetector::create("MSER");

    cv::SiftDescriptorExtractor extractor;

    vocabulary_tree tree;
    LoadVocabularyTree("vocabulary10k.out", tree);
/*
    std::vector<std::vector<double>> codebook;
    FlattenTree(tree, codebook);
    std::cout << codebook.size() << std::endl;

    HardAssignment hard_quant(codebook);
    CodewordUncertainty soft_quant(codebook, 100.0);
*/
    VocabularyTreeQuantization tree_quant(tree);

    std::ofstream outputFile;
    double total_keypoints = 0.0;
    double total_fingerprint = 0.0;
    int i = 0;
    for(std::string& imFile : image_files){
        std::cout << "computing histogram for img " << i << " of " << image_files.size();

        double start = clock();
        cv::Mat img = cv::imread(imFile);

        //detect keypoints SIFT
        std::vector<cv::KeyPoint> keypoints;
        detector->detect( img, keypoints );

        //detect keypoints MSER
        std::vector<cv::KeyPoint> keypoints2;
        detector2->detect( img, keypoints2 );

        for(cv::KeyPoint& keypoint : keypoints2){
            keypoints.push_back(keypoint);
        }

        std::cout << " - keypoint_ct: " << keypoints.size() << std::endl;
        total_keypoints += keypoints.size();

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
        tree_quant.quantize(unquantized_features, bag_of_words);
        //hard_quant.quantize(unquantized_features, bag_of_words);
        //soft_quant.quantize(unquantized_features, bag_of_words);

        //image_histograms.push_back(bag_of_words);
        double fingerprint_time = double( clock() - start ) / (double)CLOCKS_PER_SEC;
        total_fingerprint += fingerprint_time;
        std::cout << "time for image - " << fingerprint_time << std::endl;

        if(i%1000 == 0){
            //open new outputFile
            if(outputFile.is_open()){
                outputFile.close();
            }

            std::ostringstream convert;

            //the histograms folder is expected to already exist
            convert << "histograms/" << i/1000 << ".txt";
            std::string s = convert.str();

            outputFile.open(s);
        }

        //stored using sparse format - index and value separated by a space
        for(int j = 0; j < bag_of_words.size(); j++){
            if(bag_of_words[j] > 0)
                outputFile << j << " " << bag_of_words[j] << " ";
        }
        outputFile << std::endl;

        i++;
    }

    std::cout << "avg. keypoints per image: " << total_keypoints / image_files.size() << std::endl;
    std::cout << "avg. fingerprint time per image: " << total_fingerprint / image_files.size() << std::endl;

    if(outputFile.is_open()){
        outputFile.close();
    }

    return 0;
}
