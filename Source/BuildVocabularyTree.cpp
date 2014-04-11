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

    int i = 0;
    for(std::string& imFile : image_files){
        i++;
        /* --- if memory is an issue, only collect features on a random subset of images
        if(std::rand()%2 == 0){
            continue;
        }
        */
        std::cout << "detecting keypoints and computing descriptors for img " << i << " of " << image_files.size() << std::endl;

        cv::Mat img = cv::imread(imFile);

        //detect keypoints
        std::vector<cv::KeyPoint> keypoints;
        detector->detect( img, keypoints );

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
/*
    start = clock();
    std::cout << "Build Vocabulary Tree for " << samples.size() << " features" << std::endl;
    vocabulary_tree tree; //(4^4) = 256 words
    tree.K = 4; //branching factor
    tree.L = 4; //depth
    hierarchical_kmeans(samples, tree);
    std::cout << double( clock() - start ) / (double)CLOCKS_PER_SEC<< " seconds." << std::endl;

    SaveVocabularyTree("universal_vocabulary256.out", tree);


    start = clock();
    vocabulary_tree tree2; //625
    tree2.K = 5; //branching factor
    tree2.L = 4; //depth
    hierarchical_kmeans(feature_set, tree2);
    std::cout << double( clock() - start ) / (double)CLOCKS_PER_SEC<< " seconds." << std::endl;

    SaveVocabularyTree("labelme_vocabulary625.out", tree2);
*/
    start = clock();
    vocabulary_tree tree3; //1k
    tree3.K = 10; //branching factor
    tree3.L = 3; //depth
    hierarchical_kmeans(feature_set, tree3);
    std::cout << double( clock() - start ) / (double)CLOCKS_PER_SEC<< " seconds." << std::endl;

    SaveVocabularyTree("labelme_vocabulary1000.out", tree3);

    start = clock();
    vocabulary_tree tree4; //10k
    tree4.K = 10; //branching factor
    tree4.L = 4; //depth
    hierarchical_kmeans(feature_set, tree4);
    std::cout << double( clock() - start ) / (double)CLOCKS_PER_SEC<< " seconds." << std::endl;

    SaveVocabularyTree("labelme_vocabulary10k.out", tree4);

    start = clock();
    vocabulary_tree tree5; //8^5, 32k
    tree5.K = 8; //branching factor
    tree5.L = 5; //depth
    hierarchical_kmeans(feature_set, tree5);
    std::cout << double( clock() - start ) / (double)CLOCKS_PER_SEC<< " seconds." << std::endl;

    SaveVocabularyTree("labelme_vocabulary32k.out", tree5);
/*
    start = clock();
    vocabulary_tree tree6; //9^5, 60k
    tree6.K = 9; //branching factor
    tree6.L = 5; //depth
    hierarchical_kmeans(feature_set, tree6);
    std::cout << double( clock() - start ) / (double)CLOCKS_PER_SEC<< " seconds." << std::endl;

    SaveVocabularyTree("labelme_vocabulary60k.out", tree6);
*/
    return 0;
}
