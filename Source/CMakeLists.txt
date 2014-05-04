cmake_minimum_required (VERSION 2.6)

project(InvertedFileIndexing)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})

find_package(OpenCV REQUIRED)
find_package(P2 REQUIRED)
find_package(P1 REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${P2_INCLUDE_DIR})
include_directories(${P1_INCLUDE_DIR})

set(SOURCE)
set(HEADERS)

add_subdirectory(InvertedFileIndex)
add_subdirectory(LSH)

add_library(InvertedFileIndexing ${SOURCE} ${HEADERS})
target_link_libraries(InvertedFileIndexing ${P1_LIBRARY} ${P2_LIBRARY} ${OpenCV_LIBS})

add_executable(IFI-test IFI-test.cpp)
target_link_libraries(IFI-test InvertedFileIndexing)

add_executable(LSH-test LSH-test.cpp)
target_link_libraries(LSH-test InvertedFileIndexing)

add_executable(BuildVocabularyTree BuildVocabularyTree.cpp)
target_link_libraries(BuildVocabularyTree InvertedFileIndexing)

add_executable(ComputeHistograms ComputeHistograms.cpp)
target_link_libraries(ComputeHistograms InvertedFileIndexing)

add_executable(TfIdfWeighting TfIdfWeighting.cpp)
target_link_libraries(TfIdfWeighting InvertedFileIndexing)

add_executable(TestSearch TestSearch.cpp)
target_link_libraries(TestSearch InvertedFileIndexing)

add_executable(VisualizeResults VisualizeResults.cpp)
target_link_libraries(VisualizeResults InvertedFileIndexing)

add_executable(OnlineSearch OnlineSearch.cpp)
target_link_libraries(OnlineSearch InvertedFileIndexing)