#include <array>
#include <assert.h>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>
#include <LSH/LSH.hpp>

void test_random_projection(){
    InvertedFileIndexing::LSH lsh(256, 10);
    std::vector<double> rp = lsh.generateRandomProjection(256);
    for(double &d : rp){
        std::cout << d << ", ";
    }
    std::cout << std::endl;
}

int main(int argc, char **argv)
{
    test_random_projection();
    return 0;
}
