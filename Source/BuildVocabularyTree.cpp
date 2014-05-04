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

    // SIFT detector capped at 400 features per image
    std::string detector_type = "SIFT";
    cv::Ptr<cv::FeatureDetector> detector = cv::FeatureDetector::create(detector_type);
    detector->set("nFeatures", 400);

    // MSER detector
    cv::Ptr<cv::FeatureDetector> detector2 = cv::FeatureDetector::create("MSER");

    // SIFT extratctor
    cv::SiftDescriptorExtractor extractor;

    BagOfFeatures feature_set;

    // For a random subset of images in the image file list:
    // 1. load image into Mat
    // 2. gather SIFT and MSER keypoints
    // 3. compute SIFT descriptors and add them to the aggregate feature list
    int i = 0;
    for(std::string& imFile : image_files){
        i++;
        // --- if memory or run-time is an issue, only collect features on a random subset of images
        if(std::rand()%3 != 0){ //this will skip 2/3 of the images
            continue;
        }

        std::cout << "detecting keypoints and computing descriptors for img " << i << " of " << image_files.size() << std::endl;

        cv::Mat img = cv::imread(imFile);

        //detect keypoints
        std::vector<cv::KeyPoint> keypoints;
        detector->detect( img, keypoints );

        std::vector<cv::KeyPoint> keypoints2;
        detector2->detect( img, keypoints2);

        std::cout << keypoints.size() << " " << keypoints2.size() << std::endl;

        for(cv::KeyPoint& keypoint : keypoints2){
            keypoints.push_back(keypoint);
        }

        //compute descriptors
        cv::Mat descriptor_uchar;
        extractor.compute(img, keypoints, descriptor_uchar);

        cv::Mat descriptor_double;
        descriptor_uchar.convertTo(descriptor_double, CV_64F);

        //convert from mat to bag of unquantized features
        BagOfFeatures unquantized_features;
        convert_mat_to_vector(descriptor_double, unquantized_features);

        //combine all features together for clustering
        for(Histogram &h : unquantized_features){
            feature_set.push_back(h);
        }
    }

    std::cout << "total number of features: " << feature_set.size() << std::endl;
    double start;

    // Clustering using Hierarchical K-means
    start = clock();
    vocabulary_tree tree; //10k
    tree.K = 10; //branching factor
    tree.L = 4; //depth
    hierarchical_kmeans(feature_set, tree);
    std::cout << double( clock() - start ) / (double)CLOCKS_PER_SEC<< " seconds." << std::endl;

    SaveVocabularyTree("vocabulary10k.out", tree);

    return 0;
}
