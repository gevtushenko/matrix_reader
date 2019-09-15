//
// Created by egi on 9/15/19.
//

#include "gtest/gtest.h"
#include "matrix_market_reader.h"

#include <sstream>
#include <string>

using namespace std;

TEST(matrix_market_reader, empty_sparse)
{
  const string mm_file_content =
    "%%MatrixMarket matrix coordinate real symmetric\n"
    "%----------------------------------------------\n"
    "% Multi line comment \n"
    "0 0 0\n";
  istringstream iss (mm_file_content);

  matrix_market::reader reader (iss);
  ASSERT_TRUE (reader);
}
