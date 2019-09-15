//
// Created by egi on 9/14/19.
//

#include "matrix_market_reader.h"

#include <algorithm>
#include <sstream>
#include <fstream>

using namespace std;
using namespace matrix_market;

static string to_lower_case (const string &str)
{
  string result;
  transform (
      str.begin(), str.end(), std::back_inserter (result),
      [](unsigned char c){ return std::tolower(c); });
  return result;
}

template <typename enum_type>
enum_type str_to_enum (const string &lower_case_str)
{
  throw runtime_error ("Unsupported enum");
}

template <>
matrix_class::format str_to_enum<matrix_class::format> (const string &lower_case_str)
{
  const string sparse = "coordinate";
  const string dense = "array";

  if (lower_case_str == sparse)
    return matrix_class::format::coordinate;
  else if (lower_case_str == dense)
    return matrix_class::format::array;

  throw runtime_error ("Unsupported format type '" + lower_case_str + "'");
}

template <>
matrix_class::data_type str_to_enum<matrix_class::data_type > (const string &lower_case_str)
{
  const string real_dt = "real";
  const string complex_dt = "complex";
  const string pattern_dt = "pattern";
  const string integer_dt = "integer";

  if (lower_case_str == real_dt)
    return matrix_class::data_type::real;
  else if (lower_case_str == complex_dt)
    return matrix_class::data_type::complex;
  else if (lower_case_str == pattern_dt)
    return matrix_class::data_type::pattern;
  else if (lower_case_str == integer_dt)
    return matrix_class::data_type::integer;

  throw runtime_error ("Unsupported data type '" + lower_case_str + "'");
}

template <>
matrix_class::storage_scheme str_to_enum<matrix_class::storage_scheme> (const string &lower_case_str)
{
  const string general = "general";
  const string symmetric = "symmetric";
  const string hermitian = "hermitian";
  const string skew_symmetric = "skew_symmetric";

  if (lower_case_str == general)
    return matrix_class::storage_scheme::general;
  else if (lower_case_str == symmetric)
    return matrix_class::storage_scheme::symmetric;
  else if (lower_case_str == hermitian)
    return matrix_class::storage_scheme::hermitian;
  else if (lower_case_str == skew_symmetric)
    return matrix_class::storage_scheme::skew_symmetric;

  throw runtime_error ("Unsupported storage scheme '" + lower_case_str + "'");
}

static void check_header (istream &is, const string &target_value)
{
  string value;

  if (is >> value)
  {
    if (to_lower_case (value) != to_lower_case (target_value))
      throw std::runtime_error (
          "Invalid value in header: " + value + " != " + target_value);
  }
  else
  {
    throw runtime_error ("Can't read value " + target_value);
  }
}

template <typename enum_type>
enum_type get_header_value (istream &is)
{
  string value;
  if (is >> value)
    return str_to_enum<enum_type> (to_lower_case (value));
  throw runtime_error ("Broken format");
}

void skip_comments (istream &is, string &line)
{
  while (getline (is, line))
  {
    if (line.empty())
      throw std::runtime_error ("Unexpected empty line");

    if (line[0] != '%')
      break;
  }
}

matrix_class::matrix_meta read_meta (string &meta_line)
{
  istringstream iss (meta_line);
  matrix_class::matrix_meta meta;

  if (iss >> meta.rows_count >> meta.cols_count >> meta.non_zero_count)
    return meta;

  throw runtime_error ("Can't read matrix size");
}

#define DEBUG_READER 0

reader::reader (istream &is, bool throw_exceptions)
{
#if !DEBUG_READER
  try
  {
#endif
    string line;

    if (getline (is, line))
    {
      istringstream iss (line);
      check_header (iss, "%%MatrixMarket");
      check_header (iss, "matrix");

      auto format = get_header_value<matrix_class::format> (iss);
      auto data_type = get_header_value<matrix_class::data_type> (iss);
      auto storage_scheme = get_header_value<matrix_class::storage_scheme> (iss);

      skip_comments (is, line);

      matrix_class::matrix_meta meta = read_meta (line);
    }
    else
    {
      throw runtime_error ("Can't read header");
    }
#if !DEBUG_READER
  }
  catch (...)
  {
    is_correct = false;
    if (throw_exceptions)
      throw;
  }
#endif

  is_correct = true;
}

reader::operator bool() const
{
  return is_correct;
}

matrix_class &reader::matrix ()
{
  if (is_correct && matrix_data)
    return *matrix_data;

  throw runtime_error ("Current instance of reader isn't initialized");
}
