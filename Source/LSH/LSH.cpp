#include "LSH.hpp"
#include <iostream>
#include <numeric>

using namespace InvertedFileIndexing;

LSH::LSH(int dimension_ct, int bithash_ct){
    d = dimension_ct;
    n = bithash_ct;
}

/**
 * Independently sample a gaussian function with mean 0 and variance 1 K times (once for each dimension)
 *  -- this means that we will have to normalize/scale our input image representations
 *  -- OR we will need to find the appropriate mean and variance for our dataset
 * Random numbers generated using c++11 std::normal distribution.
 * If the compiler doesn't support std::normal_distribution, random numbers should be generated using Box-Muller transformation.
 * source: Ravichandran et al. "Randomized Algorithms and NLP: Using Locality Sensitive Hash Function for High Speed Noun Clustering"
 */
std::vector<double> LSH::generateRandomProjection(int K, std::default_random_engine &generator){
    std::normal_distribution<double> distribution(0.0, 1.0);

    std::vector<double> randomProjection;
    for(int i = 0; i < K; i++){
        double sample = distribution(generator);
        randomProjection.push_back(sample);
    }

    return randomProjection;
}

/**
 * compute dot product of the image vector and the hyperplane
 * return the sign
 */
double LSH::bithash(std::vector<double> image, std::vector<double> hyperplane){
    double dot_product = 0.0;
    //std::inner_product(image.begin(), image.end(), hyperplane.begin(), dot_product);
    // -- the std function is returning 0 for some reason
    for(int i = 0; i < image.size(); i++){
        dot_product += image[i] * hyperplane[i];
    }

    std::cout << dot_product << std::endl;

    return std::copysign(1.0, dot_product);
}
