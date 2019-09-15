//
// Created by egi on 9/14/19.
//

#ifndef MATRIX_MARKET_READER_H
#define MATRIX_MARKET_READER_H

#include <istream>
#include <memory>

namespace matrix_market
{

class matrix_class
{
public:

  enum class format : int
  {
    coordinate,  ///< Sparse matrices
    array        ///< Dense matrices
  };

  enum class data_type : int
  {
    integer, real, complex, pattern
  };

  enum class storage_scheme : int
  {
    general, symmetric, hermitian, skew_symmetric
  };

  struct matrix_meta
  {
    unsigned int rows_count = 0;
    unsigned int cols_count = 0;
    unsigned int non_zero_count = 0;
  };

  size_t get_rows_count () const { return meta.rows_count; }
  size_t get_cols_count () const { return meta.cols_count; }

protected:
  matrix_meta meta;
};

class reader
{
public:
  reader () = delete;
  explicit reader (std::istream &is, bool throw_exceptions=true);

  operator bool () const;

  matrix_class &matrix ();

private:
  bool is_correct = false;
  std::unique_ptr<matrix_class> matrix_data;
};

}

#endif // MATRIXMARKETREADER_MATRIX_MARKET_READER_H
