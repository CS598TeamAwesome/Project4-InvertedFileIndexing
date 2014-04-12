#pragma once
#include <vector>

namespace InvertedFileIndexing 
{
    class InvertedFileIndex
    {
        public:
            InvertedFileIndex(int d);
            void insert(const std::vector<double> &image, int value);
            std::vector<int> lookup(const std::vector<double> &query, int n);
        private:
            std::vector<std::vector<double>> inverted_index;
    };
}
