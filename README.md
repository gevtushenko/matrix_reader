# Matrix Market Reader C++ Library

[![Build status](https://travis-ci.org/senior-zero/matrix_reader.svg?branch=master)](https://travis-ci.org/senior-zero/matrix_reader.svg?branch=master)
[![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/senior-zero/matrix_reader/master/LICENSE)
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/e73a0277d8d74379b5e4152ee39f600a)](https://www.codacy.com/manual/senior-zero/matrix_reader?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=senior-zero/matrix_reader&amp;utm_campaign=Badge_Grade)

## Integration

You can use this library in your CMake project:

```cmake
add_subdirectory(external/matrix_reader)
target_link_libraries(${CMAKE_PROJECT_NAME} MatrixMarketReader)
```

## Usage

```cpp
ifstream is ("/path/to_mtx");

matrix_market::reader reader (is);

if (reader) 
{
  auto &matrix = reader.matrix ();
  auto &meta   = matrix.meta;
  auto col_ids = matrix.get_col_ids ();
  auto row_ids = matrix.get_row_ids ();
  auto data    = matrix.get_dbl_data ();

  for (unsigned int i = 0; i < meta.non_zero_count; i++)
  {
    const unsigned int col = col_ids[i];
    const unsigned int row = row_ids[i];
    const auto value = data[i];
  }
}
```