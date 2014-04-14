#pragma once
#include <vector>
#include <array>

namespace InvertedFileIndexing 
{
    class InvertedFileIndex
    {
        public:
            InvertedFileIndex(int d);
            void insert(const std::vector<double> &image, int value);
            std::vector<int> lookup(const std::vector<double> &query, int n);
        private:
            std::vector<std::vector<array<int,2>>> inverted_index;
    };
}
