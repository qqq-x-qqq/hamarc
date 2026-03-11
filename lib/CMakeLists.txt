add_library(
    hamarc_lib
    archive.cpp
    hamming.cpp
)

target_include_directories(hamarc_lib PUBLIC ${PROJECT_SOURCE_DIR})
