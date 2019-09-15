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

TEST(matrix_market_reader, sparse)
{
  const unsigned int target_nz = 9;
  const string mm_file_content =
      "%%MatrixMarket matrix coordinate real symmetric\n"
      "%----------------------------------------------\n"
      "% Multi line comment \n"
      "3 3 9\n"
      "1 1 1\n"
      "1 2 2\n"
      "1 3 3\n"
      "2 1 4\n"
      "2 2 5\n"
      "2 3 6\n"
      "3 1 7\n"
      "3 2 8\n"
      "3 3 9\n";
  istringstream iss (mm_file_content);

  matrix_market::reader reader (iss);
  ASSERT_TRUE (reader);
  ASSERT_EQ (reader.matrix ().meta.non_zero_count, target_nz);

  auto col_ids = reader.matrix ().get_col_ids ();
  auto row_ids = reader.matrix ().get_row_ids ();
  auto data    = reader.matrix ().get_dbl_data ();

  for (unsigned int i = 0; i < target_nz; i++)
  {
    ASSERT_EQ (row_ids[i], i / 3);
    ASSERT_EQ (col_ids[i], i % 3);
    ASSERT_EQ (data[i], double (i + 1));
  }
}
