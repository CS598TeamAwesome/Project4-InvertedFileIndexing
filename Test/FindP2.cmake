if (NOT P2_INCLUDE_DIR)
  find_path (P2_INCLUDE_DIR
    NAMES
    BagOfFeatures/Codewords.hpp
    PATHS
    ${P2_ROOT_DIR}/Source
    DOC
    "Project 2 include directory"
    )
endif ()

if(NOT P2_LIBRARY)
  find_library (P2_LIBRARY
    NAMES
    LocalDescriptorAndBagOfFeature
    PATHS
    ${P2_ROOT_DIR}/bin
    DOC
    "Project 2 library location"
    )
endif ()
