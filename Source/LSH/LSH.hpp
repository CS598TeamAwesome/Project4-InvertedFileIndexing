#pragma once
#include <algorithm>
#include <bitset>
#include <cstdlib>
#include <unordered_map>
#include <utility>
#include <vector>

namespace InvertedFileIndexing 
{
    template <size_t N, class Hasher>
    class LSHTable 
    {
        public:
            /* build the LSH where the image representations are dimension d
             * and the number of hash bits (random hyperplanes) is L*/
            LSHTable(int d):hash(d)
            {
                
            }

            /* takes the image histogram as a key, computes the hash index, and inserts the value
             * (value for now we are using an int - from which we can derive both the histogram location
             * and the image path, but the value should really be 1) a string to the file path,
             * 2)a pointer to the histogram in main memory, or 3) a pointer to a struct/class that holds the
             * file path, a pointer to the histogram, and a pointer to the thumbnail)
             */
            std::bitset<N> insert(const std::vector<double> &image, int value)
            {
                auto h = hash(image);
                
                std::pair<std::bitset<N>,int> key_value_pair(h, value);
                _HashTable.insert(key_value_pair);
                
                return h;
            }

            //gets the images from a perfect match of index
            std::vector<int> lookup(const std::vector<double> &query)
            {
                auto h = hash(query);

                auto its = _HashTable.equal_range(h); //get all values that are mapped to key h

                std::vector<int> neighbors;
                for (auto it = its.first; it != its.second; ++it) {
                    neighbors.push_back(it->second);
                    //std::cout << it->first << " " << it->second << std::endl;
                }

                return neighbors;
            }

            //gets all images from matches within hamming ball of radius d
            std::vector<int> lookup(const std::vector<double> &query, int d)
            {
                std::bitset<N> h = hash(query);

                std::vector<int> neighbors;

                //linear scan, checking the hamming distance from the query using xor and popcount
                //if < d, add to neighbors
                for(auto it = _HashTable.begin(); it != _HashTable.end(); ++it){
                    std::bitset<N> index = it->first;
                    int distance = (index^h).count();
                    if(distance <= d){
                        neighbors.push_back(it->second);
                    }
                }

                return neighbors;
            }

        private:
            Hasher hash;
            
            //the inverted file index, we need a multimap (because different histograms can have the same key)
            // --- but this will be a second layer has, because we already hash the image to a bitset index
            // --- this will treat the bitset as a key and hash it into the multimap
            std::unordered_multimap<std::bitset<N>, int> _HashTable;
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
