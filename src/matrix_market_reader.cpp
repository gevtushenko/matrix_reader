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

matrix_class::matrix_meta read_meta (
    string &meta_line,
    matrix_class::format format,
    matrix_class::data_type data_type,
    matrix_class::storage_scheme storage_scheme)
{
  istringstream iss (meta_line);

  std::remove_const<decltype(matrix_class::matrix_meta::rows_count)>::type rows_count {};
  std::remove_const<decltype(matrix_class::matrix_meta::cols_count)>::type cols_count {};
  std::remove_const<decltype(matrix_class::matrix_meta::non_zero_count)>::type non_zero_count {};

  if (iss >> rows_count >> cols_count >> non_zero_count)
    return { rows_count, cols_count, non_zero_count, format, data_type, storage_scheme };

  throw runtime_error ("Can't read matrix size");
}

template <typename data_t>
class sparse_matrix : public matrix_class
{
public:
  sparse_matrix (
    matrix_class::matrix_meta meta_arg,
    unique_ptr<data_t[]> data_arg,
    unique_ptr<unsigned int[]> row_ids_arg,
    unique_ptr<unsigned int[]> col_ids_arg)
    : matrix_class (meta_arg)
    , data (move (data_arg))
    , row_ids (move (row_ids_arg))
    , col_ids (move (col_ids_arg))
  { }

  const unsigned int *get_row_ids () const override { return row_ids.get (); }
  const unsigned int *get_col_ids () const override { return col_ids.get (); }
  const void *get_data () const override { return data.get (); }

private:
  unique_ptr<data_t[]> data;
  unique_ptr<unsigned int[]> row_ids;
  unique_ptr<unsigned int[]> col_ids;
};

class data_reader
{
public:
  data_reader () = delete;
  explicit data_reader (matrix_class::matrix_meta meta_arg)
    : meta (meta_arg)
  { }
  virtual ~data_reader () = default;
  virtual unique_ptr<matrix_class> read_matrix (istream &is) = 0;

protected:
  const matrix_class::matrix_meta meta;
};

class sparse_reader : public data_reader
{
public:
  explicit sparse_reader (matrix_class::matrix_meta meta_arg)
    : data_reader (meta_arg)
  {
    if (!meta.is_sparse ())
      throw runtime_error ("Sparse reader is not supposed for dense matrices");

    row_ids.reset (new unsigned int[meta.non_zero_count]);
    col_ids.reset (new unsigned int[meta.non_zero_count]);

    if (meta.matrix_data_type == matrix_class::data_type::real)
      dbl_data.reset (new double[meta.non_zero_count]);
    else if (meta.matrix_data_type == matrix_class::data_type::integer)
      int_data.reset (new int[meta.non_zero_count]);
    else
      throw std::runtime_error ("Unsupported matrix data type");
  }

  unique_ptr<matrix_class> read_matrix (istream &is) override
  {
    if (dbl_data)
      return read_lines (is, dbl_data);
    else if (int_data)
      return read_lines (is, int_data);

    return nullptr;
  }

private:
  template <typename data_t>
  unique_ptr<sparse_matrix<data_t>> read_lines (istream &is, unique_ptr<data_t[]> &data)
  {
    string line;
    size_t nz = 0;

    while (getline (is, line))
    {
      istringstream iss (line);
      iss >> row_ids[nz] >> col_ids[nz] >> data[nz];
             row_ids[nz]--; col_ids[nz]--; ///< Matrix Market counts indices from 1
      nz++;
    }

    return make_unique<sparse_matrix<data_t>> (
        meta, move (data), move (row_ids), move (col_ids));
  }

private:
  unique_ptr<int[]> int_data;
  unique_ptr<double[]> dbl_data;

  unique_ptr<unsigned int[]> row_ids;
  unique_ptr<unsigned int[]> col_ids;
};

unique_ptr<data_reader> create_reader (const matrix_class::matrix_meta &meta_arg)
{
  if (meta_arg.is_sparse ())
    return make_unique<sparse_reader> (meta_arg);

  throw std::runtime_error ("Matrix type is not supported");
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

      auto reader = create_reader (read_meta (line, format, data_type, storage_scheme));
      matrix_data = reader->read_matrix (is);
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
