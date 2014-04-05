if (NOT P4_INCLUDE_DIR)
  find_path (P4_INCLUDE_DIR
    NAMES
    LSH/LSH.hpp
    PATHS
    ${P4_ROOT_DIR}/Source
    DOC
    "Project 4 include directory"
    )
endif ()

if(NOT P4_LIBRARY)
  find_library (P4_LIBRARY
    NAMES
    InvertedFileIndexing
    PATHS
    ${P4_ROOT_DIR}/bin
    DOC
    "Project 4 library location"
    )
endif ()
