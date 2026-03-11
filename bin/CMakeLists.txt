add_executable(
    hamarc
    main.cpp
)

target_compile_features(hamarc PRIVATE cxx_std_20)
target_include_directories(hamarc PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(hamarc PRIVATE hamarc_lib)
