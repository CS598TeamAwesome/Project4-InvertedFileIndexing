#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/types_c.h>
#include <opencv2/highgui/highgui_c.h>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <Feature/ColorHistogram.hpp>
#include <Feature/HistogramOfOrientedGradients.hpp>
#include <BagOfFeatures/Codewords.hpp>
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
    detector->set("nFeatures", 200);

    cv::SiftDescriptorExtractor extractor;

    BagOfFeatures feature_set;

    vocabulary_tree tree;
    LoadVocabularyTree("labelme_vocabulary10000.out", tree);

    VocabularyTreeQuantization tree_quant(tree);

    std::vector<Histogram> image_histograms;

    int i = 0;
    for(std::string& imFile : image_files){
        i++;
        std::cout << "computing histogram for img " << i << " of " << image_files.size();

        cv::Mat img = cv::imread(imFile);

        //detect keypoints
        std::vector<cv::KeyPoint> keypoints;
        detector->detect( img, keypoints );

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
        tree_quant.quantize(unquantized_features, bag_of_words);

        image_histograms.push_back(bag_of_words);
    }

    //write image histograms to file(s), 1000 in each file, sparse-format
    std::ofstream outputFile;
    for(int i = 0; i < image_histograms.size(); i++){
        if(i%1000 == 0){
            //open new outputFile
            if(outputFile.is_open()){
                outputFile.close();
            }

            std::ostringstream convert;
            convert << "histograms/" << i/1000 << ".txt";
            std::string s = convert.str();

            outputFile.open(s);
        }
        for(int j = 0; j < image_histograms[i].size(); j++){
            if(image_histograms[i][j] > 0)
                outputFile << j << " " << image_histograms[i][j] << " ";
        }
        outputFile << std::endl;

    }
    if(outputFile.is_open()){
        outputFile.close();
    }
    return 0;
}
