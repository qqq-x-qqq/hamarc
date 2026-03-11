add_executable(
    hamarc
    main.cpp
    ../../labwork3-qqq-x-qqq/lib/argparser.cpp
)

target_compile_features(hamarc PRIVATE cxx_std_20)
target_include_directories(hamarc PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../.. /Users/ruslan/Documents/Programms)
target_link_libraries(hamarc PRIVATE hamarc_lib)
