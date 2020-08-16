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

  class matrix_meta
  {
  public:
    const unsigned int rows_count = 0;
    const unsigned int cols_count = 0;
    const unsigned int non_zero_count = 0;

    const format matrix_format;
    const data_type matrix_data_type;
    const storage_scheme  matrix_storage_scheme;

  public:
    matrix_meta (
        unsigned int rows_count_arg,
        unsigned int cols_count_arg,
        unsigned int non_zero_count_arg,

        format matrix_format_arg,
        data_type matrix_data_type_arg,
        storage_scheme  matrix_storage_scheme_arg)
      : rows_count (rows_count_arg)
      , cols_count (cols_count_arg)
      , non_zero_count (non_zero_count_arg)
      , matrix_format (matrix_format_arg)
      , matrix_data_type (matrix_data_type_arg)
      , matrix_storage_scheme (matrix_storage_scheme_arg)
    { }

    bool is_sparse () const { return matrix_format == format::coordinate; }
    bool is_dense () const { return !is_sparse (); }
  };

  matrix_class () = delete;
  explicit matrix_class (matrix_meta meta_arg) : meta (meta_arg) { }

  virtual ~matrix_class() = default;

  size_t get_rows_count () const { return meta.rows_count; }
  size_t get_cols_count () const { return meta.cols_count; }

  virtual const unsigned int *get_row_ids () const = 0; /// Return nullptr for dense matrices
  virtual const unsigned int *get_col_ids () const = 0; /// Return nullptr for dense matrices
  virtual const void *get_data () const = 0;

  const double *get_dbl_data () const
  {
    if (meta.matrix_data_type == data_type::real)
      return reinterpret_cast<const double *> (get_data ());
    throw std::runtime_error ("Accessing non dbg matrix as dbl");
  }

  const int *get_int_data () const
  {
    if (meta.matrix_data_type == data_type::integer)
      return reinterpret_cast<const int*> (get_data ());
    throw std::runtime_error ("Accessing non int matrix as int");
  }

public:
  const matrix_meta meta;
};

class reader
{
public:
  reader () = delete;
  explicit reader (std::istream &is, bool throw_exceptions=true);

  operator bool () const;

  matrix_class &matrix ();
  const matrix_class &matrix () const;

private:
  bool is_correct = false;
  std::unique_ptr<matrix_class> matrix_data;
};

}

#endif // MATRIXMARKETREADER_MATRIX_MARKET_READER_H
