if (NOT P1_INCLUDE_DIR)
  find_path (P1_INCLUDE_DIR
    NAMES
    Util/Convolve.hpp
    PATHS
    ${P1_ROOT_DIR}/Source
    DOC
    "Project 1 include directory"
    )
endif ()

if(NOT P1_LIBRARY)
  find_library (P1_LIBRARY
    NAMES
    ColorTextureShape
    PATHS
    ${P1_ROOT_DIR}/bin
    DOC
    "Project 1 library location"
    )
endif ()
