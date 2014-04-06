#pragma once
#include <array>
#include <assert.h>
#include <bitset>
#include <cmath>
#include <functional>
#include <random>
#include <vector>
#include <stdlib.h>

namespace InvertedFileIndexing {

    class LSH {
        public:
            /* build the LSH where the image representations are dimension d
             * and the number of hash bits (random hyperplanes) is L*/
            LSH(int d, int n);

            /*generate a random hyperplane that divides the images.
             * do this by picking a random number from a gaussian
             * distribution for every dimension*/
            std::vector<double> generateRandomProjection(int dimension, std::default_random_engine &generator);

            /*takes an image histogram (dimension d) and a randomly projected hyperplane,
             *returns the sign: -1 if on one side, +1 if on the other side*/
            double bithash(std::vector<double> image, std::vector<double> hyperplane);

            /*takes an image histogram, and computes the bithash against each hyperplane in L, concatenates the results into a bitset, which is the index for the image*/
            //std::bitset<n> hash(std::vector<double> image);

            void insert(std::vector<double> image);

            //gets the images from a perfect match of index - what return type?
            void lookup(std::vector<double> query);

            //gets all images from matches within hamming ball of radius d - what return type?
            void lookup(std::vector<double> query, int d);

        private:
            int d; //dimension of image representation
            int n; //length of hash index
            std::vector<std::vector<double>> L; //list of random projections -- these are the L bithash functions

            /*the bitsets are the keys and the values are the list of images mapped to the key*/
            //HashTable<bitset, IMAGES??>;

    };
}
