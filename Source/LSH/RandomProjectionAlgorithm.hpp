#pragma once

#include "LSH.hpp"
#include <random>

namespace InvertedFileIndexing
{
    template<size_t N>
    class RandomProjectionAlgorithm : public HashingAlgorithm<N>
    {
        public:
            RandomProjectionAlgorithm(int d)
                : HashingAlgorithm<N>(d),  L(N)
            {    
                std::default_random_engine generator;
                for(int i = 0; i < N; i++)
                {
                    L[i] = generateRandomProjection(d, generator);
                }
            }
            
            /*generate a random hyperplane that divides the images.
             * do this by picking a random number from a gaussian
             * distribution for every dimension*/
            std::vector<double> generateRandomProjection(int dimension, std::default_random_engine &generator) const
            {
                std::normal_distribution<double> distribution(0.0, 1.0);
            
                std::vector<double> randomProjection;
                for(int i = 0; i < dimension; i++)
                {
                    double sample = distribution(generator);
                    randomProjection.push_back(sample);
                }
            
                return randomProjection;
            }
            
            /*takes an image histogram (dimension d) and a randomly projected hyperplane,
             *returns the sign: -1 if on one side, +1 if on the other side*/
            double bithash(const std::vector<double> &image, const std::vector<double> &hyperplane) const
            {
                double dot_product = 0.0;
                //std::inner_product(image.begin(), image.end(), hyperplane.begin(), dot_product);
                // -- the std function is returning 0 for some reason
                for(int i = 0; i < image.size(); i++)
                {
                    dot_product += image[i] * hyperplane[i];
                }
            
                return std::copysign(1.0, dot_product);
            }
            
            std::bitset<N> hash(const std::vector<double> &image) const
            {
                std::bitset<N> bits;
                
                for(int i = 0; i < L.size(); i++)
                {
                    auto hyperplane = L[i];
                    
                    double sign = bithash(image, hyperplane);
                    bits[i] = sign > 0;
                }
                
                return bits;
            }
            
        private:
            int d; //dimension of image representation
            std::vector<std::vector<double>> L; //list of random projections -- these are the L bithash functions
                       
    };
}
