#pragma once
#include <cstdlib>
#include <vector>
#include <bitset>
#include <random>

namespace InvertedFileIndexing 
{
    template <size_t N>
    class LSH 
    {
        public:
            /* build the LSH where the image representations are dimension d
             * and the number of hash bits (random hyperplanes) is L*/
            LSH(int d);

            void insert(const std::vector<double> &image);

            //gets the images from a perfect match of index - what return type?
            void lookup(const std::vector<double> &query);

            //gets all images from matches within hamming ball of radius d - what return type?
            void lookup(const std::vector<double> &query, int d);

        private:
            /*takes an image histogram, and computes the bithash against each hyperplane in L, concatenates the results into a bitset, which is the index for the image*/
            std::bitset<N> hash(const std::vector<double> &image) const;
            
            /*takes an image histogram (dimension d) and a randomly projected hyperplane,
             *returns the sign: -1 if on one side, +1 if on the other side*/
            double bithash(const std::vector<double> &image, const std::vector<double> &hyperplane) const;
            
            /*generate a random hyperplane that divides the images.
             * do this by picking a random number from a gaussian
             * distribution for every dimension*/
            std::vector<double> generateRandomProjection(int dimension, std::default_random_engine &generator) const;
            
            int d; //dimension of image representation
            std::vector<std::vector<double>> L; //list of random projections -- these are the L bithash functions
    };
}
