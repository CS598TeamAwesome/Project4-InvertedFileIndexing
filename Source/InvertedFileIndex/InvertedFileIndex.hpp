#pragma once
#include <vector>
#include <array>
#include <map>
namespace InvertedFileIndexing 
{
    class InvertedFileIndex
    {
        public:
            InvertedFileIndex(double d);
			std::vector<pair<double,double>> calNorm(std::vector<std::vector<array<double,2>>> v);
            void insert(const std::vector<double> &image, int value);
			void insert(const std::map<double,double> &image, int value);
            std::vector<double> lookup(const std::vector<double> &query, int n);
			std::vector<double> lookup(const std::map<double,double> &query, int n);
        private:
            std::vector<std::vector<array<double,2>>> inverted_index;
    };
}