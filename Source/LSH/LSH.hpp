#pragma once
#include <cstdlib>
#include <vector>
#include <bitset>
#include <unordered_map>

namespace InvertedFileIndexing 
{
    template <size_t N, class Hasher>
    class LSHTable 
    {
        public:
            /* build the LSH where the image representations are dimension d
             * and the number of hash bits (random hyperplanes) is L*/
            LSHTable(int d)
            {
                
            }

            std::bitset<N> insert(const std::vector<double> &image)
            {
                auto h = hash(image);
                
                _HashTable[h] = image;
                
                return h;
            }

            //gets the images from a perfect match of index - what return type?
            void lookup(const std::vector<double> &query);

            //gets all images from matches within hamming ball of radius d - what return type?
            void lookup(const std::vector<double> &query, int d);

        private:            
            Hasher hash;
            
            std::unordered_map<std::bitset<N>, std::vector<double>> _HashTable;
    };
    
    template<size_t N>
    class HashingAlgorithm
    {
        public:
            HashingAlgorithm(int d)
            {
                this->d = d;
            }
            
            /*takes an image histogram, and computes the bithash against each hyperplane in L, concatenates the results into a bitset, which is the index for the image*/
            virtual std::bitset<N> hash(const std::vector<double> &image) const = 0;
            
            std::bitset<N> operator() (const std::vector<double> &image) const
            {
                return hash(image);
            }
            
        private:
            int d;
    };
}
