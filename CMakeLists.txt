cmake_minimum_required(VERSION 3.10)
project(MatrixMarketReader)

set(CMAKE_CXX_STANDARD 14)

find_package(GTest)

add_library(${PROJECT_NAME} include/matrix_market_reader.h src/matrix_market_reader.cpp)
target_include_directories(${PROJECT_NAME} PUBLIC include)

if (GTest_FOUND)
    enable_testing()
    add_subdirectory(tests)
endif()
