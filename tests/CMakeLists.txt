add_executable(unit_tests matrix_market_test.cpp)
target_link_libraries(unit_tests MatrixMarketReader gtest gtest_main)
add_test(NAME unit COMMAND unit_tests)